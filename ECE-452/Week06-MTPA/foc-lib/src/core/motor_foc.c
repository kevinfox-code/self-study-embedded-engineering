/**
 * @file motor_foc.c
 * @brief FOC transforms, current loop, and SVPWM implementation.
 *
 * Layer: B  Dependencies: motor_foc.h.
 *
 * Amplitude-invariant Clarke convention:
 *   α = ia
 *   β = (ia + 2·ib) / √3   (balanced 3-phase: ic = -(ia+ib))
 * This preserves amplitude (|v_ab| = |v_abc_peak|) which simplifies
 * voltage limit calculations.
 *
 * SVPWM (min-max injection / midpoint clamping):
 *   Va_ref = α (in V)
 *   Vb_ref = (−α + √3·β) / 2
 *   Vc_ref = (−α − √3·β) / 2
 *   v_zero = −(max(Va,Vb,Vc) + min(Va,Vb,Vc)) / 2   (midpoint)
 *   duty[x] = (V[x] + v_zero + vbus/2) / vbus × period
 */

#include "foc/motor_foc.h"

/* √3 constants in Q16.16 */
#define Q16_SQRT3       Q16(1.7320508075688772935)
#define Q16_1_OVER_SQRT3 Q16(0.5773502691896257645)
#define Q16_SQRT3_OVER2 Q16(0.8660254037844386468)
#define Q16_HALF        ((q16_t)0x00008000)

/* =========================================================================
 * Clarke transform
 * ====================================================================== */

void foc_clarke(const foc_abc_t *in, foc_ab_t *out)
{
    /* Amplitude-invariant 3-current Clarke:
     *   α = ia
     *   β = (ia + 2·ib) / √3
     * All in Q16.16.
     */
    out->alpha = in->a;
    /* (a + 2b) / √3 = (a + 2b) * (1/√3) */
    int64_t num = (int64_t)in->a + 2LL * (int64_t)in->b;
    /* num is Q16.16; multiply by 1/√3 (Q16.16) → q16_mul handles overflow. */
    /* Clamp num to Q16 range before mul */
    q16_t num_q;
    if (num > (int64_t)Q16_MAX) num_q = Q16_MAX;
    else if (num < (int64_t)(int32_t)Q16_MIN) num_q = Q16_MIN;
    else num_q = (q16_t)num;
    out->beta = q16_mul(num_q, Q16_1_OVER_SQRT3);
}

void foc_clarke_2(const foc_abc_t *in, foc_ab_t *out)
{
    /* 2-current form: reconstruct ic = -(ia+ib), then:
     *   α = ia
     *   β = (ib - ic) / √3 = (ib + ia + ib) / √3 = (ia + 2ib) / √3
     * Identical to 3-current form on balanced input. */
    foc_clarke(in, out);
}

/* =========================================================================
 * Park / inverse Park
 * ====================================================================== */

/* Saturating add/sub of two Q16.16 values from 64-bit intermediates. */
#define Q16_SAT64(v64) \
    (((v64) > (int64_t)Q16_MAX) ? Q16_MAX : \
     ((v64) < (int64_t)(int32_t)Q16_MIN) ? (q16_t)(int32_t)Q16_MIN : \
     (q16_t)(v64))

/* Park-specific multiply helper — see inline comments in park_mul(). */
static q16_t park_mul(q16_t a, q15_t b)
{
    /* Q16.16 × Q1.15 -> Q16.16 for Park/iPark transforms.
     * Standard >>15 shift.  The sin^2+cos^2 normalisation correction is applied
     * once per call in foc_ipark rather than per-multiply here.
     */
    int64_t prod = (int64_t)a * (int64_t)b;
    int64_t rounded;
    if (prod >= 0) {
        rounded = (prod + (int64_t)0x4000) >> 15;
    } else {
        rounded = -((-prod + (int64_t)0x4000) >> 15);
    }
    if (rounded > (int64_t)Q16_MAX) {
        FOC_SAT_HOOK("park_mul");
        return Q16_MAX;
    }
    if (rounded < (int64_t)(int32_t)Q16_MIN) {
        FOC_SAT_HOOK("park_mul");
        return Q16_MIN;
    }
    return (q16_t)rounded;
}

