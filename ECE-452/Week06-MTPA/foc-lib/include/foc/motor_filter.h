/**
 * @file motor_filter.h
 * @brief First-order IIR low-pass and high-pass filters in fixed point.
 *
 * Layer: A  Dependencies: motor_types.h, motor_math.h.
 *
 * LPF formula (bilinear / EWM form):
 *   alpha = (2π·fc·Ts) / (1 + 2π·fc·Ts),  computed at init in fixed point.
 *   y[n] = y[n-1] + alpha·(x[n] − y[n-1])
 *   State stored as Q1.31 to avoid limit cycles.
 *
 * HPF formula:
 *   y[n] = alpha·(y[n-1] + x[n] − x[n-1])
 *   alpha = 1 / (1 + 2π·fc·Ts)
 *   States: state_in (prev x), state_out (prev y), both Q1.31.
 */
#ifndef FOC_MOTOR_FILTER_H
#define FOC_MOTOR_FILTER_H

#include "motor_types.h"
#include "motor_math.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Low-pass filter context
 * ---------------------------------------------------------------------- */
typedef struct {
    q31_t  state;       /**< Filter state (Q1.31 accumulator, internal scale) */
    q16_t  alpha;       /**< alpha = 2π·fc·Ts / (1 + 2π·fc·Ts)  Q16.16 */
    q16_t  one_m_alpha; /**< 1 − alpha  Q16.16 */
} foc_lpf_t;

/** @brief Initialise LPF.
 *  @param ctx       Filter context (caller-allocated).
 *  @param cutoff_hz Cutoff frequency [Hz]  Q16.16.
 *  @param sample_hz Sample rate     [Hz]  Q16.16.
 *  @return FOC_OK or FOC_EINVAL if cutoff_hz ≥ sample_hz/2.
 */
foc_status_t foc_lpf_init(foc_lpf_t *ctx, q16_t cutoff_hz, q16_t sample_hz);

/** @brief Single filter step.  [ISR] safe.
 *  @param ctx Context (must be initialised).
 *  @param x   Input sample  Q16.16.
 *  @return    Filtered output  Q16.16.
 */
q16_t foc_lpf_step(foc_lpf_t *ctx, q16_t x);

/** @brief Reset filter state to a known value.
 *  @param value  Initial output (and state seed)  Q16.16.
 */
void foc_lpf_reset(foc_lpf_t *ctx, q16_t value);

/* -------------------------------------------------------------------------
 * High-pass filter context
 * ---------------------------------------------------------------------- */
typedef struct {
    q31_t  state_in;  /**< Previous input  (Q1.31 internal scale) */
    q31_t  state_out; /**< Previous output (Q1.31 internal scale) */
    q16_t  alpha;     /**< alpha = 1 / (1 + 2π·fc·Ts)  Q16.16    */
} foc_hpf_t;

/** @brief Initialise HPF.
 *  @param ctx       Filter context.
 *  @param cutoff_hz Cutoff frequency [Hz]  Q16.16.
 *  @param sample_hz Sample rate     [Hz]  Q16.16.
 *  @return FOC_OK or FOC_EINVAL.
 */
foc_status_t foc_hpf_init(foc_hpf_t *ctx, q16_t cutoff_hz, q16_t sample_hz);

/** @brief Single HPF step.  [ISR] safe.  */
q16_t foc_hpf_step(foc_hpf_t *ctx, q16_t x);

/** @brief Reset HPF state.  */
void foc_hpf_reset(foc_hpf_t *ctx, q16_t value);

#ifdef __cplusplus
}
#endif

#endif /* FOC_MOTOR_FILTER_H */
