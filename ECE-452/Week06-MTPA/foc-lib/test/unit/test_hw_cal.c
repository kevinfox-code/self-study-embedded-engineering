/**
 * @file test_hw_cal.c
 * @brief Tests for hw_cal_t validation (hw_cal_set/hw_cal_get, §5.8) and
 *        hw_current_scale3/hw_vbus_scale engineering-unit scaling.
 */
#include "../../test/test_harness.h"
#include "foc/motor_hw_if.h"
#include "foc/motor_types.h"
#include "foc/motor_math.h"

#include <stdio.h>
#include <string.h>

TEST_HARNESS_IMPL

static hw_cal_t make_valid_cal(void)
{
    hw_cal_t cal;
    memset(&cal, 0, sizeof(cal));
    cal.i_gain[0]      = Q16(5.0 / 2048.0);
    cal.i_gain[1]      = Q16(5.0 / 2048.0);
    cal.i_gain[2]      = Q16(5.0 / 2048.0);
    cal.i_offset_raw[0] = 2048;
    cal.i_offset_raw[1] = 2048;
    cal.i_offset_raw[2] = 2048;
    cal.i_polarity[0]  = 1;
    cal.i_polarity[1]  = -1;
    cal.i_polarity[2]  = 1;
    cal.vbus_gain      = Q16(60.0 / 4096.0);
    cal.vbus_offset_v  = 0;
    cal.temp_gain      = 0;
    cal.temp_offset_c  = 0;
    return cal;
}

static void test_round_trip(void)
{
    hw_cal_t dst;
    memset(&dst, 0, sizeof(dst));
    hw_cal_t user_cal = make_valid_cal();

    foc_status_t st = hw_cal_set(&dst, &user_cal);
    TEST_ASSERT_EQUAL_INT(FOC_OK, st);

    hw_cal_t out;
    memset(&out, 0, sizeof(out));
    hw_cal_get(&dst, &out);

    TEST_ASSERT_EQUAL_INT(0, memcmp(&out, &user_cal, sizeof(out)));
}

static void test_zero_gain_rejected(void)
{
    hw_cal_t dst = make_valid_cal();
    hw_cal_t before = dst;
    hw_cal_t bad = make_valid_cal();
    bad.i_gain[1] = 0;

    foc_status_t st = hw_cal_set(&dst, &bad);
    TEST_ASSERT_EQUAL_INT(FOC_EINVAL, st);
    TEST_ASSERT_EQUAL_INT(0, memcmp(&dst, &before, sizeof(dst)));
}

static void test_negative_gain_rejected(void)
{
    hw_cal_t dst = make_valid_cal();
    hw_cal_t before = dst;
    hw_cal_t bad = make_valid_cal();
    bad.i_gain[2] = -Q16(1.0);

    foc_status_t st = hw_cal_set(&dst, &bad);
    TEST_ASSERT_EQUAL_INT(FOC_EINVAL, st);
    TEST_ASSERT_EQUAL_INT(0, memcmp(&dst, &before, sizeof(dst)));
}

static void test_vbus_gain_nonpositive_rejected(void)
{
    hw_cal_t dst = make_valid_cal();
    hw_cal_t before = dst;
    hw_cal_t bad = make_valid_cal();
    bad.vbus_gain = 0;

    foc_status_t st = hw_cal_set(&dst, &bad);
    TEST_ASSERT_EQUAL_INT(FOC_EINVAL, st);
    TEST_ASSERT_EQUAL_INT(0, memcmp(&dst, &before, sizeof(dst)));
}

static void test_bad_polarity_rejected(void)
{
    hw_cal_t dst = make_valid_cal();
    hw_cal_t before = dst;
    hw_cal_t bad = make_valid_cal();
    bad.i_polarity[0] = 2; /* only +1/-1 allowed */

    foc_status_t st = hw_cal_set(&dst, &bad);
    TEST_ASSERT_EQUAL_INT(FOC_EINVAL, st);
    TEST_ASSERT_EQUAL_INT(0, memcmp(&dst, &before, sizeof(dst)));
}

static void test_current_scale3_known_vector(void)
{
    /* gain = 1.0 A/LSB, offset = 100 counts, polarity +1 on a/c, -1 on b.
     * raw = {110, 90, 130} -> deltas {10, -10, 30} -> amps {10, 10, 30}
     * (b flips sign because polarity[1] = -1: (-10) * (-1) = 10). */
    hw_cal_t cal;
    memset(&cal, 0, sizeof(cal));
    cal.i_gain[0] = Q16(1.0);
    cal.i_gain[1] = Q16(1.0);
    cal.i_gain[2] = Q16(1.0);
    cal.i_offset_raw[0] = 100;
    cal.i_offset_raw[1] = 100;
    cal.i_offset_raw[2] = 100;
    cal.i_polarity[0] = 1;
    cal.i_polarity[1] = -1;
    cal.i_polarity[2] = 1;

    hw_adc_raw_t raw;
    memset(&raw, 0, sizeof(raw));
    raw.ia = 110u;
    raw.ib = 90u;
    raw.ic = 130u;

    foc_abc_t out;
    hw_current_scale3(&raw, &cal, &out);

    TEST_ASSERT_EQUAL_INT(Q16(10.0), out.a);
    TEST_ASSERT_EQUAL_INT(Q16(10.0), out.b);
    TEST_ASSERT_EQUAL_INT(Q16(30.0), out.c);
}

static void test_vbus_scale_known_vector(void)
{
    /* gain = 0.1 V/LSB, offset = 0.5 V, raw = 200 -> 20.0 + 0.5 = 20.5 V */
    hw_cal_t cal;
    memset(&cal, 0, sizeof(cal));
    cal.vbus_gain     = Q16(0.1);
    cal.vbus_offset_v = Q16(0.5);

    q16_t vbus = hw_vbus_scale(200u, &cal);
    /* Q16(0.1) itself carries ~1 LSB quantization error; 200x that is
     * amplified accordingly, so allow a proportionally wider tolerance. */
    TEST_ASSERT_INT_WITHIN(100, Q16(20.5), vbus);
}

int main(void)
{
    printf("=== test_hw_cal ===\n");
    RUN_TEST(test_round_trip);
    RUN_TEST(test_zero_gain_rejected);
    RUN_TEST(test_negative_gain_rejected);
    RUN_TEST(test_vbus_gain_nonpositive_rejected);
    RUN_TEST(test_bad_polarity_rejected);
    RUN_TEST(test_current_scale3_known_vector);
    RUN_TEST(test_vbus_scale_known_vector);
    TEST_REPORT();
}