void foc_park(const foc_ab_t *ab, q15_t sin_th, q15_t cos_th, foc_dq_t *out)
{
    /* d =  α·cos + β·sin
     * q = -α·sin + β·cos  */
    int64_t d64 = (int64_t)park_mul(ab->alpha, cos_th) +
                  (int64_t)park_mul(ab->beta,  sin_th);
    int64_t q64 = (int64_t)park_mul(ab->beta,  cos_th) -
                  (int64_t)park_mul(ab->alpha, sin_th);
    out->d = Q16_SAT64(d64);
    out->q = Q16_SAT64(q64);
}

void foc_ipark(const foc_dq_t *dq, q15_t sin_th, q15_t cos_th, foc_ab_t *out)
{
    /* alpha = d*cos - q*sin
     * beta  = d*sin + q*cos */
    int64_t a64 = (int64_t)park_mul(dq->d, cos_th) -
                  (int64_t)park_mul(dq->q, sin_th);
    int64_t b64 = (int64_t)park_mul(dq->d, sin_th) +
                  (int64_t)park_mul(dq->q, cos_th);

    /* Correct for LUT interpolation error: sin^2 + cos^2 may deviate from 32768^2
     * due to independent quantization of each trig value.  This causes a systematic
     * amplitude deficit in the Park + iPark round-trip.
     * Correction: out = out * 32768^2 / (sin^2 + cos^2)
     *           = out * (1 + deficit / (sin^2 + cos^2))
     * where deficit = 32768^2 - (sin^2 + cos^2).
     * Approximation: use deficit / 32768^2 (slightly different denominator) for a
     * single >>30 shift instead of division.  Error in approximation is second-order
     * and negligible (< 1 LSB for typical deficit values). */
    int64_t sc2 = (int64_t)sin_th * (int64_t)sin_th +
                  (int64_t)cos_th * (int64_t)cos_th;
    int64_t deficit = (int64_t)32768 * (int64_t)32768 - sc2;
    if (deficit != 0) {
        /* correction = a64 * deficit >> 30  (since 32768^2 = 2^30) */
        a64 += (a64 * deficit) >> 30;
        b64 += (b64 * deficit) >> 30;
    }

    out->alpha = Q16_SAT64(a64);
    out->beta  = Q16_SAT64(b64);
}

/* =========================================================================
 * Current step
 * ====================================================================== */

void foc_current_step(foc_core_t *ctx,
                      const foc_dq_t *i_meas,
                      const foc_dq_t *i_ref,
                      q16_t vdq_limit)
{
    /* Dynamic PI limits:  vd priority — vq limited to remaining headroom. */
    foc_pi_set_limits(&ctx->pi_id, -vdq_limit, vdq_limit);
    /* Run Id PI. */
    q16_t vd = foc_pi_step(&ctx->pi_id, i_ref->d - i_meas->d);
    ctx->vd_out = vd;

    /* Remaining vq budget: sqrt(vdq_limit² − vd²) — approximate with
     * linear clamp to avoid sqrt in fast path.
     * Safe conservative: vq_max = sqrt(lim² - vd²).
     * Use: vq_lim = q16_sqrt(q16_mul(vdq_limit,vdq_limit) - q16_mul(vd,vd))
     * but q16_sqrt is documented as task-rate only.
     * Approximation (used in fast loop): vq_lim = vdq_limit − |vd|.
     * This is a conservative circle (octagonal) approximation.
     * TODO(v2): replace with circle limit using stored reciprocal vbus.
     */
    q16_t vd_abs = q16_abs(vd);
    int64_t vq_lim64 = (int64_t)vdq_limit - (int64_t)vd_abs;
    q16_t vq_lim;
    if (vq_lim64 <= 0) {
        vq_lim = 0;
    } else {
        vq_lim = (vq_lim64 > (int64_t)Q16_MAX) ? Q16_MAX : (q16_t)vq_lim64;
    }
    foc_pi_set_limits(&ctx->pi_iq, -vq_lim, vq_lim);
    q16_t vq = foc_pi_step(&ctx->pi_iq, i_ref->q - i_meas->q);
    ctx->vq_out = vq;
}

