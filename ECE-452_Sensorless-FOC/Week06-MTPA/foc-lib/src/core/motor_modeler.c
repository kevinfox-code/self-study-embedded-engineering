/**
 * @file motor_modeler.c
 * @brief Motor profile matching and parameter resolution.
 *
 * Layer: B.  No float, no HAL.
 *
 * Match metric: log-scaled normalised distance over (Rs, Ls, λm).
 *   For each quantity Q, dist_Q = |log(measured/profile)|.
 *   Score = exp(-sum of weighted distances), mapped to [0,1].
 *   We approximate log ratio as |Q_meas - Q_prof| / max(Q_meas, Q_prof)
 *   in fixed-point (avoids log; adequate for a match metric).
 *
 * PI gain recomputation (§5.13 scaling rules):
 *   kp = 2π·f_bw·Ls  (in H)
 *   ki = 2π·f_bw·Rs
 *   f_bw_current = 1000 Hz
 *   Computed at resolve time using 64-bit int math.
 */

#include "foc/motor_modeler.h"
#include <string.h>

/* -------------------------------------------------------------------------
 * Default profile table (≥3 profiles per §5.13)
 * ---------------------------------------------------------------------- */
static const foc_motor_profile_t s_profiles[] = {
    {
        "SmallGimbal_24V",
        /* params */
        { Q16(1.0),  Q16(1.0),  Q16(5.0),   7u, Q16(1.0),  Q16(733.0) },
        /* tuning — pi_id, pi_iq, pi_speed, obs gains, pll, LPFs, HPF */
        {
            { Q16(3.14), Q16(3141.0), Q16(20000.0), Q16(-24.0), Q16(24.0), Q16(0.1) },
            { Q16(3.14), Q16(3141.0), Q16(20000.0), Q16(-24.0), Q16(24.0), Q16(0.1) },
            { Q16(0.05), Q16(5.0),    Q16(1000.0),  Q16(-1.0),  Q16(1.0),  Q16(0.2) },
            Q16(1.0), Q16(1.0), Q16(500.0), Q16(5000.0), Q16(20.0), Q16(10.0), Q16(2.0)
        },
        /* startup */
        { Q16(0.3), 200u, Q16(0.5), Q16(100.0), Q16(110.0), Q16(0.5), 50u, 100u, Q16(66.0) },
        /* limits */
        FOC_LIMITS_DEFAULT
    },
    {
        "MidDrone_24V",
        { Q16(0.3),  Q16(0.2),  Q16(15.0),  7u, Q16(5.0),  Q16(2094.0) },
        {
            { Q16(1.26), Q16(1884.0), Q16(20000.0), Q16(-24.0), Q16(24.0), Q16(0.1) },
            { Q16(1.26), Q16(1884.0), Q16(20000.0), Q16(-24.0), Q16(24.0), Q16(0.1) },
            { Q16(0.02), Q16(2.0),    Q16(1000.0),  Q16(-5.0),  Q16(5.0),  Q16(0.2) },
            Q16(1.0), Q16(1.0), Q16(500.0), Q16(5000.0), Q16(20.0), Q16(10.0), Q16(2.0)
        },
        { Q16(1.0), 300u, Q16(2.0), Q16(200.0), Q16(300.0), Q16(0.5), 50u, 100u, Q16(180.0) },
        FOC_LIMITS_DEFAULT
    },
    {
        "EBike_48V",
        { Q16(0.05), Q16(0.05), Q16(50.0), 23u, Q16(20.0), Q16(3455.0) },
        {
            { Q16(0.31), Q16(15.7), Q16(20000.0), Q16(-48.0), Q16(48.0), Q16(0.05) },
            { Q16(0.31), Q16(15.7), Q16(20000.0), Q16(-48.0), Q16(48.0), Q16(0.05) },
            { Q16(0.01), Q16(1.0),  Q16(1000.0),  Q16(-20.0), Q16(20.0), Q16(0.2)  },
            Q16(1.0), Q16(1.0), Q16(300.0), Q16(3000.0), Q16(10.0), Q16(5.0), Q16(1.0)
        },
        { Q16(3.0), 400u, Q16(5.0), Q16(500.0), Q16(600.0), Q16(0.5), 80u, 150u, Q16(360.0) },
        FOC_LIMITS_DEFAULT
    },
    {
        /* user_motor_01 — normative reference motor (§5.13).
         * Rs = 0.149 Ω, Ls = 96 mH, Ke = 0.982 V_peak/Hz(elec)
         *   -> lambda_m = Ke/(2*pi) = 0.982/6.28319 ~= 0.15634 Wb = 156.34 mWb.
         * pole_pairs = 1 -> electrical speed == mechanical speed.
         * 12 V bus class, 7 A current limit (rated_current_a = 7.0 A).
         *
         * max_speed_radps_m derivation: keep peak line-neutral back-EMF
         * under ~80% of the 12 V bus for headroom:
         *   0.8 * 12 V = 9.6 V (peak) usable back-EMF budget
         *   f_max = 9.6 V / 0.982 (V_peak/Hz) ~= 9.77 Hz
         *   omega_max_e = 2*pi*f_max ~= 61.4 rad/s_e = 61.4 rad/s_m (pp=1)
         * Rounded down to a conservative Q16(60.0) rad/s_m.
         */
        "user_motor_01",
        { Q16(0.149), Q16(96.0), Q16(156.34), 1u, Q16(7.0), Q16(60.0) },
        {
            /* pi_id / pi_iq: kp = 2*pi*f_bw*Ls, ki = 2*pi*f_bw*Rs.
             * Ls = 96 mH is very high inductance; the library default
             * f_bw_current = 1000 Hz would require kp ~= 603 (kp*7A far
             * beyond any bus), so f_bw_current is reduced to 60 Hz here:
             *   kp = 2*pi*60*0.096   ~= 36.19
             *   ki = 2*pi*60*0.149   ~= 56.17
             * (kp*7A ~= 253 V is the full-scale-error artifact of the PI
             * structure, not a steady-state operating voltage — actual
             * vd/vq stay within the circle limit via foc_vdq_limit.) */
            { Q16(36.19), Q16(56.17), Q16(20000.0), Q16(-12.0), Q16(12.0), Q16(0.1) },
            { Q16(36.19), Q16(56.17), Q16(20000.0), Q16(-12.0), Q16(12.0), Q16(0.1) },
            /* pi_speed: scaled between MidDrone(5A)/EBike(20A) patterns for
             * 7 A rated current. */
            { Q16(0.015), Q16(7.0), Q16(1000.0), Q16(-7.0), Q16(7.0), Q16(0.2) },
            /* obs gains g1/g2: unity, matching pattern of other entries. */
            Q16(1.0), Q16(1.0),
            /* pll kp/ki: scaled down from SmallGimbal (pp=7, max_e=733 rad/s)
             * to this motor's max_e ~= 60 rad/s (pp=1):
             *   pll_kp ~= 500 * (60/733) ~= 40.9 -> 40.0
             *   pll_ki ~= 5000 * (60/733) ~= 409.2 -> 400.0 */
            Q16(40.0), Q16(400.0),
            /* lpf_speed/lpf_vbus/hpf_bemf: this is a slow (~9.5 Hz elec at
             * max speed), high-inductance motor, so cutoffs are reduced
             * well below the fundamental. */
            Q16(5.0), Q16(10.0), Q16(0.5)
        },
        {
            /* align_current_a: ~20% of 7 A rated. */
            Q16(1.5), 300u,
            /* ol_current_a: ~30% of 7 A rated. */
            Q16(2.0),
            /* ol_accel_radps2_m / ol_target_radps_m: this motor's max
             * mechanical speed is only ~60 rad/s_m, so the open-loop
             * ramp target is kept well below that (§ task spec 10-15
             * rad/s_m); accel/target ratio ~0.9 follows SmallGimbal_24V's
             * pattern (a similarly low-speed profile). */
            Q16(10.8), Q16(12.0),
            Q16(0.5), 50u, 100u,
            /* min_run_speed_radps_m: 0.6 * ol_target, matching the ratio
             * used by all three existing profiles. */
            Q16(7.2)
        },
        /* limits: 7.0 A phase max / 8.0 A trip, 12 V bus class (9-15 V). */
        {
            Q16(7.0),   /* i_phase_max_a  7.0 A  */
            Q16(8.0),   /* i_phase_trip_a 8.0 A  */
            Q16(9.0),   /* vbus_min_v     9 V    */
            Q16(15.0),  /* vbus_max_v     15 V   */
            Q16(80.0),  /* temp_max       80 degC */
            Q16(0.90),  /* modulation_max 0.90   */
            Q16(10.0),  /* i2t_limit      10 A^2*s */
            Q16(0.001), /* i2t_leak per sample   */
            3u          /* oc_trip_count         */
        }
    }
};

