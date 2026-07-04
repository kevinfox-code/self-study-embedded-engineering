/**
 * @file motor_pi.h
 * @brief PI controller with back-calculation anti-windup, output saturation,
 *        bumpless re-init.
 *
 * Layer: A  Dependencies: motor_types.h only.
 *
 * Discrete form:
 *   u_raw[n] = kp·e[n] + integ[n]
 *   integ[n+1] = integ[n] + ki_ts·e[n] + kaw·(u_sat[n] - u_raw[n])
 *   u_sat[n]   = clamp(u_raw[n], out_min, out_max)
 *
 * Anti-windup: back-calculation, Kaw term feeds the difference between
 *   saturated and unsaturated output back into the integrator.
 *
 * Integrator: q31_t accumulator at <<16 scale (i.e., integrator stores
 *   Q16.16 value × 2^16 = Q32.0, to preserve resolution through many small
 *   ki_ts steps).  Extracted via >> 16 for output calculation.
 */
#ifndef FOC_MOTOR_PI_H
#define FOC_MOTOR_PI_H

#include "motor_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Configuration (supplied to foc_pi_init)
 * ---------------------------------------------------------------------- */
typedef struct {
    q16_t kp;       /**< Proportional gain   Q16.16           */
    q16_t ki;       /**< Integral gain [1/s] Q16.16           */
    q16_t ts_hz;    /**< Inverse of sample time = fs [Hz] Q16.16  (ki_ts = ki/fs) */
    q16_t out_min;  /**< Output lower limit  Q16.16           */
    q16_t out_max;  /**< Output upper limit  Q16.16           */
    q16_t kaw;      /**< Anti-windup gain    Q16.16 (0 = off) */
} foc_pi_cfg_t;

/* -------------------------------------------------------------------------
 * PI controller context (caller-allocated)
 * ---------------------------------------------------------------------- */
typedef struct {
    q16_t kp;        /**< Proportional gain              Q16.16 */
    q16_t ki_ts;     /**< ki pre-multiplied by Ts = ki/fs Q16.16 */
    q16_t kaw;       /**< Anti-windup back-calc gain      Q16.16 */
    q31_t integ;     /**< Integrator state (Q16.16 × 2^16 = full precision) */
    q16_t out_min;   /**< Lower output clamp              Q16.16 */
    q16_t out_max;   /**< Upper output clamp              Q16.16 */
    q16_t out_last;  /**< Last output (for bumpless reset bookkeeping) Q16.16 */
} foc_pi_t;

/* -------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------- */

/** @brief Initialise PI from config.
 *  ki_ts is computed as ki / ts_hz (= ki × Ts) using q16_div internally.
 *  @return FOC_OK or FOC_EINVAL (out_min > out_max, or ts_hz == 0).
 */
foc_status_t foc_pi_init(foc_pi_t *ctx, const foc_pi_cfg_t *cfg);

/** @brief Compute one PI step.  [ISR] safe.
 *  @param ctx  PI context.
 *  @param err  Error = setpoint − measurement, Q16.16.
 *  @return     Saturated output, Q16.16.
 */
q16_t foc_pi_step(foc_pi_t *ctx, q16_t err);

/** @brief Bumpless reset: preloads integrator so first output at zero error
 *         equals preload_out.  Satisfies: integ = preload_out - kp*0 = preload_out.
 *  @param preload_out  Desired initial output Q16.16.
 */
void foc_pi_reset(foc_pi_t *ctx, q16_t preload_out);

/** @brief Dynamically update output limits (e.g., Vdq circle limit).  [ISR] safe. */
void foc_pi_set_limits(foc_pi_t *ctx, q16_t lo, q16_t hi);

#ifdef __cplusplus
}
#endif

#endif /* FOC_MOTOR_PI_H */