/* =========================================================================
 * SVPWM (min-max injection)
 * ====================================================================== */

void foc_svpwm(const foc_ab_t *v_ab,
               q16_t vbus,
               uint16_t period_ticks,
               uint16_t min_ticks,
               uint16_t duty_out[3])
{
    if (vbus <= 0) {
        duty_out[0] = duty_out[1] = duty_out[2] = period_ticks >> 1;
        return;
    }

    /* Convert αβ to three-phase references:
     *   Va = Vα
     *   Vb = (−Vα + √3·Vβ) / 2
     *   Vc = (−Vα − √3·Vβ) / 2
     */
    q16_t va = v_ab->alpha;
    /* √3·β in Q16.16 */
    q16_t sqrt3_beta = q16_mul(Q16_SQRT3, v_ab->beta);
    /* Range proof for q16_mul_ns: alpha and beta are voltages bounded by vbus/√3
     * (enforced by foc_current_step circle limit), so their Q16 values are
     * representable and (√3·β) is within Q16 range after one mul — use saturating. */
    int64_t vb64 = ((int64_t)(-v_ab->alpha) + (int64_t)sqrt3_beta) / 2LL;
    int64_t vc64 = ((int64_t)(-v_ab->alpha) - (int64_t)sqrt3_beta) / 2LL;
    q16_t vb, vc;
    vb = (vb64 > Q16_MAX) ? Q16_MAX : (vb64 < (int32_t)Q16_MIN) ? Q16_MIN : (q16_t)vb64;
    vc = (vc64 > Q16_MAX) ? Q16_MAX : (vc64 < (int32_t)Q16_MIN) ? Q16_MIN : (q16_t)vc64;

    /* Min-max injection (zero-sequence) to maximise linear modulation range. */
    q16_t v_max = q16_max(va, q16_max(vb, vc));
    q16_t v_min = q16_min(va, q16_min(vb, vc));
    /* v_zero = −(v_max + v_min) / 2 */
    int64_t v_zero_64 = -((int64_t)v_max + (int64_t)v_min) / 2LL;
    q16_t v_zero = (q16_t)v_zero_64;

    /* Shifted references: V[x] + v_zero */
    int64_t va_s = (int64_t)va + (int64_t)v_zero;
    int64_t vb_s = (int64_t)vb + (int64_t)v_zero;
    int64_t vc_s = (int64_t)vc + (int64_t)v_zero;

    /* Normalise to [0, period_ticks]:
     * duty = (Vx_shifted + vbus/2) / vbus × period_ticks
     *      = (Vx_shifted/vbus + 0.5) × period_ticks
     *
     * Compute inv_vbus = Q16_ONE / vbus (one divide per call, as §7.3 allows).
     * Then: duty = (Vx_s × inv_vbus + Q16_HALF) × period / Q16_ONE
     */
    q16_t inv_vbus = q16_div(Q16_ONE, vbus);

    /* Helper lambda in C99: use a local macro. */
#define SCALE_DUTY(vs) \
    do { \
        q16_t _norm = q16_mul((q16_t)(vs), inv_vbus); \
        int64_t _duty64 = ((int64_t)(_norm) + (int64_t)Q16_HALF) * (int64_t)period_ticks; \
        _duty64 >>= 16; \
        if (_duty64 < (int64_t)min_ticks) _duty64 = (int64_t)min_ticks; \
        if (_duty64 > (int64_t)(period_ticks - min_ticks)) _duty64 = (int64_t)(period_ticks - min_ticks); \
        /* final clamp to [0, period] */ \
        if (_duty64 < 0) _duty64 = 0; \
        if (_duty64 > (int64_t)period_ticks) _duty64 = (int64_t)period_ticks; \
    } while(0)

    {
        q16_t norm_a = q16_mul((q16_t)va_s, inv_vbus);
        int64_t d64 = ((int64_t)norm_a + (int64_t)Q16_HALF) * (int64_t)period_ticks;
        d64 >>= 16;
        if (d64 < (int64_t)min_ticks) d64 = (int64_t)min_ticks;
        if (d64 > (int64_t)(period_ticks - min_ticks)) d64 = (int64_t)(period_ticks - min_ticks);
        if (d64 < 0) d64 = 0;
        if (d64 > (int64_t)period_ticks) d64 = (int64_t)period_ticks;
        duty_out[0] = (uint16_t)d64;
    }
    {
        q16_t norm_b = q16_mul((q16_t)vb_s, inv_vbus);
        int64_t d64 = ((int64_t)norm_b + (int64_t)Q16_HALF) * (int64_t)period_ticks;
        d64 >>= 16;
        if (d64 < (int64_t)min_ticks) d64 = (int64_t)min_ticks;
        if (d64 > (int64_t)(period_ticks - min_ticks)) d64 = (int64_t)(period_ticks - min_ticks);
        if (d64 < 0) d64 = 0;
        if (d64 > (int64_t)period_ticks) d64 = (int64_t)period_ticks;
        duty_out[1] = (uint16_t)d64;
    }
    {
        q16_t norm_c = q16_mul((q16_t)vc_s, inv_vbus);
        int64_t d64 = ((int64_t)norm_c + (int64_t)Q16_HALF) * (int64_t)period_ticks;
        d64 >>= 16;
        if (d64 < (int64_t)min_ticks) d64 = (int64_t)min_ticks;
        if (d64 > (int64_t)(period_ticks - min_ticks)) d64 = (int64_t)(period_ticks - min_ticks);
        if (d64 < 0) d64 = 0;
        if (d64 > (int64_t)period_ticks) d64 = (int64_t)period_ticks;
        duty_out[2] = (uint16_t)d64;
    }
#undef SCALE_DUTY
}

