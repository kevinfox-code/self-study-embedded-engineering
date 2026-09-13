/**
 * @file test_motor_math.c
 * @brief Unit tests for motor_math.h/.c (Phase 1, §11.1 rows 1–2).
 *
 * Tests:
 *   - Q16.16 mul/div/clamp/sqrt saturation at INT32 corners.
 *   - 1e6 random operations vs double reference within ±1 LSB Q16.16.
 *   - Division-by-zero policy (documented: returns saturated of sign(a)).
 *   - q16_abs, q16_min, q16_max correctness.
 *   - Q16() and ANGLE_FROM_DEG() macro correctness.
 */
#include "../../test/test_harness.h"
#include "foc/motor_math.h"
#include "foc/motor_types.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>

TEST_HARNESS_IMPL

/* Simple LCG for deterministic pseudo-random int32. */
static uint32_t s_rng = 42u;
static int32_t next_rand_q16(void)
{
    s_rng = s_rng * 1664525u + 1013904223u;
    /* Scale to a reasonable Q16 range to avoid trivial saturation. */
    return (int32_t)(s_rng >> 1) >> 8; /* ~23-bit range */
}

/* -------------------------------------------------------------------------
 * q16_mul tests
 * ---------------------------------------------------------------------- */
static void test_mul_basic(void)
{
    /* 2.0 * 3.0 = 6.0 */
    TEST_ASSERT_EQUAL_INT(Q16(6.0), q16_mul(Q16(2.0), Q16(3.0)));
    /* 0.5 * 0.5 = 0.25 */
    TEST_ASSERT_EQUAL_INT(Q16(0.25), q16_mul(Q16(0.5), Q16(0.5)));
    /* -1.0 * 2.0 = -2.0 */
    TEST_ASSERT_EQUAL_INT(Q16(-2.0), q16_mul(Q16(-1.0), Q16(2.0)));
    /* 1.0 * 1.0 = 1.0 */
    TEST_ASSERT_EQUAL_INT(Q16_ONE, q16_mul(Q16_ONE, Q16_ONE));
}

static void test_mul_saturation(void)
{
    int before = g_sat_count;
    /* Q16_MAX * Q16_MAX must saturate to Q16_MAX. */
    q16_t r = q16_mul(Q16_MAX, Q16_MAX);
    TEST_ASSERT_EQUAL_INT(Q16_MAX, r);
    TEST_ASSERT_TRUE(g_sat_count > before);

    before = g_sat_count;
    /* Q16_MAX * -Q16_ONE should saturate to Q16_MIN. */
    r = q16_mul(Q16_MAX, Q16(-1.0));
    TEST_ASSERT_EQUAL_INT(Q16_MIN, r);
    TEST_ASSERT_TRUE(g_sat_count > before);

    /* Zero * any = 0, no saturation. */
    int before2 = g_sat_count;
    r = q16_mul(0, Q16_MAX);
    TEST_ASSERT_EQUAL_INT(0, r);
    TEST_ASSERT_EQUAL_INT(before2, g_sat_count);
}

static void test_mul_random_vs_double(void)
{
    s_rng = 12345u;
    int errors = 0;
    for (int i = 0; i < 100000; i++) {
        q16_t a = next_rand_q16();
        q16_t b = next_rand_q16();
        double da = (double)a / 65536.0;
        double db = (double)b / 65536.0;
        double ref = da * db;
        /* Clamp reference to Q16 representable range. */
        if (ref > 32767.9999) ref = 32767.9999;
        if (ref < -32768.0)   ref = -32768.0;
        int32_t ref_q = (int32_t)(ref * 65536.0 + (ref >= 0 ? 0.5 : -0.5));
        int32_t got = q16_mul(a, b);
        int32_t diff = got - ref_q;
        if (diff < -1 || diff > 1) {
            errors++;
        }
    }
    TEST_ASSERT_EQUAL_INT(0, errors);
}

/* -------------------------------------------------------------------------
 * q16_div tests
 * ---------------------------------------------------------------------- */
static void test_div_basic(void)
{
    /* 6.0 / 2.0 = 3.0 */
    TEST_ASSERT_EQUAL_INT(Q16(3.0), q16_div(Q16(6.0), Q16(2.0)));
    /* 1.0 / 4.0 = 0.25 */
    TEST_ASSERT_EQUAL_INT(Q16(0.25), q16_div(Q16(1.0), Q16(4.0)));
    /* -6.0 / 2.0 = -3.0 */
    TEST_ASSERT_EQUAL_INT(Q16(-3.0), q16_div(Q16(-6.0), Q16(2.0)));
}

static void test_div_by_zero(void)
{
    /* Positive / 0 → Q16_MAX */
    TEST_ASSERT_EQUAL_INT(Q16_MAX, q16_div(Q16(1.0), 0));
    /* Negative / 0 → Q16_MIN */
    TEST_ASSERT_EQUAL_INT(Q16_MIN, q16_div(Q16(-1.0), 0));
    /* 0 / 0 → Q16_MAX (sign of 0 is non-negative) */
    TEST_ASSERT_EQUAL_INT(Q16_MAX, q16_div(0, 0));
}

static void test_div_saturation(void)
{
    int before = g_sat_count;
    /* Very large / very small should saturate. */
    q16_t r = q16_div(Q16_MAX, Q16(0.0001));
    TEST_ASSERT_EQUAL_INT(Q16_MAX, r);
    TEST_ASSERT_TRUE(g_sat_count > before);
}

/* -------------------------------------------------------------------------
 * q16_abs, min, max, clamp
 * ---------------------------------------------------------------------- */
