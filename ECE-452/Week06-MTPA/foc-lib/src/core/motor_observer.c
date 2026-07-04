/**
 * @file motor_observer.c
 * @brief Sensorless FOC observer implementation.
 *
 * Layer: B  Dependencies: motor_observer.h.
 *
 * Ls stored in mH → H conversion: divide by 1000.
 *   ls_h_q16 = ls_mh / 1000  in Q16.16.
 *   At init time: int64_t ls_h_raw = (int64_t)ls_mh_q16 / 1000;
 *   Ls: mH->H / 1000
 *
 * lambda_m stored in mWb -> Wb: divide by 1000.
 *   lambda_m: mWb->Wb / 1000
 *
 * Flux integrator internal scale:
 *   psi stored × 2^15 to allow sub-LSB precision in Q16.16.
 *   psi_a (q31_t) = true_psi_wb × 65536.0 × 32768.0 (Q32.15 scale).
 *   Extracted as Q16.16 by: psi_q16 = (q16_t)(psi >> 15).
 *
 * Omega→angle integration (angle_t):
 *   Δθ = ω_e · Ts   [rad_e/sample]
 *   angle_t step = round(ω_e × Ts / (2π) × 2^32)
 *                = q16_mul(omega_q16, ts_over_2pi_q16) as angle increment.
 *   Pre-computed at init: ts_over_2pi = Q16(Ts / (2π)).
 *
 * Mechanical-rad/s decision (§7.2):
 *   The PLL's authoritative persisted speed state is obs->omega_m
 *   (MECHANICAL rad/s, Q16.16). Electrical speed ω̂_e = ω̂_m × pole_pairs is
 *   computed transiently with a 64-bit intermediate only at the angle-
 *   increment computation (this file) and the back-EMF getter
 *   (motor_observer.h). No electrical-rad/s q16_t is ever persisted, so
 *   motors >50 krpm at high pole counts (ω_e beyond ±32767 rad/s) do not
 *   saturate the exported/stored state.
 */

#include "foc/motor_observer.h"

/* 2π in Q16.16 */
#define TWO_PI_Q16  Q16(6.28318530717958647692)
/* 1/(2π) in Q16.16 */
#define INV_TWO_PI_Q16 Q16(0.15915494309189533576)

/* Flux integrator scale: psi_raw = psi_wb_q16 << 15. */
#define PSI_SCALE_SHIFT 15

foc_status_t foc_obs_init(foc_obs_t                *obs,
                           const foc_motor_params_t *params,
                           const foc_tuning_t       *tuning,
                           q16_t                     ts_s)
{
    if (!obs || !params || !tuning || ts_s <= 0) return FOC_EINVAL;

    /* Ls: mH→H ÷1000 (64-bit integer division at init) */
    int64_t ls_h_raw = (int64_t)params->ls_mh / 1000LL;
    obs->ls_h_q16 = (q16_t)ls_h_raw;
    if (obs->ls_h_q16 == 0 && params->ls_mh > 0) {
        /* Very small Ls (< 1 mH); keep as fractional Q16.16. */
        obs->ls_h_q16 = q16_div(params->ls_mh, Q16(1000.0));
    }

    /* λm: mWb→Wb ÷1000 (64-bit integer division at init) */
    int64_t lm_wb_raw = (int64_t)params->lambda_m_mwb / 1000LL;
    obs->lambda_m_wb = (q16_t)lm_wb_raw;
    if (obs->lambda_m_wb == 0 && params->lambda_m_mwb > 0) {
        obs->lambda_m_wb = q16_div(params->lambda_m_mwb, Q16(1000.0));
    }

    obs->rs_q16  = params->rs_ohm;
    obs->ts_q16  = ts_s;
    obs->pll_kp  = tuning->pll_kp;
    /* pll_ki_ts = pll_ki × Ts */
    obs->pll_ki_ts = q16_mul(tuning->pll_ki, ts_s);
    obs->pole_pairs = params->pole_pairs;

    /* Initialise HPF on each flux channel */
    q16_t fs_hz = q16_div(Q16_ONE, ts_s);
    foc_hpf_init(&obs->hpf_a, tuning->hpf_bemf_hz, fs_hz);
    foc_hpf_init(&obs->hpf_b, tuning->hpf_bemf_hz, fs_hz);

    /* Speed and error LPF */
    foc_lpf_init(&obs->lpf_omega, tuning->lpf_speed_hz, fs_hz);
    foc_lpf_init(&obs->lpf_err, Q16(5.0), fs_hz); /* 5 Hz err LPF for confidence */

    obs->psi_a = 0;
    obs->psi_b = 0;
    obs->theta = 0u;
    obs->omega_m = 0;
    obs->conf  = 0;
    obs->v_prev.alpha = 0; obs->v_prev.beta = 0;
    obs->i_prev.alpha = 0; obs->i_prev.beta = 0;

    return FOC_OK;
}