/* =========================================================================
 * Init / reset
 * ====================================================================== */

foc_status_t foc_core_init(foc_core_t *ctx,
                            const foc_pi_cfg_t *pi_id_cfg,
                            const foc_pi_cfg_t *pi_iq_cfg,
                            uint16_t min_pulse)
{
    if (ctx == NULL || pi_id_cfg == NULL || pi_iq_cfg == NULL) return FOC_EINVAL;
    foc_status_t st;
    st = foc_pi_init(&ctx->pi_id, pi_id_cfg);
    if (st != FOC_OK) return st;
    st = foc_pi_init(&ctx->pi_iq, pi_iq_cfg);
    if (st != FOC_OK) return st;
    ctx->vd_out = 0;
    ctx->vq_out = 0;
    ctx->v_ab_out.alpha = 0;
    ctx->v_ab_out.beta  = 0;
    ctx->duty[0] = ctx->duty[1] = ctx->duty[2] = 0u;
    ctx->mod_index = 0;
    ctx->min_pulse_ticks = min_pulse;
    return FOC_OK;
}

void foc_core_reset(foc_core_t *ctx, q16_t vd_preload, q16_t vq_preload)
{
    foc_pi_reset(&ctx->pi_id, vd_preload);
    foc_pi_reset(&ctx->pi_iq, vq_preload);
    ctx->vd_out = vd_preload;
    ctx->vq_out = vq_preload;
}