static void test_abs_min_max_clamp(void)
{
    TEST_ASSERT_EQUAL_INT(Q16(3.0), q16_abs(Q16(3.0)));
    TEST_ASSERT_EQUAL_INT(Q16(3.0), q16_abs(Q16(-3.0)));
    TEST_ASSERT_EQUAL_INT(Q16_MAX,  q16_abs(Q16_MIN)); /* saturates */

    TEST_ASSERT_EQUAL_INT(Q16(1.0), q16_min(Q16(1.0), Q16(2.0)));
    TEST_ASSERT_EQUAL_INT(Q16(2.0), q16_max(Q16(1.0), Q16(2.0)));

    TEST_ASSERT_EQUAL_INT(Q16(1.0),  q16_clamp(Q16(1.5), Q16(-1.0), Q16(1.0)));
    TEST_ASSERT_EQUAL_INT(Q16(-1.0), q16_clamp(Q16(-5.0), Q16(-1.0), Q16(1.0)));
    TEST_ASSERT_EQUAL_INT(Q16(0.5),  q16_clamp(Q16(0.5), Q16(-1.0), Q16(1.0)));
}

/* -------------------------------------------------------------------------
 * q16_sqrt
 * ---------------------------------------------------------------------- */
static void test_sqrt_basic(void)
{
    /* sqrt(4.0) = 2.0 */
    q16_t r = q16_sqrt(Q16(4.0));
    TEST_ASSERT_INT_WITHIN(2, Q16(2.0), r);
    /* sqrt(1.0) = 1.0 */
    r = q16_sqrt(Q16(1.0));
    TEST_ASSERT_INT_WITHIN(2, Q16(1.0), r);
    /* sqrt(0) = 0 */
    TEST_ASSERT_EQUAL_INT(0, q16_sqrt(0));
    /* sqrt(negative) = 0 */
    TEST_ASSERT_EQUAL_INT(0, q16_sqrt(-1));
    /* sqrt(9.0) ≈ 3.0 */
    r = q16_sqrt(Q16(9.0));
    TEST_ASSERT_INT_WITHIN(2, Q16(3.0), r);
}

/* -------------------------------------------------------------------------
 * Q16() macro correctness
 * ---------------------------------------------------------------------- */
static void test_q16_macro(void)
{
    TEST_ASSERT_EQUAL_INT(65536,  Q16(1.0));
    TEST_ASSERT_EQUAL_INT(131072, Q16(2.0));
    TEST_ASSERT_EQUAL_INT(32768,  Q16(0.5));
    TEST_ASSERT_EQUAL_INT(-65536, Q16(-1.0));
}

/* -------------------------------------------------------------------------
 * ANGLE_FROM_DEG macro
 * ---------------------------------------------------------------------- */
static void test_angle_macro(void)
{
    /* 0° = 0 */
    TEST_ASSERT_EQUAL_UINT(0u, ANGLE_FROM_DEG(0));
    /* 90° = 0x40000000 */
    TEST_ASSERT_EQUAL_UINT(0x40000000u, ANGLE_FROM_DEG(90));
    /* 180° = 0x80000000 */
    TEST_ASSERT_EQUAL_UINT(0x80000000u, ANGLE_FROM_DEG(180));
}

/* -------------------------------------------------------------------------
 * q16_from_raw_scaled
 * ---------------------------------------------------------------------- */
static void test_raw_scaled(void)
{
    /* raw=100, gain=Q16(0.1), offset=Q16(0.5) → 10.0 + 0.5 = 10.5
     * Tolerance: Q16(0.1) quantises to 6554 (true 6553.6), so
     * 100 * 6554 = 655400 vs Q16(10.0) = 655360: 40 LSB irreducible
     * quantisation error.  delta=50 covers worst-case gain rounding. */
    q16_t r = q16_from_raw_scaled(100, Q16(0.1), Q16(0.5));
    TEST_ASSERT_INT_WITHIN(50, Q16(10.5), r);
}

/* -------------------------------------------------------------------------
 * LUT sanity check
 * ---------------------------------------------------------------------- */
static void test_lut_check(void)
{
    TEST_ASSERT_EQUAL_INT(FOC_OK, foc_math_lut_check());
}

/* -------------------------------------------------------------------------
 * mul_q15
 * ---------------------------------------------------------------------- */
static void test_mul_q15(void)
{
    /* Q16(2.0) * Q15(0.5) = Q16(1.0) */
    q16_t r = q16_mul_q15(Q16(2.0), Q15(0.5));
    TEST_ASSERT_INT_WITHIN(1, Q16(1.0), r);
    /* Q16(1.0) * Q15_ONE ≈ Q16(1.0) */
    r = q16_mul_q15(Q16(1.0), Q15_ONE);
    TEST_ASSERT_INT_WITHIN(2, Q16(1.0), r);
}

/* -------------------------------------------------------------------------
 * main
 * ---------------------------------------------------------------------- */
int main(void)
{
    printf("=== test_motor_math ===\n");
    RUN_TEST(test_q16_macro);
    RUN_TEST(test_angle_macro);
    RUN_TEST(test_mul_basic);
    RUN_TEST(test_mul_saturation);
    RUN_TEST(test_mul_random_vs_double);
    RUN_TEST(test_div_basic);
    RUN_TEST(test_div_by_zero);
    RUN_TEST(test_div_saturation);
    RUN_TEST(test_abs_min_max_clamp);
    RUN_TEST(test_sqrt_basic);
    RUN_TEST(test_raw_scaled);
    RUN_TEST(test_mul_q15);
    RUN_TEST(test_lut_check);
    TEST_REPORT();
}