static const foc_profile_table_t s_table = {
    s_profiles,
    (uint32_t)(sizeof(s_profiles) / sizeof(s_profiles[0]))
};

const foc_profile_table_t *foc_modeler_default_table(void)
{
    return &s_table;
}

/* -------------------------------------------------------------------------
 * Match
 * ---------------------------------------------------------------------- */
const foc_motor_profile_t *foc_modeler_match(
        const foc_profile_table_t  *table,
        const foc_motor_params_t   *measured,
        q16_t                      *score_out)
{
    if (!table || !measured || table->count == 0u) {
        if (score_out) *score_out = 0;
        return NULL;
    }

    const foc_motor_profile_t *best = NULL;
    q16_t best_score = -1;

    for (uint32_t i = 0u; i < table->count; i++) {
        const foc_motor_params_t *pp = &table->entries[i].params;

        /* Normalised |meas - prof| / max(meas, prof) per quantity. */
        q16_t rs_m = q16_max(Q16(0.001), measured->rs_ohm);
        q16_t rs_p = q16_max(Q16(0.001), pp->rs_ohm);
        q16_t d_rs = q16_div(q16_abs((q16_t)((int64_t)rs_m - rs_p)),
                              q16_max(rs_m, rs_p));

        q16_t ls_m = q16_max(Q16(0.001), measured->ls_mh);
        q16_t ls_p = q16_max(Q16(0.001), pp->ls_mh);
        q16_t d_ls = q16_div(q16_abs((q16_t)((int64_t)ls_m - ls_p)),
                              q16_max(ls_m, ls_p));

        q16_t lm_m = q16_max(Q16(0.001), measured->lambda_m_mwb);
        q16_t lm_p = q16_max(Q16(0.001), pp->lambda_m_mwb);
        q16_t d_lm = q16_div(q16_abs((q16_t)((int64_t)lm_m - lm_p)),
                              q16_max(lm_m, lm_p));

        /* Equal weights, sum then invert: score = 1 - mean_dist, clamped. */
        int64_t dist_sum = (int64_t)d_rs + (int64_t)d_ls + (int64_t)d_lm;
        q16_t mean_dist = (q16_t)(dist_sum / 3LL);
        q16_t score;
        int64_t s64 = (int64_t)Q16_ONE - (int64_t)mean_dist;
        score = (s64 < 0) ? 0 : (q16_t)s64;

        if (score > best_score) {
            best_score = score;
            best = &table->entries[i];
        }
    }

    if (score_out) *score_out = (best_score < 0) ? 0 : best_score;
    return best;
}

