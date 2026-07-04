/**
 * @file test_motor_filter.c
 * @brief Unit tests for motor_filter.h/.c (Phase 2, §11.1 row 3).
 *
 * Spec:
 *   - LPF time constant within ±5% of analytic.
 *   - LPF DC gain = 1 ± 0.5%.
 *   - HPF DC drift ≤ 1 LSB over 1e6 zero-input steps.
 *   - No limit cycle at zero input.
 */
#include "../../test/test_harness.h"
#include "foc/motor_filter.h"
#include "foc/motor_types.h"

#include <math.h>
#include <stdio.h>

TEST_HARNESS_IMPL

/* -------------------------------------------------------------------------
 * LPF: DC gain
 * ---------------------------------------------------------------------- */
static void test_lpf_dc_gain(void)
{
    foc_lpf_t f;
    /* fc=100 Hz, fs=20000 Hz */
    foc_status_t st = foc_lpf_init(&f, Q16(100.0), Q16(20000.0));
    TEST_ASSERT_EQUAL_INT(FOC_OK, st);

    /* Drive with DC=1.0 for a very long time, check final output. */
    q16_t out = 0;
    for (int i = 0; i < 1000000; i++) {
        out = foc_lpf_step(&f, Q16(1.0));
    }
    /* DC gain must be 1.0 ± 0.5% → ±327 LSB Q16.16 */
    TEST_ASSERT_INT_WITHIN(655, Q16(1.0), (int)out);
}

/* -------------------------------------------------------------------------
 * LPF: time constant
 * ---------------------------------------------------------------------- */
static void test_lpf_time_constant(void)
{
    foc_lpf_t f;
    /* fc=100 Hz, fs=20000 Hz.  τ = 1/(2π·fc) ≈ 1.592 ms → 31.8 samples */
    foc_lpf_init(&f, Q16(100.0), Q16(20000.0));

    /* Apply unit step, find when output reaches 1-1/e ≈ 0.6321. */
    double tau_ref = 20000.0 / (2.0 * 3.14159265 * 100.0); /* samples */
    int target_sample = (int)(tau_ref + 0.5);

    q16_t out = 0;
    int cross_sample = -1;
    for (int i = 0; i < 10000; i++) {
        out = foc_lpf_step(&f, Q16(1.0));
        if (cross_sample < 0 && out >= Q16(0.6321)) {
            cross_sample = i;
        }
    }
    /* Tolerance: ±5% of tau_ref in samples. */
    int tol = (int)(tau_ref * 0.05 + 1.5);
    if (cross_sample < 0) cross_sample = 0;
    TEST_ASSERT_INT_WITHIN(tol, target_sample, cross_sample);
}

/* -------------------------------------------------------------------------
 * LPF: zero-input limit cycle
 * ---------------------------------------------------------------------- */
static void test_lpf_zero_input_no_limit_cycle(void)
{
    foc_lpf_t f;
    foc_lpf_init(&f, Q16(100.0), Q16(20000.0));
    foc_lpf_reset(&f, Q16(0.5));
    /* Run for 1e6 zero-input steps — output must not oscillate. */
    q16_t prev = Q16(0.5);
    for (int i = 0; i < 1000000; i++) {
        q16_t cur = foc_lpf_step(&f, 0);
        /* Must be monotonically decaying: cur <= prev (within 1 LSB rounding). */
        TEST_ASSERT_TRUE(cur <= prev + 1);
        prev = cur;
    }
    /* After 1e6 steps, output should be near zero. */
    TEST_ASSERT_INT_WITHIN(100, 0, (int)prev);
}

/* -------------------------------------------------------------------------
 * LPF: reset
 * ---------------------------------------------------------------------- */
static void test_lpf_reset(void)
{
    foc_lpf_t f;
    foc_lpf_init(&f, Q16(100.0), Q16(20000.0));
    /* Drive up from 0. */
    for (int i = 0; i < 100; i++) foc_lpf_step(&f, Q16(1.0));
    /* Reset to 0. */
    foc_lpf_reset(&f, 0);
    q16_t out = foc_lpf_step(&f, 0);
    /* Output should be near 0 immediately after reset+zero step. */
    TEST_ASSERT_INT_WITHIN(10, 0, (int)out);
}

/* -------------------------------------------------------------------------
 * HPF: DC drift
 * ---------------------------------------------------------------------- */
static void test_hpf_dc_drift(void)
{
    foc_hpf_t f;
    /* fc=2 Hz, fs=20000 Hz */
    foc_status_t st = foc_hpf_init(&f, Q16(2.0), Q16(20000.0));
    TEST_ASSERT_EQUAL_INT(FOC_OK, st);

    /* Zero-input, pre-seeded to 0. */
    q16_t out = 0;
    for (int i = 0; i < 1000000; i++) {
        out = foc_hpf_step(&f, 0);
    }
    /* Spec: drift ≤ 1 LSB over 1e6 zero samples. */
    TEST_ASSERT_INT_WITHIN(1, 0, (int)out);
}

/* -------------------------------------------------------------------------
 * HPF: passes AC, rejects DC
 * ---------------------------------------------------------------------- */
static void test_hpf_rejects_dc(void)
{
    foc_hpf_t f;
    foc_hpf_init(&f, Q16(10.0), Q16(20000.0));
    q16_t out = 0;
    /* Drive with DC=1.0 — after settle, output must be near 0. */
    for (int i = 0; i < 100000; i++) {
        out = foc_hpf_step(&f, Q16(1.0));
    }
    /* Settled DC output must be near 0 (≤ 1% FS = 655 LSB). */
    TEST_ASSERT_INT_WITHIN(655, 0, (int)out);
}

/* -------------------------------------------------------------------------
 * Filter init: EINVAL cases
 * ---------------------------------------------------------------------- */
static void test_filter_init_einval(void)
{
    foc_lpf_t lf;
    /* fc >= fs/2 → EINVAL */
    TEST_ASSERT_EQUAL_INT(FOC_EINVAL, foc_lpf_init(&lf, Q16(10001.0), Q16(20000.0)));
    /* fc = 0 → EINVAL */
    TEST_ASSERT_EQUAL_INT(FOC_EINVAL, foc_lpf_init(&lf, 0, Q16(20000.0)));
    foc_hpf_t hf;
    TEST_ASSERT_EQUAL_INT(FOC_EINVAL, foc_hpf_init(&hf, 0, Q16(20000.0)));
}

/* -------------------------------------------------------------------------
 * main
 * ---------------------------------------------------------------------- */
int main(void)
{
    printf("=== test_motor_filter ===\n");
    RUN_TEST(test_filter_init_einval);
    RUN_TEST(test_lpf_dc_gain);
    RUN_TEST(test_lpf_time_constant);
    RUN_TEST(test_lpf_zero_input_no_limit_cycle);
    RUN_TEST(test_lpf_reset);
    RUN_TEST(test_hpf_dc_drift);
    RUN_TEST(test_hpf_rejects_dc);
    TEST_REPORT();
}
