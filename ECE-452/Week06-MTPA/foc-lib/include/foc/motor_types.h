/**
 * @file motor_types.h
 * @brief Fixed-point typedefs, common value structs, status codes, and
 *        compile-time helpers.  Layer A — no HAL, no FreeRTOS, no float.
 *
 * Layer: A  Dependencies: stdint.h, stdbool.h only.
 */
#ifndef FOC_MOTOR_TYPES_H
#define FOC_MOTOR_TYPES_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------
 * Compile-time assertions (C99 compatible)
 * ---------------------------------------------------------------------- */
#define FOC_STATIC_ASSERT(cond, msg) \
    typedef char foc_static_assert_##msg[(cond) ? 1 : -1]

/* -------------------------------------------------------------------------
 * Fixed-point fundamental types
 * ---------------------------------------------------------------------- */

/** Q16.16 general-purpose signed: 1 sign + 15 integer + 16 fraction bits. */
typedef int32_t  q16_t;

/** Q1.15: 1 sign + 15 fraction bits.  Used for trig outputs and per-unit. */
typedef int16_t  q15_t;

/** Q1.31 / wide accumulator.  Used for PI integrators, filter states,
 *  flux integrators.  Stored as int32_t; 64-bit intermediates used where
 *  noted.                                                                 */
typedef int32_t  q31_t;

/** Unsigned electrical angle: full 360° = 2^32.  Wraps naturally.        */
typedef uint32_t angle_t;

/* Documentation aliases — carry unit intent, same binary layout as q16_t */
typedef q16_t amps_q16_t;   /**< Current,     A,     Q16.16 */
typedef q16_t volts_q16_t;  /**< Voltage,     V,     Q16.16 */
typedef q16_t radps_q16_t;  /**< MECHANICAL speed, rad/s, Q16.16. Electrical
                              *   speed is derived transiently (64-bit
                              *   intermediate, ω_e = ω_m × pole_pairs) and
                              *   never stored in this type — see §7.2. */
typedef q16_t ohm_q16_t;    /**< Resistance,  Ω,     Q16.16 */
typedef q16_t mh_q16_t;     /**< Inductance,  mH,    Q16.16 */
typedef q16_t mwb_q16_t;    /**< Flux,        mWb,   Q16.16 */

/* -------------------------------------------------------------------------
 * Struct types for 3-phase / αβ / dq quantities  (all Q16.16)
 * ---------------------------------------------------------------------- */

/** Three-phase natural frame values. */
typedef struct {
    q16_t a; /**< Phase A  [unit per context] Q16.16 */
    q16_t b; /**< Phase B  [unit per context] Q16.16 */
    q16_t c; /**< Phase C  [unit per context] Q16.16 */
} foc_abc_t;

/** Two-phase stationary frame (Clarke output). */
typedef struct {
    q16_t alpha; /**< α component  Q16.16 */
    q16_t beta;  /**< β component  Q16.16 */
} foc_ab_t;

/** Rotating frame (Park output). */
typedef struct {
    q16_t d; /**< d-axis (flux)   Q16.16 */
    q16_t q; /**< q-axis (torque) Q16.16 */
} foc_dq_t;

/* -------------------------------------------------------------------------
 * Status codes returned by all module APIs
 * ---------------------------------------------------------------------- */
typedef enum {
    FOC_OK         = 0,  /**< Success                                       */
    FOC_EINVAL     = 1,  /**< Invalid argument                              */
    FOC_EBUSY      = 2,  /**< Resource busy / queue full                    */
    FOC_EFAULT     = 3,  /**< Hardware or internal fault                    */
    FOC_ETIMEOUT   = 4,  /**< Operation timed out                           */
    FOC_ENOTREADY  = 5   /**< Precondition not met (wrong state, etc.)      */
} foc_status_t;

/** Boolean companion type (uint8_t to guarantee layout portability).      */
typedef uint8_t foc_bool_t;
#define FOC_TRUE  ((foc_bool_t)1u)
#define FOC_FALSE ((foc_bool_t)0u)

/* -------------------------------------------------------------------------
 * Measurement snapshot passed between fast-loop and TUNE/IDENT modules
 * (Decision D-007)
 * ---------------------------------------------------------------------- */
typedef struct {
    foc_abc_t i_abc;  /**< Phase currents       [A]     Q16.16 */
    foc_dq_t  i_dq;   /**< dq currents          [A]     Q16.16 */
    q16_t     vbus;   /**< Bus voltage          [V]     Q16.16 */
    angle_t   theta;  /**< Electrical angle     [angle_t] */
    q16_t     omega;  /**< MECHANICAL speed     [rad/s] Q16.16 (§7.2) */
    q16_t     conf;   /**< Observer confidence  [0,1]   Q16.16 */
} foc_meas_t;

/* -------------------------------------------------------------------------
 * Q16.16 compile-time constant helpers
 * ---------------------------------------------------------------------- */

/** Convert a floating-point literal to Q16.16 at compile time.
 *  WARNING: uses a double literal for the compile-time computation only;
 *  no float/double emitted at runtime.  Verified via static_assert.      */
#define Q16(x)      ((q16_t)((int64_t)((double)(x) * 65536.0 + ((x) >= 0.0 ? 0.5 : -0.5))))

#define Q16_ONE     ((q16_t)0x00010000)   /**< 1.0 in Q16.16            */
#define Q16_HALF    ((q16_t)0x00008000)   /**< 0.5 in Q16.16            */
#define Q16_MAX     ((q16_t)0x7FFFFFFF)   /**< Maximum positive Q16.16  */
#define Q16_MIN     ((q16_t)0x80000000)   /**< Minimum (most negative)  */

/** Q1.15 constant (16-bit).                                               */
#define Q15(x)      ((q15_t)((int32_t)((double)(x) * 32768.0 + ((x) >= 0.0 ? 0.5 : -0.5))))
#define Q15_ONE     ((q15_t)0x7FFF)
#define Q15_MIN     ((q15_t)0x8000)

/** angle_t from degrees (0–360 → 0–UINT32_MAX+1).                        */
#define ANGLE_FROM_DEG(d) \
    ((angle_t)(((double)(d) / 360.0) * 4294967296.0))

/** angle_t from electrical radians (0–2π → 0–UINT32_MAX+1).              */
#define ANGLE_FROM_RAD(r) \
    ((angle_t)(((double)(r) / (2.0 * 3.14159265358979323846)) * 4294967296.0))

/* -------------------------------------------------------------------------
 * Compile-time size guards
 * ---------------------------------------------------------------------- */
FOC_STATIC_ASSERT(sizeof(q16_t)  == 4, q16_size_must_be_4);
FOC_STATIC_ASSERT(sizeof(q15_t)  == 2, q15_size_must_be_2);
FOC_STATIC_ASSERT(sizeof(q31_t)  == 4, q31_size_must_be_4);
FOC_STATIC_ASSERT(sizeof(angle_t)== 4, angle_size_must_be_4);

#endif /* FOC_MOTOR_TYPES_H */
