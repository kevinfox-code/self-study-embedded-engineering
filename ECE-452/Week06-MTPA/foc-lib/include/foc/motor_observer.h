/**
 * @file motor_observer.h
 * @brief Sensorless rotor angle and speed estimator.
 *        Flux-linkage form with HPF drift mitigation and PLL tracking.
 *
 * Layer: B  Dependencies: motor_types.h, motor_math.h, motor_filter.h,
 *            motor_params.h.
 *
 * Algorithm (§5.11 authoritative spec):
 *   1. Flux estimate: ψ[n] += (v_applied_prev − Rs·i)·Ts − Ls·Δi
 *      with HPF (fc from tuning, default 2 Hz) to kill integrator drift.
 *   2. θ_flux = atan2(ψ_β, ψ_α)
 *   3. PLL: err = wrap(θ_flux − θ̂)   (angle_t subtraction — unaffected by rpm)
 *           ω̂_m += pll_ki·err·Ts / pole_pairs   (mechanical-rad/s state)
 *           θ̂   += (ω̂_e + pll_kp·err)·Ts   where ω̂_e = ω̂_m × pole_pairs
 *                                          (64-bit intermediate, computed on
 *                                          demand — never stored in q16_t)
 *   4. Speed output: ω̂_m LPF-filtered — exported MECHANICAL rad/s Q16.16.
 *   5. Confidence: |ψ| vs λm + LPF'd PLL residual.
 *
 * Mechanical-rad/s decision (§7.2): the PLL's authoritative persisted speed
 * state is mechanical rad/s (radps_q16_t). Electrical angle/speed is used
 * only transiently, computed with 64-bit intermediates, so motors >50 krpm
 * at high pole counts do not overflow Q16.16 (±32767 rad/s_e cap).
 *
 * Units note: Ls stored in mH → converted to H at init (÷1000).
 *             λm stored in mWb → converted to Wb at init (÷1000).
 *             Both conversions use 64-bit integer math at init only.
 */
#ifndef FOC_MOTOR_OBSERVER_H
#define FOC_MOTOR_OBSERVER_H

#include "motor_types.h"
#include "motor_math.h"
#include "motor_filter.h"
#include "motor_params.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Observer context
 * ---------------------------------------------------------------------- */
typedef struct {
    /* Flux integrator states (Q1.31 × 2^15 scale for precision) */
    q31_t    psi_a;   /**< Flux α [internal Q31 units; extracted to Q16.16 mWb] */
    q31_t    psi_b;   /**< Flux β [same]                                         */

    /* HPF for drift removal (α and β channels) */
    foc_hpf_t hpf_a;  /**< HPF on ψα */
    foc_hpf_t hpf_b;  /**< HPF on ψβ */

    /* PLL state */
    angle_t  theta;   /**< Estimated electrical angle angle_t                    */
    q16_t    omega_m; /**< Estimated MECHANICAL speed [rad/s_m] Q16.16 — the
                        *   PLL's authoritative persisted speed state. Electrical
                        *   ω̂_e is derived transiently (64-bit) as
                        *   ω̂_e = ω̂_m × pole_pairs, only at the angle-increment
                        *   and back-EMF computation points (§7.2). */

    /* Speed LPF and PLL error LPF */
    foc_lpf_t lpf_omega; /**< Speed LPF (tuning.lpf_speed_hz), mechanical rad/s */
    foc_lpf_t lpf_err;   /**< PLL error LPF (for confidence)  */

    /* Confidence [0,1] Q16.16 */
    q16_t    conf;

    /* Previous applied voltage (needed for flux update, §3.3 pipeline) */
    foc_ab_t v_prev; /**< vαβ from previous fast-loop cycle [V] Q16.16 */

    /* Previous current (for di/dt) */
    foc_ab_t i_prev; /**< iαβ from previous cycle [A] Q16.16 */

    /* Pre-computed at init: fixed-point plant parameters */
    q16_t    rs_q16;       /**< Rs [Ω]  Q16.16 */
    q16_t    ls_h_q16;     /**< Ls [H]  Q16.16 (converted from mH at init) */
    q16_t    lambda_m_wb;  /**< λm [Wb] Q16.16 (converted from mWb at init) */
    q16_t    ts_q16;       /**< Ts [s]  Q16.16 = 1/fs */
    q16_t    pll_kp;       /**< PLL Kp [rad/s]  Q16.16 */
    q16_t    pll_ki_ts;    /**< PLL Ki·Ts       Q16.16 */
    uint8_t  pole_pairs;   /**< Cached from params at init — used for the
                            *   mechanical<->electrical conversion points
                            *   (PLL angle update, EMF getter). §7.2. */
} foc_obs_t;

