#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "clarke.h"
#include "park.h"

#define NUM_RANDOM_TESTS  1000
#define ROUND_TRIP_TOL    1e-5f
#define PI                3.14159265358979323846f

/* Return a float in [lo, hi]. */
static float32_t randf(float32_t lo, float32_t hi)
{
    return lo + (hi - lo) * ((float32_t)rand() / (float32_t)RAND_MAX);
}

/* ------------------------------------------------------------------ */
/* Known-answer spot checks                                            */
/* ------------------------------------------------------------------ */

static int test_clarke_known_answers(void)
{
    int ok = 1;

    /* Balanced 3-phase at θ=0: ia=1, ib=cos(-120°)=-0.5, ic=cos(120°)=-0.5
     * Expect: α=1, β=0, zero=0                                         */
    float32_t ia = 1.0f, ib = -0.5f, ic = -0.5f;
    Clarke_Output c = clarke_forward(ia, ib, ic);
    if (fabsf(c.alpha - 1.0f) > ROUND_TRIP_TOL ||
        fabsf(c.beta)         > ROUND_TRIP_TOL ||
        fabsf(c.zero)         > ROUND_TRIP_TOL) {
        printf("FAIL  KA-Clarke-1: alpha=%.6f beta=%.6f zero=%.6f (expected 1,0,0)\n",
               c.alpha, c.beta, c.zero);
        ok = 0;
    } else {
        printf("PASS  KA-Clarke-1: balanced abc at theta=0  -> alpha=%.4f beta=%.4f zero=%.4f\n",
               c.alpha, c.beta, c.zero);
    }

    /* Balanced 3-phase at θ=90°: ia=0, ib=cos(-30°)=√3/2, ic=cos(210°)=-√3/2
     * Expect: α=0, β=1, zero=0                                            */
    ia = 0.0f; ib = 0.8660254f; ic = -0.8660254f;
    c = clarke_forward(ia, ib, ic);
    if (fabsf(c.alpha)         > ROUND_TRIP_TOL ||
        fabsf(c.beta  - 1.0f) > ROUND_TRIP_TOL ||
        fabsf(c.zero)          > ROUND_TRIP_TOL) {
        printf("FAIL  KA-Clarke-2: alpha=%.6f beta=%.6f zero=%.6f (expected 0,1,0)\n",
               c.alpha, c.beta, c.zero);
        ok = 0;
    } else {
        printf("PASS  KA-Clarke-2: balanced abc at theta=90 -> alpha=%.4f beta=%.4f zero=%.4f\n",
               c.alpha, c.beta, c.zero);
    }

    /* Pure zero-sequence: ia=ib=ic=1 → α=0, β=0, zero=1 */
    ia = ib = ic = 1.0f;
    c = clarke_forward(ia, ib, ic);
    if (fabsf(c.alpha)         > ROUND_TRIP_TOL ||
        fabsf(c.beta)          > ROUND_TRIP_TOL ||
        fabsf(c.zero  - 1.0f) > ROUND_TRIP_TOL) {
        printf("FAIL  KA-Clarke-3: alpha=%.6f beta=%.6f zero=%.6f (expected 0,0,1)\n",
               c.alpha, c.beta, c.zero);
        ok = 0;
    } else {
        printf("PASS  KA-Clarke-3: pure zero-sequence          -> alpha=%.4f beta=%.4f zero=%.4f\n",
               c.alpha, c.beta, c.zero);
    }

    return ok;
}

static int test_park_known_answers(void)
{
    int ok = 1;

    /* α=1, β=0, θ=0 → d=1, q=0 */
    Park_Output p = park_forward(1.0f, 0.0f, 0.0f);
    if (fabsf(p.d - 1.0f) > ROUND_TRIP_TOL || fabsf(p.q) > ROUND_TRIP_TOL) {
        printf("FAIL  KA-Park-1: d=%.6f q=%.6f (expected 1,0)\n", p.d, p.q);
        ok = 0;
    } else {
        printf("PASS  KA-Park-1: alpha=1,beta=0,theta=0   -> d=%.4f q=%.4f\n", p.d, p.q);
    }

    /* α=0, β=1, θ=0 → d=0, q=1 */
    p = park_forward(0.0f, 1.0f, 0.0f);
    if (fabsf(p.d) > ROUND_TRIP_TOL || fabsf(p.q - 1.0f) > ROUND_TRIP_TOL) {
        printf("FAIL  KA-Park-2: d=%.6f q=%.6f (expected 0,1)\n", p.d, p.q);
        ok = 0;
    } else {
        printf("PASS  KA-Park-2: alpha=0,beta=1,theta=0   -> d=%.4f q=%.4f\n", p.d, p.q);
    }

    /* α=1, β=0, θ=90° → d=0, q=-1 */
    p = park_forward(1.0f, 0.0f, PI / 2.0f);
    if (fabsf(p.d) > ROUND_TRIP_TOL || fabsf(p.q + 1.0f) > ROUND_TRIP_TOL) {
        printf("FAIL  KA-Park-3: d=%.6f q=%.6f (expected 0,-1)\n", p.d, p.q);
        ok = 0;
    } else {
        printf("PASS  KA-Park-3: alpha=1,beta=0,theta=90  -> d=%.4f q=%.4f\n", p.d, p.q);
    }

    return ok;
}

