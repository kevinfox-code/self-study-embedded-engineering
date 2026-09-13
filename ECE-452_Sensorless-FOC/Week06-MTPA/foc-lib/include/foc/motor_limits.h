/**
 * @file motor_limits.h
 * @brief Protection thresholds and saturation policy constants.
 *
 * Layer: A  Dependencies: motor_types.h, motor_math.h.
 */
#ifndef FOC_MOTOR_LIMITS_H
#define FOC_MOTOR_LIMITS_H

#include "motor_types.h"
#include "motor_math.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Protection threshold struct
 * Also aliased as motor_limits_s for the forward declaration in motor_params.h.
 * ---------------------------------------------------------------------- */
typedef struct motor_limits_s {
    amps_q16_t  i_phase_max_a;    /**< Max phase current (fold-back threshold) [A]   Q16.16 */
    amps_q16_t  i_phase_trip_a;   /**< Hard overcurrent trip level             [A]   Q16.16 */
    volts_q16_t vbus_min_v;       /**< Bus undervoltage trip                   [V]   Q16.16 */
    volts_q16_t vbus_max_v;       /**< Bus overvoltage trip                    [V]   Q16.16 */
    q16_t       temp_max;         /**< Max temperature (units application-defined)   Q16.16 */
    q16_t       modulation_max;   /**< Max modulation index (≤ 0.95)                Q16.16 */
    q16_t       i2t_limit;        /**< I²t thermal limit [A²·s]                     Q16.16 */
    q16_t       i2t_leak;         /**< I²t integrator leak per sample (1/τ·Ts)       Q16.16 */
    uint16_t    oc_trip_count;    /**< Debounce count for OC_SW fault (fast-loop cycles) */
} motor_limits_t;

/* -------------------------------------------------------------------------
 * Circle voltage limit helper (inline, Layer A — no HAL)
 * Returns modulation_max × vbus / √3 as Q16.16.
 * Denominator √3 ≈ Q16(1.7320508).
 * ---------------------------------------------------------------------- */
#define Q16_SQRT3   Q16(1.7320508075688772935)

static inline q16_t foc_vdq_limit(q16_t vbus, q16_t modulation_max)
{
    /* limit = mod_max × vbus / √3
     * Computed as: q16_div(q16_mul(mod_max, vbus), Q16_SQRT3)
     */
    q16_t mv = q16_mul(modulation_max, vbus);
    return q16_div(mv, Q16_SQRT3);
}

/* -------------------------------------------------------------------------
 * Default limits for a 12–48 V, 1 A class motor
 * ---------------------------------------------------------------------- */
#define FOC_LIMITS_DEFAULT \
    { Q16(1.5),   /* i_phase_max_a  1.5 A      */ \
      Q16(3.0),   /* i_phase_trip_a 3.0 A      */ \
      Q16(10.0),  /* vbus_min_v    10 V         */ \
      Q16(52.0),  /* vbus_max_v    52 V         */ \
      Q16(80.0),  /* temp_max      80 °C        */ \
      Q16(0.90),  /* modulation_max 0.90        */ \
      Q16(10.0),  /* i2t_limit     10 A²·s      */ \
      Q16(0.001), /* i2t_leak per sample        */ \
      3u          /* oc_trip_count              */ \
    }

#ifdef __cplusplus
}
#endif

#endif /* FOC_MOTOR_LIMITS_H */
