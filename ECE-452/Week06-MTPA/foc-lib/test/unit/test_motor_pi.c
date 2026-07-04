/**
 * @file test_motor_pi.c
 * @brief Unit tests for motor_pi.h/.c (Phase 2, §11.1 row 4).
 *
 * Spec:
 *   - Tracking vs float reference (tolerance §11.6: ≤0.5% FS RMS).
 *   - Anti-windup: integrator stops growing when saturated.
 *   - Windup recovery bounded.
 *   - Bumpless reset: first output at zero error equals preload exactly.
 */
#include "../../test/test_harness.h"
#include "foc/motor_pi.h"
#include "foc/motor_types.h"

#include <math.h>
#include <stdio.h>

TEST_HARNESS_IMPL

/* -------------------------------------------------------------------------
 * Basic step response
 * ---------------------------------------------------------------------- */
static void test_pi_step_response(void)
{
    foc_pi_t pi;
    foc_pi_cfg_t cfg = {
        Q16(1.0),      /* kp */
        Q16(100.0),    /* ki [1/s] */
        Q16(20000.0),  /* ts_hz = fs */
        Q16(-10.0),    /* out_min */
        Q16(10.0),     /* out_max */
        Q16(0.1)       /* kaw */
    };
    foc_pi_init(&pi, &cfg);

    /* With constant error = 1.0, output should converge toward limit. */
    q16_t out = 0;
    for (int i = 0; i < 200; i++) {
        out = foc_pi_step(&pi, Q16(1.0));
    }
    /* After 200 steps with ki_ts = 100/20000 = 0.005 per step,
     * integral ≈ 0.005 * 200 = 1.0, plus kp*err = 1.0 → output ≈ 2.0.
     * Must be positive and non-zero. */
    TEST_ASSERT_TRUE(out > Q16(0.5));
}

/* -------------------------------------------------------------------------
 * Saturation: output clamped to out_max
 * ---------------------------------------------------------------------- */
static void test_pi_saturation(void)
{
    foc_pi_t pi;
    foc_pi_cfg_t cfg = {
        Q16(1.0), Q16(100.0), Q16(20000.0),
        Q16(-5.0), Q16(5.0), Q16(0.1)
    };
    foc_pi_init(&pi, &cfg);
    /* Drive with large error. */
    q16_t out = 0;
    for (int i = 0; i < 1000; i++) {
        out = foc_pi_step(&pi, Q16(100.0));
    }
    TEST_ASSERT_EQUAL_INT(Q16(5.0), out);
}

/* -------------------------------------------------------------------------
 * Anti-windup: integrator clamp when saturated
 * ---------------------------------------------------------------------- */
static void test_pi_antiwindup(void)
{
    foc_pi_t pi;
    foc_pi_cfg_t cfg = {
        Q16(0.0),     /* kp=0 → pure integrator for this test */
        Q16(100.0), Q16(20000.0),
        Q16(-5.0), Q16(5.0), Q16(1.0) /* kaw=1 */
    };
    foc_pi_init(&pi, &cfg);

    /* Drive to saturation. */
    for (int i = 0; i < 2000; i++) {
        foc_pi_step(&pi, Q16(100.0));
    }
    /* Apply reverse error — recovery should happen within a bounded time. */
    int recover_step = -1;
    for (int i = 0; i < 2000; i++) {
        q16_t out = foc_pi_step(&pi, Q16(-100.0));
        if (out < Q16(4.5) && recover_step < 0) {
            recover_step = i;
        }
    }
    /* Recovery must happen within 200 steps (kaw=1 means fast). */
    TEST_ASSERT_TRUE(recover_step >= 0 && recover_step < 200);
}

/* -------------------------------------------------------------------------
 * Bumpless reset
 * ---------------------------------------------------------------------- */
static void test_pi_bumpless_reset(void)
{
    foc_pi_t pi;
    foc_pi_cfg_t cfg = {
        Q16(1.0), Q16(100.0), Q16(20000.0),
        Q16(-10.0), Q16(10.0), Q16(0.1)
    };
    foc_pi_init(&pi, &cfg);

    /* Run for a while then reset to 3.0. */
    for (int i = 0; i < 500; i++) {
        foc_pi_step(&pi, Q16(1.0));
    }
    foc_pi_reset(&pi, Q16(3.0));
    /* With zero error, first output must equal preload (within 1 LSB). */
    q16_t out = foc_pi_step(&pi, 0);
    TEST_ASSERT_INT_WITHIN(1, Q16(3.0), (int)out);
}

/* -------------------------------------------------------------------------
 * Dynamic limits update
 * ---------------------------------------------------------------------- */
static void test_pi_set_limits(void)
{
    foc_pi_t pi;
    foc_pi_cfg_t cfg = {
        Q16(10.0), Q16(100.0), Q16(20000.0),
        Q16(-10.0), Q16(10.0), Q16(0.0)
    };
    foc_pi_init(&pi, &cfg);
    /* Saturate. */
    for (int i = 0; i < 100; i++) foc_pi_step(&pi, Q16(1.0));
    /* Tighten limits. */
    foc_pi_set_limits(&pi, Q16(-2.0), Q16(2.0));
    /* Output must now be clamped. */
    q16_t out = foc_pi_step(&pi, 0);
    TEST_ASSERT_TRUE(out <= Q16(2.0) && out >= Q16(-2.0));
}

/* -------------------------------------------------------------------------
 * EINVAL
 * ---------------------------------------------------------------------- */
static void test_pi_einval(void)
{
    foc_pi_t pi;
    foc_pi_cfg_t cfg = {
        Q16(1.0), Q16(100.0), Q16(20000.0),
        Q16(5.0), Q16(-5.0), Q16(0.1) /* out_min > out_max → EINVAL */
    };
    TEST_ASSERT_EQUAL_INT(FOC_EINVAL, foc_pi_init(&pi, &cfg));
    foc_pi_cfg_t cfg2 = { Q16(1.0), Q16(100.0), 0, Q16(-5.0), Q16(5.0), 0 };
    TEST_ASSERT_EQUAL_INT(FOC_EINVAL, foc_pi_init(&pi, &cfg2));
}

/* -------------------------------------------------------------------------
 * main
 * ---------------------------------------------------------------------- */
int main(void)
{
    printf("=== test_motor_pi ===\n");
    RUN_TEST(test_pi_einval);
    RUN_TEST(test_pi_step_response);
    RUN_TEST(test_pi_saturation);
    RUN_TEST(test_pi_antiwindup);
    RUN_TEST(test_pi_bumpless_reset);
    RUN_TEST(test_pi_set_limits);
    TEST_REPORT();
}
