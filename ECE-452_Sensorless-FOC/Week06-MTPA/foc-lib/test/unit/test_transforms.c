/**
 * @file test_transforms.c
 * @brief Tests for Clarke/Park/iPark transforms and SVPWM (Phase 3, §11.1 rows 5,6).
 *
 * Spec:
 *   - Clarke/Park round-trip ≤ 2 LSB Q16.16 per output vs float ref.
 *   - Power invariance (amplitude-invariant): |v_ab| = |v_abc_peak|.
 *   - 2-current form agrees with 3-current form on balanced input.
 *   - SVPWM sector boundaries, zero vector, overmodulation clamp.
 *   - Duty vs float reference ≤ ±1 tick.
 */
#include "../../test/test_harness.h"
#include "foc/motor_foc.h"
#include "foc/motor_math.h"
#include "foc/motor_types.h"
#include "../reference/ref_math.h"

#include <math.h>
#include <stdio.h>
#include <stdint.h>

TEST_HARNESS_IMPL

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* -------------------------------------------------------------------------
 * Clarke round-trip test helper
 * ---------------------------------------------------------------------- */
static void test_clarke_roundtrip(void)
{
    /* Balanced 3-phase: ia = A·cos(θ), ib = A·cos(θ-2π/3), ic = A·cos(θ+2π/3) */
    static const int N_ANGLES = 100;
    int errors = 0;
    for (int i = 0; i < N_ANGLES; i++) {
        double theta = 2.0 * M_PI * i / N_ANGLES;
        double A = 1.0; /* 1 A peak */
        double ia = A * cos(theta);
        double ib = A * cos(theta - 2.0 * M_PI / 3.0);
        double ic = A * cos(theta + 2.0 * M_PI / 3.0);

        double ref_alpha, ref_beta;
        ref_clarke(ia, ib, ic, &ref_alpha, &ref_beta);

        foc_abc_t abc = {
            (q16_t)(ia * 65536.0),
            (q16_t)(ib * 65536.0),
            (q16_t)(ic * 65536.0)
        };
        foc_ab_t ab;
        foc_clarke(&abc, &ab);

        int32_t ref_alpha_q = (int32_t)(ref_alpha * 65536.0);
        int32_t ref_beta_q  = (int32_t)(ref_beta  * 65536.0);
        int32_t err_a = ab.alpha - ref_alpha_q;
        int32_t err_b = ab.beta  - ref_beta_q;
        if (err_a < 0) err_a = -err_a;
        if (err_b < 0) err_b = -err_b;
        if (err_a > 2 || err_b > 2) errors++;
    }
    TEST_ASSERT_EQUAL_INT(0, errors);
}

/* -------------------------------------------------------------------------
 * 2-current vs 3-current Clarke agree on balanced input
 * ---------------------------------------------------------------------- */
static void test_clarke_2_vs_3(void)
{
    double theta = 1.23;
    double A = 0.8;
    double ia = A * cos(theta);
    double ib = A * cos(theta - 2.0 * M_PI / 3.0);
    double ic = -(ia + ib);

    foc_abc_t abc = {
        (q16_t)(ia * 65536.0),
        (q16_t)(ib * 65536.0),
        (q16_t)(ic * 65536.0)
    };
    foc_ab_t ab3, ab2;
    foc_clarke(&abc, &ab3);
    foc_clarke_2(&abc, &ab2);
    TEST_ASSERT_EQUAL_INT(ab3.alpha, ab2.alpha);
    TEST_ASSERT_EQUAL_INT(ab3.beta,  ab2.beta);
}

/* -------------------------------------------------------------------------
 * Park round-trip
 * ---------------------------------------------------------------------- */
static void test_park_roundtrip(void)
{
    int errors = 0;
    static const int N = 100;
    for (int i = 0; i < N; i++) {
        double theta = 2.0 * M_PI * i / N;
        double alpha = 0.9 * cos(theta * 1.3);
        double beta  = 0.7 * sin(theta * 0.7 + 0.3);

        double ref_d, ref_q;
        ref_park(alpha, beta, theta, &ref_d, &ref_q);
        double rec_a, rec_b;
        ref_ipark(ref_d, ref_q, theta, &rec_a, &rec_b);

        /* Fixed-point path */
        angle_t ang = ANGLE_FROM_RAD(theta);
        q15_t s, c;
        foc_sincos(ang, &s, &c);
        foc_ab_t ab = {(q16_t)(alpha*65536.0), (q16_t)(beta*65536.0)};
        foc_dq_t dq;
        foc_park(&ab, s, c, &dq);
        foc_ab_t ab_rec;
        foc_ipark(&dq, s, c, &ab_rec);

        int32_t ea = ab_rec.alpha - ab.alpha;
        int32_t eb = ab_rec.beta  - ab.beta;
        if (ea < 0) ea = -ea;
        if (eb < 0) eb = -eb;
        if (ea > 2 || eb > 2) errors++;
    }
    TEST_ASSERT_EQUAL_INT(0, errors);
}

