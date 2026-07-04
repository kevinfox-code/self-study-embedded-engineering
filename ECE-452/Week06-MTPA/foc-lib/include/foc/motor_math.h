/**
 * @file motor_math.h
 * @brief Fixed-point arithmetic kernel and trigonometry.
 *        All functions are [ISR] safe: pure, no state, bounded WCET.
 *
 * Layer: A  Dependencies: motor_types.h only.
 *
 * Scaling convention:
 *   q16_mul / q16_div: both operands Q16.16, result Q16.16.
 *   q16_mul_q15:       Q16.16 × Q1.15 → Q16.16.
 *   foc_sin/cos:       angle_t in → Q1.15 out.
 *   foc_atan2:         Q16.16 y, x → angle_t.
 */
#ifndef FOC_MOTOR_MATH_H
#define FOC_MOTOR_MATH_H

#include "motor_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Saturation event hook (Decision D-002)
 * In test builds (FOC_TESTING=1) calls foc_sat_event() to count events.
 * In release builds it compiles to nothing.
 * ====================================================================== */
#ifdef FOC_TESTING
    void foc_sat_event(const char *tag);
    #define FOC_SAT_HOOK(tag) foc_sat_event(tag)
#else
    #ifndef FOC_SAT_HOOK
    #define FOC_SAT_HOOK(tag) ((void)0)
    #endif
#endif

/* =========================================================================
 * Q16.16 arithmetic  [ISR]
 * ====================================================================== */

/**
 * Saturating Q16.16 multiply.
 * Uses 64-bit intermediate, round-half-away-from-zero, saturates to
 * Q16_MAX / Q16_MIN on overflow.  Calls FOC_SAT_HOOK("q16_mul") on sat.
 */
q16_t q16_mul(q16_t a, q16_t b);

/**
 * Non-saturating fast Q16.16 multiply.  No overflow check, no rounding.
 * EVERY call site must carry a range-proof comment asserting |a|, |b|
 * are small enough that the 64-bit product fits in Q16.16.
 */
q16_t q16_mul_ns(q16_t a, q16_t b);

/**
 * Mixed-format multiply: Q16.16 × Q1.15 → Q16.16.
 * Result = (a * b) >> 15, saturated.  [ISR] safe.
 */
q16_t q16_mul_q15(q16_t a, q15_t b);

/**
 * Saturating Q16.16 divide: (a / b), 64-bit numerator shift.
 * b == 0: returns Q16_MAX when a >= 0, Q16_MIN when a < 0; no errno.
 */
q16_t q16_div(q16_t a, q16_t b);

/** Absolute value of Q16.16.  Q16_MIN maps to Q16_MAX (saturation). */
q16_t q16_abs(q16_t v);

/** Minimum of two Q16.16 values. */
q16_t q16_min(q16_t a, q16_t b);

/** Maximum of two Q16.16 values. */
q16_t q16_max(q16_t a, q16_t b);

/** Clamp v to [lo, hi].  Undefined behaviour if lo > hi. */
q16_t q16_clamp(q16_t v, q16_t lo, q16_t hi);

/**
 * Integer square root of a Q16.16 value.
 * Input: x >= 0 (returns 0 for negative/zero inputs).
 * Output: Q16.16 (e.g. q16_sqrt(Q16(4.0)) == Q16(2.0)).
 * Algorithm: Newton–Raphson, 16 iterations, fixed WCET.
 * NOTE: Task-rate only; document if used in ISR.
 */
q16_t q16_sqrt(q16_t x);

/**
 * Scale a raw ADC/sensor integer reading to Q16.16:
 *   result = (raw * gain) + offset
 * where gain and offset are Q16.16.
 */
q16_t q16_from_raw_scaled(int32_t raw, q16_t gain, q16_t offset);

/* =========================================================================
 * Trigonometry  [ISR]
 * ====================================================================== */

/**
 * Sine of angle theta (angle_t, full circle = 2^32).
 * Uses quarter-wave 256-entry Q1.15 LUT + linear interpolation.
 * Maximum absolute error: ≤ 4 LSB Q1.15.
 */
q15_t foc_sin(angle_t theta);

/**
 * Cosine of angle theta (see foc_sin for precision spec).
 */
q15_t foc_cos(angle_t theta);

/**
 * Combined sine and cosine; avoids two LUT lookups in the hot path.
 */
void foc_sincos(angle_t theta, q15_t *s_out, q15_t *c_out);

/**
 * Four-quadrant arctangent, angle_t result.
 * Inputs: Q16.16 y, x (SI, same unit).
 * Max error: ≤ 0.05° electrical (≈ 486 angle_t LSB).
 * Algorithm: 8-octant LUT + linear interpolation (Decision D-004).
 */
angle_t foc_atan2(q16_t y, q16_t x);

/* =========================================================================
 * LUT sanity check (called once at library init)
 * ====================================================================== */

/**
 * Verifies LUT consistency (spot checks a handful of known values).
 * Returns FOC_OK if sane, FOC_EFAULT if corrupted.
 */
foc_status_t foc_math_lut_check(void);

#ifdef __cplusplus
}
#endif

#endif /* FOC_MOTOR_MATH_H */
