/**
 * @file motor_filter.c
 * @brief First-order IIR LPF and HPF implementation.
 *
 * Layer: A  Dependencies: motor_filter.h → motor_types.h, motor_math.h.
 *
 * Alpha computation (init-time, 64-bit integer math only):
 *   Let wc_ts = 2π · fc · Ts = 2π · fc / fs.
 *   alpha_LPF = wc_ts / (1 + wc_ts).
 *
 *   We compute wc_ts in Q16.16 using 64-bit intermediates at init:
 *     wc_ts_q16 = (2π × fc_q16 × inv_fs_q16) >> 16
 *     where 2π ≈ Q16(6.28318...).
 *   alpha = q16_div(wc_ts_q16, Q16_ONE + wc_ts_q16).
 *
 * State scaling:
 *   state (q31_t) = output_q16 << 15.
 *   This keeps 15 extra bits of state precision to avoid limit cycles,
 *   while the output extracted by >> 15 is Q16.16.
 */

#include "foc/motor_filter.h"

/* 2π in Q16.16 */
#define TWO_PI_Q16  Q16(6.28318530717958647692)

/* State scale shift: state = output_q16 << STATE_SHIFT */
#define STATE_SHIFT 15

/* =========================================================================
 * Low-pass filter
 * ====================================================================== */

foc_status_t foc_lpf_init(foc_lpf_t *ctx, q16_t cutoff_hz, q16_t sample_hz)
{
    if (ctx == NULL || cutoff_hz <= 0 || sample_hz <= 0) {
        return FOC_EINVAL;
    }
    /* Nyquist guard: fc < fs/2 */
    if (cutoff_hz >= (sample_hz >> 1)) {
        return FOC_EINVAL;
    }

    /* wc_ts = 2π · fc / fs  in Q16.16
     * = TWO_PI_Q16 * cutoff_hz / sample_hz
     * Use 64-bit multiply then divide to keep precision.
     * Both are Q16.16, so intermediate: (TWO_PI * fc) >> 16, then / fs.
     */
    int64_t wc_num = (int64_t)TWO_PI_Q16 * (int64_t)cutoff_hz; /* Q32.32 */
    wc_num >>= 16; /* Q16.16 */
    q16_t wc_ts = q16_div((q16_t)wc_num, sample_hz);

    /* alpha = wc_ts / (1 + wc_ts) */
    int64_t denom64 = (int64_t)Q16_ONE + (int64_t)wc_ts;
    if (denom64 > (int64_t)Q16_MAX) denom64 = (int64_t)Q16_MAX;
    ctx->alpha = q16_div(wc_ts, (q16_t)denom64);

    /* one_m_alpha = Q16_ONE - alpha (saturating sub) */
    int64_t oma = (int64_t)Q16_ONE - (int64_t)ctx->alpha;
    ctx->one_m_alpha = (q16_t)oma; /* alpha <= 1 so no overflow */

    ctx->state = 0;
    return FOC_OK;
}

q16_t foc_lpf_step(foc_lpf_t *ctx, q16_t x)
{
    /* y[n] = (1-α)·y[n-1] + α·x[n]
     * state = y[n-1] << STATE_SHIFT
     * y_prev_q16 = state >> STATE_SHIFT
     * Compute in Q16.16 then scale back.
     */
    q16_t y_prev = (q16_t)(ctx->state >> STATE_SHIFT);
    /* alpha·x */
    q16_t ax = q16_mul(ctx->alpha, x);
    /* (1-alpha)·y_prev */
    q16_t omy = q16_mul(ctx->one_m_alpha, y_prev);
    /* y_new = ax + omy */
    int64_t y_new_64 = (int64_t)ax + (int64_t)omy;
    q16_t y_new;
    if (y_new_64 > (int64_t)Q16_MAX) y_new = Q16_MAX;
    else if (y_new_64 < (int64_t)(int32_t)Q16_MIN) y_new = Q16_MIN;
    else y_new = (q16_t)y_new_64;

    ctx->state = (q31_t)((int64_t)y_new << STATE_SHIFT);
    return y_new;
}

void foc_lpf_reset(foc_lpf_t *ctx, q16_t value)
{
    ctx->state = (q31_t)((int64_t)value << STATE_SHIFT);
}

/* =========================================================================
 * High-pass filter
 * ====================================================================== */

foc_status_t foc_hpf_init(foc_hpf_t *ctx, q16_t cutoff_hz, q16_t sample_hz)
{
    if (ctx == NULL || cutoff_hz <= 0 || sample_hz <= 0) {
        return FOC_EINVAL;
    }
    if (cutoff_hz >= (sample_hz >> 1)) {
        return FOC_EINVAL;
    }

    /* wc_ts same as LPF. */
    int64_t wc_num = (int64_t)TWO_PI_Q16 * (int64_t)cutoff_hz;
    wc_num >>= 16;
    q16_t wc_ts = q16_div((q16_t)wc_num, sample_hz);

    /* alpha = 1 / (1 + wc_ts) */
    int64_t denom64 = (int64_t)Q16_ONE + (int64_t)wc_ts;
    if (denom64 > (int64_t)Q16_MAX) denom64 = (int64_t)Q16_MAX;
    ctx->alpha = q16_div(Q16_ONE, (q16_t)denom64);

    ctx->state_in  = 0;
    ctx->state_out = 0;
    return FOC_OK;
}

q16_t foc_hpf_step(foc_hpf_t *ctx, q16_t x)
{
    /* y[n] = alpha·(y[n-1] + x[n] − x[n-1])
     * state_in  = x[n-1] stored directly as q16_t (no shift — avoids
     *             int32 overflow when |x| is close to Q16 full scale).
     * state_out = y[n-1] << STATE_SHIFT (shifted for sub-LSB precision).
     */
    q16_t x_prev = (q16_t)ctx->state_in;
    q16_t y_prev = (q16_t)(ctx->state_out >> STATE_SHIFT);

    int64_t inner = (int64_t)y_prev + (int64_t)x - (int64_t)x_prev;
    q16_t inner_q;
    if (inner > (int64_t)Q16_MAX) inner_q = Q16_MAX;
    else if (inner < (int64_t)(int32_t)Q16_MIN) inner_q = Q16_MIN;
    else inner_q = (q16_t)inner;

    q16_t y_new = q16_mul(ctx->alpha, inner_q);

    ctx->state_in  = (q31_t)x;
    ctx->state_out = (q31_t)((int64_t)y_new << STATE_SHIFT);
    return y_new;
}

void foc_hpf_reset(foc_hpf_t *ctx, q16_t value)
{
    ctx->state_in  = 0;
    ctx->state_out = (q31_t)((int64_t)value << STATE_SHIFT);
}