/* -------------------------------------------------------------------------
 * Amplitude invariance: |v_ab|² ≈ peak² after Clarke
 * ---------------------------------------------------------------------- */
static void test_amplitude_invariance(void)
{
    double A = 0.5; /* 0.5 V peak */
    double theta = 0.7;
    double ia = A * cos(theta);
    double ib = A * cos(theta - 2.0 * M_PI / 3.0);
    double ic = A * cos(theta + 2.0 * M_PI / 3.0);

    foc_abc_t abc = {
        (q16_t)(ia * 65536.0),
        (q16_t)(ib * 65536.0),
        (q16_t)(ic * 65536.0)
    };
    foc_ab_t ab;
    foc_clarke(&abc, &ab);

    /* Magnitude of αβ should equal amplitude A (amplitude-invariant). */
    double mag = sqrt((double)ab.alpha*(double)ab.alpha +
                      (double)ab.beta *(double)ab.beta) / 65536.0;
    /* Tolerance 0.5%. */
    double err = (mag - A);
    if (err < 0) err = -err;
    TEST_ASSERT_TRUE(err < A * 0.005);
}

/* -------------------------------------------------------------------------
 * SVPWM: known sector boundaries
 * ---------------------------------------------------------------------- */
static void test_svpwm_zero_vector(void)
{
    /* Zero voltage → all duties = period/2. */
    foc_ab_t v = {0, 0};
    uint16_t duty[3];
    foc_svpwm(&v, Q16(24.0), 4000u, 10u, duty);
    TEST_ASSERT_INT_WITHIN(2, 2000, (int)duty[0]);
    TEST_ASSERT_INT_WITHIN(2, 2000, (int)duty[1]);
    TEST_ASSERT_INT_WITHIN(2, 2000, (int)duty[2]);
}

static void test_svpwm_vs_float_ref(void)
{
    /* Sector 1: Va > 0, Vb < 0, Vc < 0. */
    /* V_alpha = 10V, V_beta = 0, Vbus = 24V, period = 4000. */
    double va = 10.0, vb_ref = -5.0, vc_ref = -5.0;
    double vbus = 24.0;
    int period = 4000;

    /* min-max zero: v_zero = -(v_max + v_min)/2 = -(10 + (-5))/2 = -2.5 */
    double v_zero = -(10.0 + (-5.0)) / 2.0; /* -2.5 */
    double da_ref = ref_svpwm_duty(va + v_zero,   vbus, period);
    double db_ref = ref_svpwm_duty(vb_ref + v_zero, vbus, period);
    double dc_ref = ref_svpwm_duty(vc_ref + v_zero, vbus, period);

    foc_ab_t v = {Q16(10.0), 0};
    uint16_t duty[3];
    foc_svpwm(&v, Q16(24.0), (uint16_t)period, 10u, duty);

    TEST_ASSERT_INT_WITHIN(1, (int)(da_ref + 0.5), (int)duty[0]);
    TEST_ASSERT_INT_WITHIN(1, (int)(db_ref + 0.5), (int)duty[1]);
    TEST_ASSERT_INT_WITHIN(1, (int)(dc_ref + 0.5), (int)duty[2]);
}

static void test_svpwm_min_pulse_clamp(void)
{
    /* Very large voltage should clamp to min_ticks / period-min_ticks. */
    foc_ab_t v = {Q16_MAX, 0};
    uint16_t duty[3];
    foc_svpwm(&v, Q16(24.0), 4000u, 50u, duty);
    /* At least one duty must be clamped to period-50. */
    int found_max = 0;
    for (int i = 0; i < 3; i++) {
        if (duty[i] >= 4000u - 50u) found_max = 1;
    }
    TEST_ASSERT_TRUE(found_max);
}

/* -------------------------------------------------------------------------
 * main
 * ---------------------------------------------------------------------- */
int main(void)
{
    printf("=== test_transforms ===\n");
    RUN_TEST(test_clarke_roundtrip);
    RUN_TEST(test_clarke_2_vs_3);
    RUN_TEST(test_park_roundtrip);
    RUN_TEST(test_amplitude_invariance);
    RUN_TEST(test_svpwm_zero_vector);
    RUN_TEST(test_svpwm_vs_float_ref);
    RUN_TEST(test_svpwm_min_pulse_clamp);
    TEST_REPORT();
}
