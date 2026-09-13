/**
 * @file motor_tune.c
 * @brief Tuning mode implementation (§5.21 normative contracts).
 *
 * Layer: B  No HAL, no RTOS.
 *
 * TUNE_TORQUE sequence (§5.21):
 *   per rep: hi for tq_step_ms → lo for tq_step_ms → 0 for tq_step_ms.
 *   Capture: {iq*, iq_meas, vq, id_meas}.
 *
 * TUNE_SPEED sequence:
 *   Square wave sp_min_pct·max_speed ↔ sp_max_pct·max_speed,
 *   dwell sp_dwell_ms, sp_reps half-cycles.  No ramp.
 *   Capture: {ω*, ω̂_filt, iq*, iq_meas}.
 *
 * TUNE_OBSERVER:
 *   Hold constant speed obs_speed_radps_m (MECHANICAL rad/s) for obs_capture_ms.
 *   Capture: {i_a_meas, e_a_est, θ̂, conf}.
 */

#include "foc/motor_tune.h"
#include <string.h>

/* -------------------------------------------------------------------------
 * Capture helpers
 * ---------------------------------------------------------------------- */
static void cap_write(foc_capture_t *cap,
                      int32_t ch0, int32_t ch1, int32_t ch2, int32_t ch3)
{
    if (cap->decim_cnt > 0u) { cap->decim_cnt--; return; }
    cap->decim_cnt = (cap->decim > 1u) ? (uint16_t)(cap->decim - 1u) : 0u;

    /* Seq-lock write (even → odd → write → even). */
    cap->seq++;  /* now odd */
    uint32_t idx = cap->write_idx;
    cap->data[idx][0] = ch0;
    cap->data[idx][1] = ch1;
    cap->data[idx][2] = ch2;
    cap->data[idx][3] = ch3;
    cap->write_idx = (uint32_t)(idx + 1u) % FOC_TUNE_CAP_DEPTH;
    cap->seq++;  /* back to even */
}

/* -------------------------------------------------------------------------
 * Init
 * ---------------------------------------------------------------------- */
foc_status_t foc_tune_init(foc_tune_t            *ctx,
                            foc_tune_mode_t        mode,
                            const foc_tune_cfg_t  *cfg,
                            const foc_params_t    *params,
                            const motor_limits_t  *limits)
{
    if (!ctx || !cfg || !params || !limits) return FOC_EINVAL;

    memset(ctx, 0, sizeof(*ctx));
    ctx->mode   = mode;
    ctx->cfg    = *cfg;
    ctx->params = params;
    ctx->limits = limits;

    /* Clamp cfg against limits (§5.21: FOC_EINVAL on violation). */
    if (ctx->cfg.tq_i_hi_a > limits->i_phase_max_a) return FOC_EINVAL;
    if (ctx->cfg.tq_i_lo_a > limits->i_phase_max_a) return FOC_EINVAL;
    if (ctx->cfg.tq_step_ms == 0u) ctx->cfg.tq_step_ms = 50u;
    if (ctx->cfg.tq_reps == 0u)    ctx->cfg.tq_reps = 4u;
    if (ctx->cfg.sp_reps == 0u)    ctx->cfg.sp_reps = 6u;
    if (ctx->cfg.decim == 0u)      ctx->cfg.decim = 1u;

    ctx->cap.decim     = cfg->decim;
    ctx->cap.decim_cnt = 0u;
    ctx->cap.write_idx = 0u;
    ctx->cap.read_idx  = 0u;
    ctx->cap.seq       = 0u;

    ctx->phase           = 0u;
    ctx->rep             = 0u;
    ctx->phase_start_ms  = 0u;
    ctx->done            = FOC_FALSE;
    ctx->aborted         = FOC_FALSE;
    ctx->gains_pending   = FOC_FALSE;

    return FOC_OK;
}

/* -------------------------------------------------------------------------
 * Fast step  [ISR]
 * ---------------------------------------------------------------------- */
