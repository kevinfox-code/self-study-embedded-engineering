/**
 * @file motor_ctrl.c
 * @brief Top-level orchestration implementation (§5.15, §3.3 pipeline).
 *
 * Layer: B  No HAL, no RTOS.
 *
 * Fast-loop pipeline (§3.3):
 *   1. Read ADC via hw_if → scale currents + vbus.
 *   2. Clarke → iαβ.
 *   3. Observer step (shadow or active, every powered fast loop).
 *   4. Angle selection: forced or observer.
 *   5. Park → idq.
 *   6. Current PI step → vdq.
 *   7. Circle limit.
 *   8. Inverse Park → vαβ.
 *   9. SVPWM → duty.
 *  10. Write PWM compares.
 *  11. Fast protections (OC_SW, OV_BUS).
 *
 * WCET target: ≤ 15 µs at 160 MHz.  Hot path avoids sqrt; uses q16_mul_ns
 * where range is proven.
 */

#include "foc/motor_ctrl.h"
#include <string.h>

/* -------------------------------------------------------------------------
 * Constants
 * ---------------------------------------------------------------------- */
/* Fast-loop rate: 20 kHz → Ts = 1/20000 s. */
#define FAST_LOOP_FS   Q16(20000.0)
#define FAST_LOOP_TS   Q16(1.0 / 20000.0)

/* Current task rate: 1 kHz */
#define CURRENT_TASK_FS Q16(1000.0)
#define CURRENT_TASK_TS Q16(1.0 / 1000.0)

/* Speed task rate: 1 kHz (same period) */
#define SPEED_TASK_FS CURRENT_TASK_FS

/* OL accel step per fast-loop cycle: accel [rad/s²] × Ts. */

/* -------------------------------------------------------------------------
 * Telemetry seq-lock write helpers
 * ---------------------------------------------------------------------- */
static void telem_write_start(foc_ctrl_t *ctx)  { ctx->telem.seq++; }
static void telem_write_end(foc_ctrl_t *ctx)    { ctx->telem.seq++; }

/* -------------------------------------------------------------------------
 * foc_ctrl_init
 * ---------------------------------------------------------------------- */
foc_status_t foc_ctrl_init(foc_ctrl_t                *ctx,
                            const foc_params_t        *p,
                            const foc_profile_table_t *profiles)
{
    if (!ctx || !p) return FOC_EINVAL;
    memset(ctx, 0, sizeof(*ctx));

    /* Double-buffer param setup. */
    ctx->params_a = *p;
    ctx->params_b = *p;
    ctx->params_active  = &ctx->params_a;
    ctx->params_staging = &ctx->params_b;
    ctx->profiles = profiles;

    /* Fault manager. */
    foc_faults_init(&ctx->faults, g_foc_fault_map_default, FOC_FAULT_NUM_ENTRIES);

    /* FOC core. */
    foc_status_t st = foc_core_init(&ctx->foc,
                                     &p->tuning.pi_id,
                                     &p->tuning.pi_iq,
                                     10u); /* default 10-tick min pulse */
    if (st != FOC_OK) return st;

    /* Observer. */
    st = foc_obs_init(&ctx->obs, &p->motor, &p->tuning, FAST_LOOP_TS);
    if (st != FOC_OK) return st;

    /* Speed PI. */
    foc_pi_init(&ctx->pi_speed, &p->tuning.pi_speed);

    /* SM. */
    foc_sm_init(&ctx->sm, p);

    /* Ident (using observer). */
    foc_ident_init(&ctx->ident, &p->motor, &p->tuning, &p->limits, &ctx->obs);

    /* Default calibration: gain + offsets from params (host mock). */
    ctx->cal.i_gain[0]      = Q16(5.0 / 2048.0);  /* 5 A FS, 12-bit biased mid */
    ctx->cal.i_gain[1]      = Q16(5.0 / 2048.0);
    ctx->cal.i_gain[2]      = Q16(5.0 / 2048.0);
    ctx->cal.i_offset_raw[0] = 2048;
    ctx->cal.i_offset_raw[1] = 2048;
    ctx->cal.i_offset_raw[2] = 2048;
    ctx->cal.i_polarity[0]  = 1;
    ctx->cal.i_polarity[1]  = 1;
    ctx->cal.i_polarity[2]  = 1;
    ctx->cal.vbus_gain      = Q16(60.0 / 4096.0); /* 60 V FS */
    ctx->cal.vbus_offset_v  = 0;
    ctx->cal.temp_gain      = 0;  /* temperature channel unused by default */
    ctx->cal.temp_offset_c  = 0;

    /* OL generator defaults from startup params (mechanical rad/s, §7.2). */
    ctx->ol.omega_accel  = q16_mul(p->startup.ol_accel_radps2_m, FAST_LOOP_TS);
    ctx->ol.omega_target = p->startup.ol_target_radps_m;

    /* Calibration not yet run; mark done=false. */
    ctx->cal_done = FOC_FALSE;
    ctx->cal_pass = FOC_FALSE;

    /* LUT sanity check. */
    if (foc_math_lut_check() != FOC_OK) {
        foc_faults_raise(&ctx->faults, FOC_FAULT_ADC_CAL_FAIL);
    }

    return FOC_OK;
}