/* -------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------- */

/**
 * Initialise observer.
 * @param obs     Observer context (caller-allocated).
 * @param params  Motor plant parameters (rs, ls in mH, lambda_m in mWb).
 * @param tuning  Controller tuning (HPF fc, PLL gains, speed LPF).
 * @param ts_s    Sample period [s] as Q16.16 (= Q16(1.0/fs)).
 * @return FOC_OK or FOC_EINVAL.
 */
foc_status_t foc_obs_init(foc_obs_t                *obs,
                           const foc_motor_params_t *params,
                           const foc_tuning_t       *tuning,
                           q16_t                     ts_s);

/**
 * Single observer step.  [ISR]
 * Must be called every fast loop with the PREVIOUS cycle's applied voltage.
 * @param obs          Observer context.
 * @param i_ab         Measured αβ currents this cycle [A] Q16.16.
 * @param v_ab_prev    Applied αβ voltage from previous cycle [V] Q16.16.
 */
void foc_obs_step(foc_obs_t       *obs,
                  const foc_ab_t  *i_ab,
                  const foc_ab_t  *v_ab_prev);

/**
 * Reset observer state (seeded at TRANSITION from open-loop generator).
 * @param seed_theta  Seed angle (from forced angle generator).
 * @param seed_omega_m Seed speed, MECHANICAL rad/s Q16.16.
 */
void foc_obs_reset(foc_obs_t *obs, angle_t seed_theta, q16_t seed_omega_m);

/* -------------------------------------------------------------------------
 * Inline getters  [ISR]
 * ---------------------------------------------------------------------- */

static inline angle_t foc_obs_get_theta(const foc_obs_t *obs)
{
    return obs->theta;
}

/** Get estimated speed, MECHANICAL rad/s Q16.16 (§7.2). */
static inline q16_t foc_obs_get_omega(const foc_obs_t *obs)
{
    return obs->omega_m;
}

static inline q16_t foc_obs_get_conf(const foc_obs_t *obs)
{
    return obs->conf;
}

/**
 * Get estimated back-EMF in αβ frame (for TUNE_OBSERVER capture).
 * e_α = -λm·ω̂_e·sin(θ̂),  e_β = λm·ω̂_e·cos(θ̂)   (from ψ derivative approx.)
 * Back-EMF magnitude is proportional to ELECTRICAL speed, so ω̂_e is computed
 * transiently here (64-bit intermediate) from the persisted mechanical state:
 * ω̂_e = ω̂_m × pole_pairs. Never stored in a q16_t (§7.2).
 * Decision D-013 (from §5.21): expose e_α as e_a_est.
 */
static inline void foc_obs_get_emf_ab(const foc_obs_t *obs,
                                       q16_t *e_alpha, q16_t *e_beta)
{
    q15_t s, c;
    foc_sincos(obs->theta, &s, &c);

    /* ω̂_e = ω̂_m × pole_pairs, 64-bit intermediate — pole_pairs ≤ 255,
     * ω̂_m ≤ ~2^15 rad/s_m in Q16.16 → product safely fits int64_t, then
     * saturated to q16_t range before use (electrical speed can exceed
     * Q16.16 range at very high pole counts / rpm; the EMF getter is
     * task-rate/tune-only, so saturation here is an accepted display clamp,
     * not a control-path hazard). */
    int64_t omega_e_64 = (int64_t)obs->omega_m * (int64_t)obs->pole_pairs;
    if (omega_e_64 > (int64_t)Q16_MAX) omega_e_64 = (int64_t)Q16_MAX;
    if (omega_e_64 < (int64_t)(int32_t)Q16_MIN) omega_e_64 = (int64_t)(int32_t)Q16_MIN;
    q16_t omega_e = (q16_t)omega_e_64;

    /* λm·ω̂_e in Q16.16: q16_mul(lambda_m_wb, omega_e)
     * Range proof: λm [Wb] ≤ 0.1, ω̂_e ≤ 1000 rad/s_e (rated) → product ≤ 100 V,
     * fits Q16. Saturating q16_mul used since ω̂_e may be a saturated clamp
     * above; further saturation here is a documented display-path behavior. */
    q16_t lm_w = q16_mul(obs->lambda_m_wb, omega_e);
    *e_alpha = (q16_t)(-(int64_t)q16_mul_q15(lm_w, s));
    *e_beta  = q16_mul_q15(lm_w, c);
}

#ifdef __cplusplus
}
#endif

#endif /* FOC_MOTOR_OBSERVER_H */
