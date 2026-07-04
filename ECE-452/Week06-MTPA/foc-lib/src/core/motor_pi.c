/**
 * @file motor_pi.c
 * @brief PI controller implementation.
 *
 * Layer: A  Dependencies: motor_pi.h → motor_types.h; motor_math.h.
 *
 * Integrator state encoding:
 *   integ (q31_t, used as q16_t) stores the accumulated integral directly
 *   in Q16.16 output units.  No internal scaling is applied.
 *   This avoids int32 overflow that occurred when the previous
 *   implementation stored integ × 2^16.
 */

#include "foc/motor_pi.h"
#include "foc/motor_math.h"

foc_status_t foc_pi_init(foc_pi_t *ctx, const foc_pi_cfg_t *cfg)
{
    if (ctx == NULL || cfg == NULL) return FOC_EINVAL;
    if (cfg->ts_hz == 0)            return FOC_EINVAL;
    if (cfg->out_min > cfg->out_max) return FOC_EINVAL;

    ctx->kp      = cfg->kp;
    /* ki_ts = ki / fs   (Q16.16 / Q16.16 → Q16.16) */
    ctx->ki_ts   = q16_div(cfg->ki, cfg->ts_hz);
    ctx->kaw     = cfg->kaw;
    ctx->out_min = cfg->out_min;
    ctx->out_max = cfg->out_max;
    ctx->integ   = 0;
    ctx->out_last = 0;
    return FOC_OK;
}

q16_t foc_pi_step(foc_pi_t *ctx, q16_t err)
{
    /* Integrator is stored directly in Q16.16. */
    q16_t integ_val = (q16_t)ctx->integ;

    /* u_raw = kp·err + integ_val */
    q16_t p_term = q16_mul(ctx->kp, err);
    int64_t u_raw64 = (int64_t)p_term + (int64_t)integ_val;
    q16_t u_raw;
    if (u_raw64 > (int64_t)Q16_MAX) u_raw = Q16_MAX;
    else if (u_raw64 < (int64_t)(int32_t)Q16_MIN) u_raw = Q16_MIN;
    else u_raw = (q16_t)u_raw64;

    /* u_sat = clamp(u_raw, out_min, out_max) */
    q16_t u_sat = q16_clamp(u_raw, ctx->out_min, ctx->out_max);
    ctx->out_last = u_sat;

    /* Update integrator:
     *   integ[n+1] = clamp(integ[n] + ki_ts·e + kaw·(u_sat − u_raw))
     * All in Q16.16.
     */
    q16_t ki_contrib = q16_mul(ctx->ki_ts, err);
    q16_t aw_diff    = (q16_t)((int64_t)u_sat - (int64_t)u_raw);
    q16_t kaw_term   = q16_mul(ctx->kaw, aw_diff);

    int64_t new_integ = (int64_t)integ_val + (int64_t)ki_contrib + (int64_t)kaw_term;

    /* Clamp integrator to output range (prevents windup when kaw=0). */
    if (new_integ > (int64_t)ctx->out_max) new_integ = (int64_t)ctx->out_max;
    if (new_integ < (int64_t)ctx->out_min) new_integ = (int64_t)ctx->out_min;

    ctx->integ = (q31_t)new_integ;
    return u_sat;
}

void foc_pi_reset(foc_pi_t *ctx, q16_t preload_out)
{
    /* Bumpless: with err=0, u_raw = kp*0 + integ = integ.
     * First output after reset = integ = clamped preload. */
    q16_t clamped  = q16_clamp(preload_out, ctx->out_min, ctx->out_max);
    ctx->integ     = (q31_t)clamped;
    ctx->out_last  = clamped;
}

void foc_pi_set_limits(foc_pi_t *ctx, q16_t lo, q16_t hi)
{
    ctx->out_min = lo;
    ctx->out_max = hi;
    /* Re-clamp integrator to new limits immediately. */
    if (ctx->integ > (q31_t)hi) ctx->integ = (q31_t)hi;
    if (ctx->integ < (q31_t)lo) ctx->integ = (q31_t)lo;
}