/* -------------------------------------------------------------------------
 * Fast loop  [ISR]
 * ---------------------------------------------------------------------- */
void foc_ctrl_fast_loop(foc_ctrl_t *ctx)
{
    uint32_t t_start = hw_cycles_now();

    const foc_params_t *p = ctx->params_active;
    uint32_t actions = foc_sm_actions(&ctx->sm);
    foc_sm_step_fast(&ctx->sm, foc_faults_active(&ctx->faults));

    /* Always update actions after fast SM step. */
    actions = foc_sm_actions(&ctx->sm);

    /* ----------------------------------------------------------------
     * 1. Read ADC.
     * -------------------------------------------------------------- */
    hw_adc_raw_t adc_raw;
    hw_adc_read_raw(&adc_raw);

    /* ----------------------------------------------------------------
     * 2. Scale currents and vbus.
     * -------------------------------------------------------------- */
    foc_abc_t i_abc;
    hw_current_scale3(&adc_raw, &ctx->cal, &i_abc);
    q16_t vbus = hw_vbus_scale(adc_raw.vbus, &ctx->cal);

    /* LPF vbus — inline 1st-order: simple EWM. */
    /* Range proof for q16_mul_ns: vbus ≤ 60 V (Q16 ≤ 3932160), alpha ≈ Q16(0.001).
     * Product ~4096 << Q16 range — safe.  Use saturating mul for safety. */
    static q16_t s_vbus_filt = Q16(24.0); /* initialised to nominal */
    const q16_t VBUS_ALPHA = Q16(0.001);
    s_vbus_filt = (q16_t)((int64_t)s_vbus_filt
                  + q16_mul(VBUS_ALPHA, (q16_t)((int64_t)vbus - s_vbus_filt)));

    /* SW overcurrent check. */
    q16_t i_trip = p->limits.i_phase_trip_a;
    if (q16_abs(i_abc.a) > i_trip || q16_abs(i_abc.b) > i_trip ||
        q16_abs(i_abc.c) > i_trip) {
        foc_faults_raise_debounced(&ctx->faults, FOC_FAULT_OC_SW,
                                    (uint8_t)p->limits.oc_trip_count);
    }

    /* ----------------------------------------------------------------
     * 3. Clarke → iαβ.
     * -------------------------------------------------------------- */
    foc_ab_t i_ab;
    foc_clarke(&i_abc, &i_ab);

    /* ----------------------------------------------------------------
     * 4. Observer step (shadow or active, every powered fast loop).
     * -------------------------------------------------------------- */
    if (actions & (ACT_OBS_SHADOW | ACT_OBS_ACTIVE)) {
        foc_obs_step(&ctx->obs, &i_ab, &ctx->v_ab_prev);
    }

    /* ----------------------------------------------------------------
     * 5. Angle selection.
     * -------------------------------------------------------------- */
    angle_t theta_used;
    if (actions & ACT_ANGLE_FORCED) {
        /* Forced angle: use ol generator, update blend during TRANSITION. */
        theta_used = ctx->ol.theta_forced;
    } else {
        theta_used = foc_obs_get_theta(&ctx->obs);
    }

    /* ----------------------------------------------------------------
     * 6. Park → idq.
     * -------------------------------------------------------------- */
    q15_t sin_t, cos_t;
    foc_sincos(theta_used, &sin_t, &cos_t);
    foc_dq_t i_dq;
    foc_park(&i_ab, sin_t, cos_t, &i_dq);

    /* ----------------------------------------------------------------
     * 6a. Open-loop angle / speed integration.
     * -------------------------------------------------------------- */
    if (actions & ACT_ANGLE_FORCED) {
        /* Ramp omega_forced toward omega_target. */
        if (ctx->ol.omega_forced < ctx->ol.omega_target) {
            int64_t new_omega = (int64_t)ctx->ol.omega_forced
                                + (int64_t)ctx->ol.omega_accel;
            if (new_omega > (int64_t)ctx->ol.omega_target)
                new_omega = (int64_t)ctx->ol.omega_target;
            ctx->ol.omega_forced = (q16_t)new_omega;
        }
        /* Integrate forced angle.
         * Δθ_e = ω_forced_m × pole_pairs × Ts / (2π) × 2^32.
         * ω_forced is MECHANICAL rad/s (§7.2); the electrical angle
         * increment requires the pole_pairs multiplier, computed here as a
         * 64-bit intermediate so it never overflows a q16_t even at
         * >50 krpm / high pole counts. */
        uint8_t pp = p->motor.pole_pairs;
        int64_t omega_e_64 = (int64_t)ctx->ol.omega_forced * (int64_t)pp;
        q16_t delta_rad = (q16_t)((omega_e_64 * (int64_t)FAST_LOOP_TS) >> 16);
        q16_t delta_norm = q16_mul(delta_rad, Q16(0.15915494)); /* ÷2π */
        ctx->ol.theta_forced += (angle_t)((int64_t)delta_norm << 16);
        theta_used = ctx->ol.theta_forced;
        foc_sincos(theta_used, &sin_t, &cos_t);
        foc_park(&i_ab, sin_t, cos_t, &i_dq); /* Re-park with correct angle. */
    }

    /* ----------------------------------------------------------------
     * 7. Current PI and voltage limit.
     * -------------------------------------------------------------- */
    foc_dq_t i_ref = { ctx->id_ref, ctx->iq_ref };

    /* Apply tune override if active. */
    if ((actions & ACT_TUNE_ACTIVE) && !ctx->tune.done && !ctx->tune.aborted) {
        foc_tune_refs_t trefs;
        foc_tune_fast_step(&ctx->tune, NULL /* meas handled separately */, &ctx->obs, &trefs);
        if (trefs.override_refs) {
            i_ref.d = trefs.id_ref;
            i_ref.q = trefs.iq_ref;
        }
    }

    /* Apply ident override if active. */
    if (actions & ACT_IDENT_ACTIVE) {
        foc_meas_t meas_snap;
        meas_snap.i_dq   = i_dq;
        meas_snap.i_abc  = i_abc;
        meas_snap.vbus   = vbus;
        meas_snap.theta  = theta_used;
        meas_snap.omega  = foc_obs_get_omega(&ctx->obs);
        meas_snap.conf   = foc_obs_get_conf(&ctx->obs);
        foc_dq_t ident_ref;
        foc_ident_fast_step(&ctx->ident, &meas_snap, &ident_ref);
        i_ref = ident_ref;
    }

    if (actions & ACT_RUN_CURRENT_LOOP) {
        q16_t vdq_lim = foc_vdq_limit(s_vbus_filt,
                                       p->limits.modulation_max);
        foc_current_step(&ctx->foc, &i_dq, &i_ref, vdq_lim);
    } else {
        ctx->foc.vd_out = 0;
        ctx->foc.vq_out = 0;
    }

    /* ----------------------------------------------------------------
     * 8. Inverse Park → vαβ.
     * -------------------------------------------------------------- */
    foc_dq_t v_dq = { ctx->foc.vd_out, ctx->foc.vq_out };
    foc_ab_t v_ab;
    foc_ipark(&v_dq, sin_t, cos_t, &v_ab);
    ctx->v_ab_prev = v_ab; /* Store for next observer step. */
    ctx->foc.v_ab_out = v_ab;

    /* ----------------------------------------------------------------
     * 9. SVPWM.
     * ---------------------------------------------------------------- */
    uint16_t period = hw_pwm_period_ticks();
    if ((actions & ACT_PWM_ENABLE) && !(actions & ACT_ZERO_DUTY)) {
        foc_svpwm(&v_ab, s_vbus_filt, period, ctx->foc.min_pulse_ticks,
                   ctx->foc.duty);
        hw_pwm_set_compare(ctx->foc.duty[0], ctx->foc.duty[1], ctx->foc.duty[2]);
    } else {
        /* Zero vector or PWM off. */
        uint16_t half = (uint16_t)(period >> 1);
        hw_pwm_set_compare(half, half, half);
    }

    /* ----------------------------------------------------------------
     * 10. OV_BUS fast protection.
     * -------------------------------------------------------------- */
    if (s_vbus_filt > p->limits.vbus_max_v) {
        foc_faults_raise(&ctx->faults, FOC_FAULT_OV_BUS);
        hw_pwm_outputs_disable();
    }

    /* ----------------------------------------------------------------
     * 11. Update telemetry (seq-lock).
     * -------------------------------------------------------------- */
    telem_write_start(ctx);
    ctx->telem.id     = i_dq.d;
    ctx->telem.iq     = i_dq.q;
    ctx->telem.vd     = ctx->foc.vd_out;
    ctx->telem.vq     = ctx->foc.vq_out;
    ctx->telem.theta  = theta_used;
    ctx->telem.omega  = foc_obs_get_omega(&ctx->obs);
    ctx->telem.conf   = foc_obs_get_conf(&ctx->obs);
    ctx->telem.vbus   = s_vbus_filt;
    ctx->telem.state  = foc_sm_state(&ctx->sm);
    ctx->telem.faults = foc_faults_active(&ctx->faults);
    uint32_t t_now = hw_cycles_now();
    uint32_t wcet  = t_now - t_start;
    if (wcet > ctx->fast_loop_wcet_max) ctx->fast_loop_wcet_max = wcet;
    ctx->telem.wcet_cycles = ctx->fast_loop_wcet_max;
    telem_write_end(ctx);
}