void foc_tune_fast_step(foc_tune_t           *ctx,
                         const foc_meas_t     *meas,
                         const foc_obs_t      *obs,
                         foc_tune_refs_t      *out)
{
    if (ctx->done || ctx->aborted) {
        out->id_ref = 0; out->iq_ref = 0; out->omega_ref = 0;
        out->override_refs = FOC_FALSE;
        return;
    }

    out->override_refs = FOC_TRUE;
    out->id_ref    = 0;
    out->iq_ref    = 0;
    out->omega_ref = 0;

    switch (ctx->mode) {
    case TUNE_TORQUE:
        /* Phase 0: hi, phase 1: lo, phase 2: zero — per §5.21. */
        switch (ctx->phase % 3u) {
        case 0u: out->iq_ref = ctx->cfg.tq_i_hi_a; break;
        case 1u: out->iq_ref = ctx->cfg.tq_i_lo_a; break;
        default: out->iq_ref = 0; break;
        }
        out->id_ref = 0;
        /* Capture: {iq*, iq_meas, vq(=0 here, caller fills), id_meas}. */
        cap_write(&ctx->cap,
                  out->iq_ref,
                  meas->i_dq.q,
                  0, /* vq not available here — motor_ctrl fills if needed */
                  meas->i_dq.d);
        break;

    case TUNE_SPEED:
        /* Alternating min/max speed, no ramp. */
        out->omega_ref = ((ctx->rep % 2u) == 0u) ?
                q16_mul(ctx->params->motor.max_speed_radps_m, ctx->cfg.sp_max_pct) :
                q16_mul(ctx->params->motor.max_speed_radps_m, ctx->cfg.sp_min_pct);
        /* Capture: {ω*, ω̂_filt, iq*, iq_meas}. */
        cap_write(&ctx->cap,
                  out->omega_ref,
                  meas->omega,
                  ctx->last_refs.iq_ref,
                  meas->i_dq.q);
        break;

    case TUNE_OBSERVER:
        /* Hold constant speed; capture {i_a, e_a_est, θ̂, conf}. */
        out->omega_ref = ctx->cfg.obs_speed_radps_m;
        if (obs != NULL) {
            q16_t e_alpha = 0, e_beta = 0;
            foc_obs_get_emf_ab(obs, &e_alpha, &e_beta);
            cap_write(&ctx->cap,
                      meas->i_abc.a,        /* i_a_meas */
                      e_alpha,              /* e_a_est  */
                      (int32_t)obs->theta,  /* θ̂        */
                      obs->conf);           /* conf     */
        }
        break;

    default:
        break;
    }

    ctx->last_refs = *out;
}

/* -------------------------------------------------------------------------
 * Slow step
 * ---------------------------------------------------------------------- */
void foc_tune_slow_step(foc_tune_t *ctx, uint32_t ms_now)
{
    if (ctx->done || ctx->aborted) return;
    ctx->ms_now = ms_now;

    uint32_t el = ms_now - ctx->phase_start_ms;

    switch (ctx->mode) {
    case TUNE_TORQUE: {
        /* Each phase is tq_step_ms long.  3 phases per rep. */
        if (el >= (uint32_t)ctx->cfg.tq_step_ms) {
            ctx->phase++;
            ctx->phase_start_ms = ms_now;
            if (ctx->phase >= 3u) {
                ctx->phase = 0u;
                ctx->rep++;
                if (ctx->rep >= (uint32_t)ctx->cfg.tq_reps) {
                    ctx->done = FOC_TRUE;
                }
            }
        }
        break;
    }
    case TUNE_SPEED: {
        if (el >= (uint32_t)ctx->cfg.sp_dwell_ms) {
            ctx->rep++;
            ctx->phase_start_ms = ms_now;
            if (ctx->rep >= (uint32_t)ctx->cfg.sp_reps) {
                ctx->done = FOC_TRUE;
            }
        }
        break;
    }
    case TUNE_OBSERVER: {
        if (el >= (uint32_t)ctx->cfg.obs_capture_ms) {
            ctx->done = FOC_TRUE;
        }
        break;
    }
    default:
        break;
    }
}

/* -------------------------------------------------------------------------
 * Abort
 * ---------------------------------------------------------------------- */
void foc_tune_abort(foc_tune_t *ctx)
{
    ctx->aborted        = FOC_TRUE;
    ctx->last_refs.id_ref    = 0;
    ctx->last_refs.iq_ref    = 0;
    ctx->last_refs.omega_ref = 0;
    ctx->last_refs.override_refs = FOC_FALSE;
}

/* -------------------------------------------------------------------------
 * Gain staging
 * ---------------------------------------------------------------------- */
foc_status_t foc_tune_set_gains(foc_tune_t           *ctx,
                                  const foc_tune_gains_t *gains,
                                  foc_sm_state_t          sm_state)
{
    if (sm_state != FOC_SM_TUNE) return FOC_ENOTREADY;
    ctx->staged_gains  = *gains;
    ctx->gains_pending = FOC_TRUE;
    return FOC_OK;
}

/* -------------------------------------------------------------------------
 * Capture drain
 * ---------------------------------------------------------------------- */
foc_status_t foc_tune_capture_read(foc_tune_t *ctx,
                                    int32_t (*dst)[FOC_TUNE_CAP_CH],
                                    uint32_t max, uint32_t *n_out)
{
    if (!ctx || !dst || !n_out) return FOC_EINVAL;
    *n_out = 0u;
    uint32_t n = 0u;

    while (n < max) {
        /* Seq-lock consistent read: retry if write was in progress. */
        uint32_t seq0, seq1;
        uint32_t ridx = ctx->cap.read_idx;
        if (ridx == ctx->cap.write_idx) break; /* no new data */
        do {
            seq0 = ctx->cap.seq;
            if ((seq0 & 1u) != 0u) continue; /* odd = write in progress */
            dst[n][0] = ctx->cap.data[ridx][0];
            dst[n][1] = ctx->cap.data[ridx][1];
            dst[n][2] = ctx->cap.data[ridx][2];
            dst[n][3] = ctx->cap.data[ridx][3];
            seq1 = ctx->cap.seq;
        } while (seq0 != seq1 || (seq0 & 1u) != 0u);

        ctx->cap.read_idx = (ridx + 1u) % FOC_TUNE_CAP_DEPTH;
        n++;
    }
    *n_out = n;
    return FOC_OK;
}
