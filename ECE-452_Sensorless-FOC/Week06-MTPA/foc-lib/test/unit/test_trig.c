/**
 * @file test_trig.c
 * @brief Tests for foc_sin, foc_cos, foc_atan2 (Phase 1, §11.1 row 2).
 *
 * Spec:
 *   sin/cos: exhaustive 2^16 angle grid — max error ≤ 4 LSB Q1.15.
 *   atan2:   1e5 random vectors including axes/quadrant edges — ≤ 0.05°.
 */
#include "../../test/test_harness.h"
#include "foc/motor_math.h"
#include "foc/motor_types.h"

#include <math.h>
#include <stdio.h>
#include <stdint.h>

TEST_HARNESS_IMPL

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* -------------------------------------------------------------------------
 * sin/cos: exhaustive 2^16 grid
 * ---------------------------------------------------------------------- */
static void test_sin_exhaustive_grid(void)
{
    int max_err = 0;
    /* Step through 2^16 evenly spaced angles. */
    for (uint32_t i = 0u; i < 65536u; i++) {
        angle_t theta = (angle_t)(i << 16); /* evenly spaced */
        double angle_rad = ((double)theta / 4294967296.0) * 2.0 * M_PI;
        double ref_f = sin(angle_rad) * 32768.0;
        int32_t ref_i = (int32_t)(ref_f >= 0 ? ref_f + 0.5 : ref_f - 0.5);
        if (ref_i >  32767) ref_i =  32767;
        if (ref_i < -32768) ref_i = -32768;
        int32_t got = (int32_t)foc_sin(theta);
        int32_t err = got - ref_i;
        if (err < 0) err = -err;
        if (err > max_err) max_err = err;
    }
    /* Spec: ≤ 4 LSB Q1.15. */
    if (max_err > 4) {
        printf("    sin max_err = %d LSB (spec ≤ 4)\n", max_err);
    }
    TEST_ASSERT_TRUE(max_err <= 4);
}

static void test_cos_exhaustive_grid(void)
{
    int max_err = 0;
    for (uint32_t i = 0u; i < 65536u; i++) {
        angle_t theta = (angle_t)(i << 16);
        double angle_rad = ((double)theta / 4294967296.0) * 2.0 * M_PI;
        double ref_f = cos(angle_rad) * 32768.0;
        int32_t ref_i = (int32_t)(ref_f >= 0 ? ref_f + 0.5 : ref_f - 0.5);
        if (ref_i >  32767) ref_i =  32767;
        if (ref_i < -32768) ref_i = -32768;
        int32_t got = (int32_t)foc_cos(theta);
        int32_t err = got - ref_i;
        if (err < 0) err = -err;
        if (err > max_err) max_err = err;
    }
    if (max_err > 4) {
        printf("    cos max_err = %d LSB (spec ≤ 4)\n", max_err);
    }
    TEST_ASSERT_TRUE(max_err <= 4);
}

/* -------------------------------------------------------------------------
 * sin/cos known values
 * ---------------------------------------------------------------------- */
static void test_sin_known_values(void)
{
    /* sin(0) = 0 */
    TEST_ASSERT_EQUAL_INT(0, foc_sin(0u));
    /* sin(π/2) = Q15_ONE = 32767 */
    TEST_ASSERT_EQUAL_INT(Q15_ONE, foc_sin(0x40000000u));
    /* sin(π) ≈ 0, tolerance ≤ 4 */
    TEST_ASSERT_INT_WITHIN(4, 0, (int32_t)foc_sin(0x80000000u));
    /* sin(3π/2) ≈ -32767, tolerance ≤ 4 */
    TEST_ASSERT_INT_WITHIN(4, -32767, (int32_t)foc_sin(0xC0000000u));
}