/* -------------------------------------------------------------------------
 * Current task step (1 kHz)
 * ---------------------------------------------------------------------- */
void foc_ctrl_current_task_step(foc_ctrl_t *ctx)
{
    const foc_params_t *p = ctx->params_active;

    /* I²t thermal proxy (§6.5). */
    foc_telemetry_t snap;
    foc_ctrl_get_telemetry(ctx, &snap);
    q16_t iq2 = q16_mul(snap.iq, snap.iq);
    ctx->i2t_acc = (q16_t)((int64_t)ctx->i2t_acc
                   + (int64_t)iq2
                   - q16_mul(ctx->i2t_acc, p->limits.i2t_leak));
    if (ctx->i2t_acc > p->limits.i2t_limit) {
        foc_faults_raise(&ctx->faults, FOC_FAULT_OT);
    }

    /* Ident slow step. */
    foc_ident_slow_step(&ctx->ident, ctx->ms_now);

    /* Apply pending tune gains. */
    if (ctx->tune.gains_pending) {
        ctx->tune.gains_pending = FOC_FALSE;
        /* (PI and observer parameter update would go here in the full implementation;
         *  omitted in skeleton — gains staged and acknowledged.) */
    }
}

/* -------------------------------------------------------------------------
 * Speed task step (1 kHz)
 * ---------------------------------------------------------------------- */
