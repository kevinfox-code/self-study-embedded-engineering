/**
 * @file motor_foc.h
 * @brief FOC core: Clarke/Park transforms, current regulators, SVPWM.
 *        All public functions are [ISR] safe.
 *
 * Layer: B  Dependencies: motor_types.h, motor_math.h, motor_pi.h.
 *
 * Amplitude-invariant Clarke convention (Decision — documented in test):
 *   α = a, β = (a + 2b) / √3  for 2-current form.
 *   α = a, β = (b - c) / √3   for 3-current form.
 * Power is preserved when all quantities use the same unit.
 */
#ifndef FOC_MOTOR_FOC_H
#define FOC_MOTOR_FOC_H

#include "motor_types.h"
#include "motor_math.h"
#include "motor_pi.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * FOC core context
 * ---------------------------------------------------------------------- */
typedef struct {
    foc_pi_t  pi_id;       /**< d-axis current PI                    */
    foc_pi_t  pi_iq;       /**< q-axis current PI                    */
    q16_t     vd_out;      /**< Last computed Vd [V]     Q16.16      */
    q16_t     vq_out;      /**< Last computed Vq [V]     Q16.16      */
    foc_ab_t  v_ab_out;    /**< Inverse-Park output Vαβ [V] Q16.16  */
    uint16_t  duty[3];     /**< SVPWM duty cycles [timer ticks]      */
    q16_t     mod_index;   /**< Modulation index (0..1)  Q16.16      */
    uint16_t  min_pulse_ticks; /**< Min low-side pulse (sampling window) */
} foc_core_t;

/* -------------------------------------------------------------------------
 * Transforms  [ISR]
 * ---------------------------------------------------------------------- */

/**
 * Clarke transform (amplitude-invariant, 3-current form).
 *   α = ia
 *   β = (ia + 2·ib) / √3   [3-current balanced]
 * Assumes ia + ib + ic = 0; uses ia and ib.
 */
void foc_clarke(const foc_abc_t *in, foc_ab_t *out);

/**
 * Clarke transform (2-current form — reconstructs ic = -(ia+ib)).
 * Equivalent to the 3-current form on balanced inputs.
 */
void foc_clarke_2(const foc_abc_t *in, foc_ab_t *out);

/**
 * Park transform.
 * d =  α·cos + β·sin
 * q = -α·sin + β·cos
 */
void foc_park(const foc_ab_t *ab, q15_t sin_th, q15_t cos_th, foc_dq_t *out);

/**
 * Inverse Park transform.
 * α = d·cos - q·sin
 * β = d·sin + q·cos
 */
void foc_ipark(const foc_dq_t *dq, q15_t sin_th, q15_t cos_th, foc_ab_t *out);

/* -------------------------------------------------------------------------
 * Current step  [ISR]
 * ---------------------------------------------------------------------- */

/**
 * Run Id/Iq PI regulators, apply circle limit, store vd/vq.
 * @param ctx        FOC core context.
 * @param i_meas     Measured dq currents [A] Q16.16.
 * @param i_ref      Reference dq currents [A] Q16.16.
 * @param vdq_limit  Circle radius [V] Q16.16 (= mod_max·vbus/√3).
 */
void foc_current_step(foc_core_t *ctx,
                      const foc_dq_t *i_meas,
                      const foc_dq_t *i_ref,
                      q16_t vdq_limit);

/* -------------------------------------------------------------------------
 * SVPWM  [ISR]
 * ---------------------------------------------------------------------- */

/**
 * Space-vector PWM: min-max injection.
 * @param v_ab        Reference voltage vector [V] Q16.16.
 * @param vbus        DC bus voltage [V] Q16.16.
 * @param period_ticks PWM counter period (from hw_pwm_period_ticks).
 * @param min_ticks   Minimum pulse width (from min_pulse_ticks).
 * @param duty_out    Output compare values [0, period_ticks] (3 elements).
 */
void foc_svpwm(const foc_ab_t *v_ab,
               q16_t vbus,
               uint16_t period_ticks,
               uint16_t min_ticks,
               uint16_t duty_out[3]);

/* -------------------------------------------------------------------------
 * Init / reset
 * ---------------------------------------------------------------------- */

/**
 * Initialise FOC core.
 * @param ctx        Context to initialise.
 * @param pi_id_cfg  d-axis PI configuration.
 * @param pi_iq_cfg  q-axis PI configuration.
 * @param min_pulse  Minimum pulse in timer ticks.
 * @return FOC_OK or FOC_EINVAL.
 */
foc_status_t foc_core_init(foc_core_t *ctx,
                            const foc_pi_cfg_t *pi_id_cfg,
                            const foc_pi_cfg_t *pi_iq_cfg,
                            uint16_t min_pulse);

/**
 * Reset FOC core regulators (called at each state entry that re-engages
 * current control).  Preloads PIs for bumpless transition.
 * @param vd_preload  d-axis preload voltage [V] Q16.16.
 * @param vq_preload  q-axis preload voltage [V] Q16.16.
 */
void foc_core_reset(foc_core_t *ctx, q16_t vd_preload, q16_t vq_preload);

#ifdef __cplusplus
}
#endif

#endif /* FOC_MOTOR_FOC_H */