static void test_cos_known_values(void)
{
    TEST_ASSERT_EQUAL_INT(Q15_ONE, foc_cos(0u));
    TEST_ASSERT_INT_WITHIN(4, 0, (int32_t)foc_cos(0x40000000u));
    TEST_ASSERT_INT_WITHIN(4, -32767, (int32_t)foc_cos(0x80000000u));
    TEST_ASSERT_INT_WITHIN(4, 0, (int32_t)foc_cos(0xC0000000u));
}

/* -------------------------------------------------------------------------
 * foc_sincos consistency
 * ---------------------------------------------------------------------- */
static void test_sincos_consistency(void)
{
    q15_t s, c;
    foc_sincos(ANGLE_FROM_DEG(30), &s, &c);
    TEST_ASSERT_INT_WITHIN(4, (int32_t)foc_sin(ANGLE_FROM_DEG(30)), (int32_t)s);
    TEST_ASSERT_INT_WITHIN(4, (int32_t)foc_cos(ANGLE_FROM_DEG(30)), (int32_t)c);
}

/* -------------------------------------------------------------------------
 * atan2: random vectors
 * ---------------------------------------------------------------------- */
static uint32_t s_rng_trig = 99999u;
static double rand_f(void)
{
    s_rng_trig = s_rng_trig * 1664525u + 1013904223u;
    return (double)(int32_t)s_rng_trig / 2147483648.0;
}

static void test_atan2_random(void)
{
    /* Angle_t error threshold for 0.05°:
     * 0.05° / 360° * 2^32 ≈ 596523 */
    const uint32_t TOL = 596523u;
    int max_err = 0;
    int errors = 0;
    s_rng_trig = 7777u;

    for (int i = 0; i < 100000; i++) {
        double fx = rand_f();
        double fy = rand_f();
        if (fx == 0.0 && fy == 0.0) continue;
        /* Scale to Q16 range (within ±100.0). */
        q16_t qx = (q16_t)(fx * 65536.0 * 50.0);
        q16_t qy = (q16_t)(fy * 65536.0 * 50.0);

        angle_t got = foc_atan2(qy, qx);
        double ref_rad = atan2(fy, fx);
        if (ref_rad < 0.0) ref_rad += 2.0 * M_PI;
        uint32_t ref_ang = (uint32_t)(ref_rad / (2.0 * M_PI) * 4294967296.0);

        uint32_t diff = got - ref_ang;
        if (diff > (uint32_t)0x80000000u) diff = (uint32_t)(0u - diff);
        if ((int)diff > max_err) max_err = (int)diff;
        if (diff > TOL) errors++;
    }
    if (errors > 0) {
        printf("    atan2 errors: %d / 100000, max_err = %d angle_t (tol %u)\n",
               errors, max_err, TOL);
    }
    TEST_ASSERT_EQUAL_INT(0, errors);
}

static void test_atan2_axes(void)
{
    const uint32_t TOL = 596523u;
    /* (1, 0) → 0° */
    angle_t r = foc_atan2(0, Q16(1.0));
    TEST_ASSERT_INT_WITHIN((int)TOL, 0, (int)r);
    /* (0, 1) → 90° */
    r = foc_atan2(Q16(1.0), 0);
    TEST_ASSERT_INT_WITHIN((int)TOL, (int)0x40000000u, (int)r);
    /* (-1, 0) → 180° */
    r = foc_atan2(0, Q16(-1.0));
    TEST_ASSERT_INT_WITHIN((int)TOL, (int)0x80000000u, (int)r);
}

/* -------------------------------------------------------------------------
 * main
 * ---------------------------------------------------------------------- */
int main(void)
{
    printf("=== test_trig ===\n");
    RUN_TEST(test_sin_known_values);
    RUN_TEST(test_cos_known_values);
    RUN_TEST(test_sincos_consistency);
    RUN_TEST(test_sin_exhaustive_grid);
    RUN_TEST(test_cos_exhaustive_grid);
    RUN_TEST(test_atan2_axes);
    RUN_TEST(test_atan2_random);
    TEST_REPORT();
}