void foc_ctrl_speed_task_step(foc_ctrl_t *ctx)
{
    foc_sm_state_t state = foc_sm_state(&ctx->sm);
    if (state != FOC_SM_CLOSED_LOOP_RUN && state != FOC_SM_TUNE) return;

    foc_telemetry_t snap;
    foc_ctrl_get_telemetry(ctx, &snap);

    /* Speed ramp (simple: direct step for TUNE_SPEED, ramped for normal). */
    q16_t omega_err = (q16_t)((int64_t)ctx->speed_ref - snap.omega);
    q16_t iq_out = foc_pi_step(&ctx->pi_speed, omega_err);

    /* Rate limit. */
    iq_out = q16_clamp(iq_out,
                        ctx->params_active->limits.i_phase_max_a,
                        ctx->params_active->limits.i_phase_max_a);
    ctx->iq_ref = iq_out;
}

/* -------------------------------------------------------------------------
 * Supervisor step (100 Hz)
 * ---------------------------------------------------------------------- */
void foc_ctrl_supervisor_step(foc_ctrl_t *ctx, uint32_t ms)
{
    ctx->ms_now = ms;

    /* Drain command queue. */
    foc_bool_t got_cmd = FOC_FALSE;
    hw_crit_enter();
    if (ctx->cmd_valid) {
        ctx->sm_in.cmd   = ctx->cmd_pending;
        ctx->cmd_valid   = FOC_FALSE;
        got_cmd = FOC_TRUE;
    }
    hw_crit_exit();
    if (!got_cmd) {
        ctx->sm_in.cmd.op = FOC_CMD_NONE;
    }

    /* Build SM inputs. */
    ctx->sm_in.faults_active = foc_faults_active(&ctx->faults);
    ctx->sm_in.obs_conf      = foc_obs_get_conf(&ctx->obs);
    ctx->sm_in.obs_omega     = foc_obs_get_omega(&ctx->obs);
    ctx->sm_in.ol_omega      = ctx->ol.omega_forced;
    ctx->sm_in.ident_done    = ctx->ident.done;
    ctx->sm_in.ident_pass    = (ctx->ident.done && ctx->ident.result.quality.overall_pass) ? FOC_TRUE : FOC_FALSE;
    ctx->sm_in.cal_done      = ctx->cal_done;
    ctx->sm_in.cal_pass      = ctx->cal_pass;
    ctx->sm_in.tune_done     = foc_tune_done(&ctx->tune);
    ctx->sm_in.retry_allowed = foc_faults_retry_allowed(&ctx->faults);
    ctx->sm_in.ms_now        = ms;

    /* Feed command to SM. */
    foc_sm_input(&ctx->sm, &ctx->sm_in);

    /* Slow SM step. */
    foc_sm_step_slow(&ctx->sm, &ctx->sm_in);

    /* Tune slow step. */
    foc_tune_slow_step(&ctx->tune, ms);

    /* Fault timers. */
    foc_faults_tick(&ctx->faults, 10u); /* supervisor at 100 Hz = 10 ms */

    /* Healthy tick (if in CLOSED_LOOP_RUN). */
    if (foc_sm_state(&ctx->sm) == FOC_SM_CLOSED_LOOP_RUN) {
        foc_faults_healthy_tick(&ctx->faults, 10u);
    }

    /* CALIBRATE: perform mock calibration on host. */
    if (foc_sm_state(&ctx->sm) == FOC_SM_CALIBRATE && !ctx->cal_done) {
        /* On host, cal always passes. */
        ctx->cal_done = FOC_TRUE;
        ctx->cal_pass = FOC_TRUE;
    }

    /* Speed ref: accept from SM (stored via foc_ctrl_command → sm). */
    if (ctx->sm.run_pending && ctx->sm.speed_ref != 0) {
        ctx->speed_ref = ctx->sm.speed_ref;
    }

    /* Handle PWM enable/disable based on SM actions. */
    uint32_t actions = foc_sm_actions(&ctx->sm);
    if (actions & ACT_PWM_ENABLE) {
        hw_pwm_outputs_enable();
    } else if (!(actions & ACT_PWM_ENABLE)) {
        hw_pwm_outputs_disable();
    }

    /* OL generator: reset on OPEN_LOOP_START entry. */
    foc_sm_state_t state = foc_sm_state(&ctx->sm);
    static foc_sm_state_t prev_state = FOC_SM_IDLE;
    if (state == FOC_SM_OPEN_LOOP_START && prev_state != FOC_SM_OPEN_LOOP_START) {
        ctx->ol.theta_forced  = ctx->obs.theta; /* seed from observer */
        ctx->ol.omega_forced  = 0;
        foc_obs_reset(&ctx->obs, ctx->ol.theta_forced, 0);
    }
    prev_state = state;
}