/* ------------------------------------------------------------------ */
/* Round-trip randomised test                                          */
/* ------------------------------------------------------------------ */

static int test_round_trip_random(void)
{
    int pass = 0, fail = 0;
    float32_t max_err = 0.0f;

    for (int i = 0; i < NUM_RANDOM_TESTS; i++) {
        float32_t ia    = randf(-2.0f,  2.0f);
        float32_t ib    = randf(-2.0f,  2.0f);
        float32_t ic    = randf(-2.0f,  2.0f);
        float32_t theta = randf(0.0f,   2.0f * PI);

        /* Forward path */
        Clarke_Output   ab0 = clarke_forward(ia, ib, ic);
        Park_Output      dq = park_forward(ab0.alpha, ab0.beta, theta);

        /* Inverse path */
        AlphaBeta_Vector ab  = park_inverse(dq.d, dq.q, theta);
        ABC_Vector    abc_r  = clarke_inverse(ab.alpha, ab.beta, ab0.zero);

        float32_t err_a = fabsf(abc_r.a - ia);
        float32_t err_b = fabsf(abc_r.b - ib);
        float32_t err_c = fabsf(abc_r.c - ic);
        float32_t err   = fmaxf(err_a, fmaxf(err_b, err_c));

        if (err > max_err) max_err = err;

        if (err < ROUND_TRIP_TOL) {
            pass++;
        } else {
            fail++;
            printf("FAIL  RT[%4d]: ia=% .5f ib=% .5f ic=% .5f theta=%.4f | "
                   "err_a=%.2e err_b=%.2e err_c=%.2e\n",
                   i, (double)ia, (double)ib, (double)ic, (double)theta,
                   (double)err_a, (double)err_b, (double)err_c);
        }
    }

    printf("\nRound-trip random test (%d vectors, tol=%.0e):\n",
           NUM_RANDOM_TESTS, (double)ROUND_TRIP_TOL);
    printf("  PASS: %d   FAIL: %d   max_err: %.3e\n", pass, fail, (double)max_err);

    return fail == 0;
}

/* ------------------------------------------------------------------ */
/* Edge cases                                                          */
/* ------------------------------------------------------------------ */

static int test_edge_cases(void)
{
    int ok = 1;

    /* Zero vector */
    Clarke_Output c = clarke_forward(0.0f, 0.0f, 0.0f);
    Park_Output   p = park_forward(c.alpha, c.beta, 0.785f);
    AlphaBeta_Vector ab = park_inverse(p.d, p.q, 0.785f);
    ABC_Vector abc_r = clarke_inverse(ab.alpha, ab.beta, c.zero);
    if (fabsf(abc_r.a) > ROUND_TRIP_TOL || fabsf(abc_r.b) > ROUND_TRIP_TOL ||
        fabsf(abc_r.c) > ROUND_TRIP_TOL) {
        printf("FAIL  EC-Zero: a=%.2e b=%.2e c=%.2e\n",
               (double)abc_r.a, (double)abc_r.b, (double)abc_r.c);
        ok = 0;
    } else {
        printf("PASS  EC-Zero: zero input round-trips correctly\n");
    }

    /* θ = 2π (should behave identically to θ = 0) */
    p = park_forward(1.0f, 0.0f, 2.0f * PI);
    if (fabsf(p.d - 1.0f) > ROUND_TRIP_TOL || fabsf(p.q) > ROUND_TRIP_TOL) {
        printf("FAIL  EC-2pi:  d=%.6f q=%.6f (expected 1,0)\n", (double)p.d, (double)p.q);
        ok = 0;
    } else {
        printf("PASS  EC-2pi:  theta=2π equivalent to theta=0\n");
    }

    /* Large amplitude */
    float32_t A = 1000.0f;
    c = clarke_forward(A, -A/2.0f, -A/2.0f);
    if (fabsf(c.alpha - A) > A * ROUND_TRIP_TOL) {
        printf("FAIL  EC-Large: alpha=%.4f (expected %.4f)\n", (double)c.alpha, (double)A);
        ok = 0;
    } else {
        printf("PASS  EC-Large: amplitude %.0f yields expected alpha=%.4f\n", (double)A, (double)c.alpha);
    }

    return ok;
}

/* ------------------------------------------------------------------ */
/* Main                                                                */
/* ------------------------------------------------------------------ */

int main(void)
{
    srand((unsigned int)time(NULL));

    printf("=== Clarke & Park Transform Test Suite ===\n\n");

    printf("--- Known-Answer: Clarke ---\n");
    int ka_c = test_clarke_known_answers();

    printf("\n--- Known-Answer: Park ---\n");
    int ka_p = test_park_known_answers();

    printf("\n--- Edge Cases ---\n");
    int ec = test_edge_cases();

    printf("\n--- Round-Trip Random ---\n");
    int rt = test_round_trip_random();

    int all_pass = ka_c && ka_p && ec && rt;

    printf("\n==========================================\n");
    printf("Overall result: %s\n", all_pass ? "PASS" : "FAIL");
    printf("==========================================\n");

    return all_pass ? 0 : 1;
}