/* -------------------------------------------------------------------------
 * Resolve
 * ---------------------------------------------------------------------- */
foc_status_t foc_modeler_resolve(
        const foc_profile_table_t  *table,
        const foc_ident_result     *ident_res,
        foc_param_source_t          policy,
        uint32_t                    profile_idx,
        foc_params_t               *out)
{
    if (!out || !ident_res) return FOC_EINVAL;

    const foc_motor_params_t *meas = &ident_res->params;

    switch (policy) {
    case FOC_POLICY_MEASURED_ONLY:
        out->motor  = *meas;
        break;

    case FOC_POLICY_PROFILE_ONLY:
        if (!table || profile_idx >= table->count) return FOC_EINVAL;
        out->motor   = table->entries[profile_idx].params;
        out->tuning  = table->entries[profile_idx].tuning;
        out->startup = table->entries[profile_idx].startup;
        out->limits  = table->entries[profile_idx].limits;
        return FOC_OK;

    case FOC_POLICY_MEASURED_THEN_PROFILE: {
        q16_t score = 0;
        const foc_motor_profile_t *best = foc_modeler_match(table, meas, &score);
        if (best && score >= MODELER_MIN_SCORE) {
            out->motor   = *meas; /* use measured plant */
            out->tuning  = best->tuning;
            out->startup = best->startup;
            out->limits  = best->limits;
        } else {
            out->motor = *meas;
        }
        break;
    }

    case FOC_POLICY_PROFILE_TUNING_MEASURED_PLANT: {
        /* Use measured Rs/Ls/λm but recompute PI gains from them.
         * kp = 2π·f_bw·Ls [H], ki = 2π·f_bw·Rs.
         * f_bw = 1000 Hz.  All in 64-bit integer math (no float). */
        out->motor = *meas;
        q16_t score = 0;
        const foc_motor_profile_t *best = foc_modeler_match(table, meas, &score);
        if (best) {
            out->tuning  = best->tuning;
            out->startup = best->startup;
            out->limits  = best->limits;
        }
        /* Recompute current PI gains.
         * ls_H = ls_mh / 1000 (mH → H, 64-bit divide at init).
         * kp = 2π × 1000 Hz × ls_H  (Q16.16)
         *    = Q16(6283.18) × ls_H / 1000
         * ki = 2π × 1000 Hz × rs    (Q16.16)
         *    = Q16(6283.18) × rs
         */
        int64_t ls_h_raw = (int64_t)meas->ls_mh / 1000LL;
        q16_t ls_h = (q16_t)ls_h_raw;
        q16_t kp_new = q16_mul(Q16(6283.185307), ls_h);
        q16_t ki_new = q16_mul(Q16(6283.185307), meas->rs_ohm);
        out->tuning.pi_id.kp = kp_new;
        out->tuning.pi_id.ki = ki_new;
        out->tuning.pi_iq.kp = kp_new;
        out->tuning.pi_iq.ki = ki_new;
        break;
    }

    default:
        return FOC_EINVAL;
    }

    return FOC_OK;
}