/* -------------------------------------------------------------------------
 * Command
 * ---------------------------------------------------------------------- */
foc_status_t foc_ctrl_command(foc_ctrl_t *ctx, const foc_cmd_t *cmd)
{
    if (!ctx || !cmd) return FOC_EINVAL;
    hw_crit_enter();
    if (ctx->cmd_valid) {
        hw_crit_exit();
        return FOC_EBUSY;
    }
    ctx->cmd_pending = *cmd;
    ctx->cmd_valid   = FOC_TRUE;
    hw_crit_exit();
    return FOC_OK;
}

/* -------------------------------------------------------------------------
 * Telemetry read (seq-lock)
 * ---------------------------------------------------------------------- */
void foc_ctrl_get_telemetry(foc_ctrl_t *ctx, foc_telemetry_t *out)
{
    uint32_t seq0, seq1;
    do {
        seq0 = ctx->telem.seq;
        if ((seq0 & 1u) != 0u) continue;
        *out = ctx->telem;
        seq1 = ctx->telem.seq;
    } while (seq0 != seq1 || (seq0 & 1u) != 0u);
}

/* -------------------------------------------------------------------------
 * Apply params (double-buffered)
 * ---------------------------------------------------------------------- */
foc_status_t foc_ctrl_apply_params(foc_ctrl_t *ctx, const foc_params_t *p)
{
    if (!ctx || !p) return FOC_EINVAL;
    foc_sm_state_t state = foc_sm_state(&ctx->sm);
    if (state != FOC_SM_IDLE && state != FOC_SM_FAULT) return FOC_ENOTREADY;

    *ctx->params_staging = *p;
    /* Swap pointers in critical section. */
    hw_crit_enter();
    foc_params_t *tmp = ctx->params_active;
    ctx->params_active  = ctx->params_staging;
    ctx->params_staging = tmp;
    hw_crit_exit();
    return FOC_OK;
}