void foc_obs_reset(foc_obs_t *obs, angle_t seed_theta, q16_t seed_omega_m)
{
    obs->theta   = seed_theta;
    obs->omega_m = seed_omega_m;
    /* Seed flux to match expected λm at the seed angle. */
    q15_t s, c;
    foc_sincos(seed_theta, &s, &c);
    /* ψα = λm·cos(θ),  ψβ = λm·sin(θ)  in Q16.16 Wb. */
    q16_t psi_a_q16 = q16_mul_q15(obs->lambda_m_wb, c);
    q16_t psi_b_q16 = q16_mul_q15(obs->lambda_m_wb, s);
    obs->psi_a = (q31_t)((int64_t)psi_a_q16 << PSI_SCALE_SHIFT);
    obs->psi_b = (q31_t)((int64_t)psi_b_q16 << PSI_SCALE_SHIFT);
    foc_hpf_reset(&obs->hpf_a, psi_a_q16);
    foc_hpf_reset(&obs->hpf_b, psi_b_q16);
    foc_lpf_reset(&obs->lpf_omega, seed_omega_m);
    obs->conf = Q16_HALF;
}

void foc_obs_step(foc_obs_t       *obs,
                  const foc_ab_t  *i_ab,
                  const foc_ab_t  *v_ab_prev)
{
    /* ------------------------------------------------------------------
     * Step 1: Flux integrator update
     *   ψ[n] += (v_prev − Rs·i)·Ts − Ls·Δi
     * All computations in Q16.16, then scaled into q31_t integrator.
     * ------------------------------------------------------------------ */
    q16_t ts = obs->ts_q16;

    /* (v_prev - Rs·i)  per channel */
    q16_t v_minus_ri_a = (q16_t)((int64_t)v_ab_prev->alpha
                                  - (int64_t)q16_mul(obs->rs_q16, i_ab->alpha));
    q16_t v_minus_ri_b = (q16_t)((int64_t)v_ab_prev->beta
                                  - (int64_t)q16_mul(obs->rs_q16, i_ab->beta));

    /* (v - Ri)·Ts */
    q16_t vri_ts_a = q16_mul(v_minus_ri_a, ts);
    q16_t vri_ts_b = q16_mul(v_minus_ri_b, ts);

    /* Ls·Δi */
    q16_t di_a = (q16_t)((int64_t)i_ab->alpha - (int64_t)obs->i_prev.alpha);
    q16_t di_b = (q16_t)((int64_t)i_ab->beta  - (int64_t)obs->i_prev.beta);
    q16_t ls_di_a = q16_mul(obs->ls_h_q16, di_a);
    q16_t ls_di_b = q16_mul(obs->ls_h_q16, di_b);

    /* Δψ = (v-Ri)·Ts - Ls·Δi */
    q16_t dpsi_a = (q16_t)((int64_t)vri_ts_a - (int64_t)ls_di_a);
    q16_t dpsi_b = (q16_t)((int64_t)vri_ts_b - (int64_t)ls_di_b);

    /* Accumulate into q31_t integrator (PSI_SCALE_SHIFT precision boost) */
    obs->psi_a += (q31_t)((int64_t)dpsi_a << PSI_SCALE_SHIFT);
    obs->psi_b += (q31_t)((int64_t)dpsi_b << PSI_SCALE_SHIFT);

    /* Extract Q16.16 flux for HPF input */
    q16_t psi_a_q16 = (q16_t)(obs->psi_a >> PSI_SCALE_SHIFT);
    q16_t psi_b_q16 = (q16_t)(obs->psi_b >> PSI_SCALE_SHIFT);

    /* HPF drift removal */
    q16_t psi_a_hpf = foc_hpf_step(&obs->hpf_a, psi_a_q16);
    q16_t psi_b_hpf = foc_hpf_step(&obs->hpf_b, psi_b_q16);

    /* ------------------------------------------------------------------
     * Step 2: Angle from flux
     * ------------------------------------------------------------------ */
    angle_t theta_flux = foc_atan2(psi_b_hpf, psi_a_hpf);

    /* ------------------------------------------------------------------
     * Step 3: PLL update
     *   err = wrap(θ_flux − θ̂)   [signed, via int32_t subtraction — angle_t
     *                              wraps naturally, unaffected by rpm]
     *   ω̂_m += pll_ki_ts·err / pole_pairs   (mechanical-rad/s persisted state)
     *   ω̂_e  = ω̂_m × pole_pairs             (transient, 64-bit, for angle update)
     *   θ̂   += (ω̂_e + pll_kp·err)·Ts
     *
     * Mechanical-rad/s decision (§7.2): obs->omega_m is the ONLY persisted
     * speed state (q16_t). ω̂_e never occupies a q16_t/int32 variable that
     * could overflow at >50 krpm — it exists only as the int64_t
     * `omega_e_64` intermediate below, at this one conversion point.
     * ------------------------------------------------------------------ */
    int32_t err_raw = (int32_t)(theta_flux - obs->theta);
    /* err_raw in angle_t units; convert to Q16.16 rad (electrical, since the
     * flux/PLL angle error is inherently an electrical-angle quantity):
     * err_rad_q16 = (int32_t)((int64_t)err_raw * TWO_PI_Q16 >> 32)
     * Range proof: err_raw ≤ 2^31 angle_t, TWO_PI_Q16 ≈ 412316 Q16;
     *   product >> 32 fits in int32 (≈ ±2π rad as Q16). */
    q16_t err_rad = (q16_t)(((int64_t)err_raw * (int64_t)TWO_PI_Q16) >> 32);

    /* ω̂_m += (pll_ki_ts·err) / pole_pairs
     * pll_ki_ts·err is an electrical-rad/s-equivalent increment (the PLL
     * error term is inherently electrical); dividing by pole_pairs converts
     * the integral update into the mechanical domain before accumulation,
     * so the persisted state never carries electrical magnitude. */
    uint8_t pp = (obs->pole_pairs != 0u) ? obs->pole_pairs : 1u;
    q16_t domega_e = q16_mul(obs->pll_ki_ts, err_rad);
    q16_t domega_m = (q16_t)((int64_t)domega_e / (int32_t)pp);
    int64_t new_omega_m = (int64_t)obs->omega_m + (int64_t)domega_m;
    if (new_omega_m > (int64_t)Q16_MAX) new_omega_m = (int64_t)Q16_MAX;
    if (new_omega_m < (int64_t)(int32_t)Q16_MIN) new_omega_m = (int64_t)(int32_t)Q16_MIN;
    obs->omega_m = (q16_t)new_omega_m;

    /* θ̂ += (ω̂_e + kp·err) · Ts
     * ω̂_e = ω̂_m × pole_pairs — computed transiently in a 64-bit intermediate,
     * NOT stored in a q16_t (this is the one place electrical speed may
     * legitimately exceed the Q16.16 range at very high rpm/pole-count; the
     * 64-bit path is immune, and only the angle_t increment — which wraps
     * naturally — is derived from it).
     * Δθ_rad = (ω̂_e + kp·err) × Ts  [rad, kept in int64_t]
     * Δθ_angle = Δθ_rad / (2π) × 2^32
     */
    int64_t omega_e_64 = (int64_t)obs->omega_m * (int64_t)pp;
    q16_t kp_err = q16_mul(obs->pll_kp, err_rad);
    int64_t omega_total64 = omega_e_64 + (int64_t)kp_err;
    q16_t delta_theta_rad = (q16_t)(((int64_t)omega_total64 * (int64_t)ts) >> 16);
    /* angle_t increment = delta_theta_rad × (1/(2π)) × 2^32
     * = delta_theta_rad * INV_TWO_PI_Q16 (Q16.16 fraction) << 16 */
    int64_t delta_ang64 = (int64_t)q16_mul(delta_theta_rad, INV_TWO_PI_Q16);
    delta_ang64 <<= 16; /* convert Q16.16 fraction to angle_t units */
    obs->theta += (angle_t)(int32_t)delta_ang64;

    /* ------------------------------------------------------------------
     * Step 4: Filtered speed output (mechanical rad/s)
     * ------------------------------------------------------------------ */
    obs->omega_m = foc_lpf_step(&obs->lpf_omega, obs->omega_m);

    /* ------------------------------------------------------------------
     * Step 5: Confidence metric
     *   |ψ| vs λm, and LPF'd |pll_err|.
     * ------------------------------------------------------------------ */
    /* |ψ|² = ψa² + ψb²  — avoid q16_sqrt in hot path for confidence.
     * Use ratio: conf = min(|ψ|, λm) / max(|ψ|, λm)  [0,1]
     * Approximate |ψ| by max(|ψa|, |ψb|) × 1.0 + min × 0.414 (L∞ norm approximation).
     * Simpler and faster: use |ψa| + |ψb| / 2 as crude magnitude.
     * This is task-rate quality info; not cycle-critical.
     */
    q16_t abs_pa = q16_abs(psi_a_hpf);
    q16_t abs_pb = q16_abs(psi_b_hpf);
    /* psi_mag_approx = (|ψa| + |ψb|) × 0.7071 ≈ L2-L1 approximation */
    int64_t psi_sum = (int64_t)abs_pa + (int64_t)abs_pb;
    if (psi_sum > (int64_t)Q16_MAX) psi_sum = (int64_t)Q16_MAX;
    q16_t psi_mag = q16_mul((q16_t)psi_sum, Q16(0.7071067811865476));

    q16_t lm = obs->lambda_m_wb;
    q16_t conf;
    if (lm <= 0 || psi_mag <= 0) {
        conf = 0;
    } else {
        q16_t num = q16_min(psi_mag, lm);
        q16_t den = q16_max(psi_mag, lm);
        conf = q16_div(num, den); /* [0,1] Q16.16 */
    }
    /* LPF confidence to reduce noise sensitivity. */
    foc_lpf_step(&obs->lpf_err, q16_abs(err_rad));
    obs->conf = conf;

    /* Store state for next cycle. */
    obs->i_prev = *i_ab;
    /* Note: v_ab_prev storage is caller's responsibility (stored in foc_ctrl). */
}
