/**
 * @file test_limits_sat.c
 * @brief Tests for motor_limits.h circle limit and saturation (Phase 2/3, §11.1 row 6).
 */
#include "../../test/test_harness.h"
#include "foc/motor_limits.h"
#include "foc/motor_types.h"
#include "foc/motor_math.h"

#include <math.h>
#include <stdio.h>

TEST_HARNESS_IMPL

static void test_vdq_limit_basic(void)
{
    /* vdq_limit(24V, 0.9) = 0.9 * 24 / √3 ≈ 12.47 V */
    q16_t lim = foc_vdq_limit(Q16(24.0), Q16(0.9));
    /* Double ref: 0.9 * 24 / 1.7320508 ≈ 12.470 */
    TEST_ASSERT_INT_WITHIN((int)(0.01 * 65536), Q16(12.47), lim);
}

static void test_vdq_limit_zero_vbus(void)
{
    q16_t lim = foc_vdq_limit(0, Q16(0.9));
    TEST_ASSERT_EQUAL_INT(0, lim);
}

static void test_limits_default_compile(void)
{
    /* Just verifies default macro can be used to initialize a struct. */
    motor_limits_t lim = FOC_LIMITS_DEFAULT;
    TEST_ASSERT_TRUE(lim.i_phase_trip_a > lim.i_phase_max_a);
    TEST_ASSERT_TRUE(lim.vbus_max_v > lim.vbus_min_v);
    TEST_ASSERT_TRUE(lim.modulation_max <= Q16(0.95));
}

int main(void)
{
    printf("=== test_limits_sat ===\n");
    RUN_TEST(test_vdq_limit_basic);
    RUN_TEST(test_vdq_limit_zero_vbus);
    RUN_TEST(test_limits_default_compile);
    TEST_REPORT();
}
