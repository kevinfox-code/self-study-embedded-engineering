#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "svpwm.h"

#define PI              3.14159265358979323846f
#define SQRT3           1.7320508075688773f
#define SQRT3_OVER_2    0.8660254037844386f
#define DEG2RAD(d)      ((d) * PI / 180.0f)

/* Test tolerance: 1% relative error on duty cycles and dwell times. */
#define TOL_DUTY        0.0001f   /* absolute — 0.01 % of full scale */
#define TOL_SUM         1e-5f     /* T1+T2+T0 = 1 invariant          */

static int g_pass = 0, g_fail = 0;

static void check(const char *name, float32_t got, float32_t expected, float32_t tol)
{
    float32_t err = fabsf(got - expected);
    if (err <= tol) {
        printf("  PASS  %-30s  got=%8.6f  expected=%8.6f  err=%.2e\n",
               name, (double)got, (double)expected, (double)err);
        g_pass++;
    } else {
        printf("  FAIL  %-30s  got=%8.6f  expected=%8.6f  err=%.2e  (tol=%.2e)\n",
               name, (double)got, (double)expected, (double)err, (double)tol);
        g_fail++;
    }
}

static void check_int(const char *name, int got, int expected)
{
    if (got == expected) {
        printf("  PASS  %-30s  got=%d  expected=%d\n", name, got, expected);
        g_pass++;
    } else {
        printf("  FAIL  %-30s  got=%d  expected=%d\n", name, got, expected);
        g_fail++;
    }
}

/* -------------------------------------------------------------------------
 * KA-1: Deliverable hand-calculation — Vref = 0.5·Vdc at θ = 30°
 *
 * Hand derivation:
 *   Vα = 0.5·Vdc·cos(30°) = (√3/4)·Vdc ≈ 0.43301·Vdc
 *   Vβ = 0.5·Vdc·sin(30°) = 0.25·Vdc
 *
 *   Sector 1 (0° ≤ θ < 60°):
 *     X  = √3·Vβ/Vdc              = √3/4          ≈ 0.43301
 *     Y  = (3·Vα + √3·Vβ)/(2Vdc) = (3√3/4+√3/4)/2 = √3/2 ≈ 0.86603
 *     Z  = Y - X                  = √3/4          ≈ 0.43301
 *     T1 = Z = √3/4  ≈ 0.43301   (V1 dwell)
 *     T2 = X = √3/4  ≈ 0.43301   (V2 dwell)
 *     T0 = 1 - √3/2  ≈ 0.13397   (zero dwell)
 *
 *   Symmetrical 7-segment duty cycles (Sector 1: A > B > C):
 *     ta = (T1 + T2 + T0/2) / Tsw = 0.5·(1 + T1 + T2) = 0.5 + √3/4 ≈ 0.93301
 *     tb = (T2 + T0/2) / Tsw      = 0.5·(1 − T1 + T2) = 0.5         (exactly)
 *     tc = T0/(2·Tsw)             = 0.5·(1 − T1 − T2) = 0.5 − √3/4 ≈ 0.06699
 *
 *   For ARR = 1000 (center-aligned TIM1):
 *     CCR1 = 933,  CCR2 = 500,  CCR3 = 67
 * ------------------------------------------------------------------------- */
static int test_ka_30deg(void)
{
    printf("\n--- KA-1: Vref=0.5·Vdc, θ=30° (hand-calculated deliverable) ---\n");

    const float32_t vdc   = 400.0f;
    const float32_t vref  = 0.5f * vdc;
    const float32_t theta = DEG2RAD(30.0f);

    float32_t valpha = vref * cosf(theta);
    float32_t vbeta  = vref * sinf(theta);

    SVPWM_Output out;
    svpwm_compute(valpha, vbeta, vdc, &out);

    /* Expected values (exact symbolic results) */
    const float32_t t_exp   = SQRT3 / 4.0f;           /* T1 = T2 = √3/4  */
    const float32_t t0_exp  = 1.0f - SQRT3 / 2.0f;   /* T0 = 1 − √3/2   */
    const float32_t ta_exp  = 0.5f + SQRT3 / 4.0f;
    const float32_t tb_exp  = 0.5f;
    const float32_t tc_exp  = 0.5f - SQRT3 / 4.0f;

    check_int("sector",  out.sector, 1);
    check("T1/Tsw",      out.t1, t_exp,  TOL_DUTY);
    check("T2/Tsw",      out.t2, t_exp,  TOL_DUTY);
    check("T0/Tsw",      out.t0, t0_exp, TOL_DUTY);
    check("ta (duty A)", out.ta, ta_exp, TOL_DUTY);
    check("tb (duty B)", out.tb, tb_exp, TOL_DUTY);
    check("tc (duty C)", out.tc, tc_exp, TOL_DUTY);
    check("T1+T2+T0=1",  out.t1 + out.t2 + out.t0, 1.0f, TOL_SUM);

    /* CCR values for ARR = 1000 */
    const uint32_t ARR = 1000u;
    printf("\n  CCR values for ARR=%u:\n", ARR);
    printf("    CCR1 (Phase A) = %u  (expected 933)\n",
           (unsigned)(out.ta * (float32_t)ARR));
    printf("    CCR2 (Phase B) = %u  (expected 500)\n",
           (unsigned)(out.tb * (float32_t)ARR));
    printf("    CCR3 (Phase C) = %u  (expected 66 — truncation of 66.99)\n",
           (unsigned)(out.tc * (float32_t)ARR));

    return 1;
}

/* -------------------------------------------------------------------------
 * KA-2: θ = 90° — vector points toward phase B.
 *   Vα = 0,  Vβ = 0.5·Vdc  → Sector 2
 *   T1 = T2 = √3/4  (midpoint of Sector 2)
 *   ta = 0.5,  tb = 0.5 + √3/4,  tc = 0.5 − √3/4
 * ------------------------------------------------------------------------- */
static int test_ka_90deg(void)
{
    printf("\n--- KA-2: Vref=0.5·Vdc, θ=90° ---\n");

    const float32_t vdc  = 400.0f;
    const float32_t vref = 0.5f * vdc;

    SVPWM_Output out;
    svpwm_compute(0.0f, vref, vdc, &out);

    const float32_t t_exp  = SQRT3 / 4.0f;
    const float32_t t0_exp = 1.0f - SQRT3 / 2.0f;

    check_int("sector",  out.sector, 2);
    check("T1/Tsw",      out.t1, t_exp,  TOL_DUTY);
    check("T2/Tsw",      out.t2, t_exp,  TOL_DUTY);
    check("T0/Tsw",      out.t0, t0_exp, TOL_DUTY);
    check("ta (duty A)", out.ta, 0.5f,              TOL_DUTY);
    check("tb (duty B)", out.tb, 0.5f + SQRT3/4.0f, TOL_DUTY);
    check("tc (duty C)", out.tc, 0.5f - SQRT3/4.0f, TOL_DUTY);
    check("T1+T2+T0=1",  out.t1 + out.t2 + out.t0, 1.0f, TOL_SUM);

    return 1;
}

/* -------------------------------------------------------------------------
 * KA-3: Zero reference vector → all duty cycles = 0.5
 * ------------------------------------------------------------------------- */
static int test_ka_zero(void)
{
    printf("\n--- KA-3: Vref=0 (zero vector) ---\n");

    SVPWM_Output out;
    svpwm_compute(0.0f, 0.0f, 400.0f, &out);

    check("ta (duty A)", out.ta, 0.5f, TOL_DUTY);
    check("tb (duty B)", out.tb, 0.5f, TOL_DUTY);
    check("tc (duty C)", out.tc, 0.5f, TOL_DUTY);
    check("T1+T2+T0=1",  out.t1 + out.t2 + out.t0, 1.0f, TOL_SUM);

    return 1;
}

/* -------------------------------------------------------------------------
 * Sector sweep: verify sector detection around the full circle.
 *
 * Expected sectors at θ = 30°, 90°, 150°, 210°, 270°, 330°: 1,2,3,4,5,6
 * ------------------------------------------------------------------------- */
static int test_sector_sweep(void)
{
    printf("\n--- Sector sweep (Vref=0.3·Vdc, 6 cardinal angles) ---\n");

    const float32_t vdc  = 400.0f;
    const float32_t vref = 0.3f * vdc;

    const float32_t angles_deg[6]  = { 30.0f, 90.0f, 150.0f, 210.0f, 270.0f, 330.0f };
    const int       expected_sec[6] = { 1,     2,     3,      4,      5,      6      };

    int ok = 1;
    for (int i = 0; i < 6; i++) {
        float32_t theta = DEG2RAD(angles_deg[i]);
        SVPWM_Output out;
        svpwm_compute(vref * cosf(theta), vref * sinf(theta), vdc, &out);

        char name[32];
        snprintf(name, sizeof(name), "sector at θ=%.0f°", (double)angles_deg[i]);
        check_int(name, out.sector, expected_sec[i]);

        /* Invariant: T1+T2+T0 = 1 */
        float32_t sum = out.t1 + out.t2 + out.t0;
        if (fabsf(sum - 1.0f) > TOL_SUM) {
            printf("  FAIL  T1+T2+T0=%.6f at θ=%.0f°\n",
                   (double)sum, (double)angles_deg[i]);
            ok = 0; g_fail++;
        }
    }
    return ok;
}

/* -------------------------------------------------------------------------
 * Invariant sweep: T1+T2+T0 = 1 for 360 angles.
 * Duty cycle output must stay in [0, 1].
 * αβ reconstruction: inverse Clarke of duty cycles must match the input.
 * ------------------------------------------------------------------------- */
static int test_invariants(void)
{
    printf("\n--- Invariant sweep (360 angles × 3 amplitudes) ---\n");

    const float32_t vdc        = 400.0f;
    const float32_t amplitudes[3] = { 0.1f, 0.5f, 1.0f / SQRT3 - 0.001f };
    int ok = 1;
    int n_tested = 0;

    for (int ai = 0; ai < 3; ai++) {
        float32_t vref = amplitudes[ai] * vdc;
        for (int deg = 0; deg < 360; deg++) {
            float32_t theta  = DEG2RAD((float32_t)deg);
            float32_t valpha = vref * cosf(theta);
            float32_t vbeta  = vref * sinf(theta);

            SVPWM_Output out;
            svpwm_compute(valpha, vbeta, vdc, &out);

            /* T1+T2+T0 = 1 */
            float32_t sum = out.t1 + out.t2 + out.t0;
            if (fabsf(sum - 1.0f) > TOL_SUM) {
                printf("  FAIL  sum=%.6f at amp=%.3f θ=%d°\n",
                       (double)sum, (double)amplitudes[ai], deg);
                ok = 0; g_fail++;
            }

            /* Duty in [0, 1] */
            if (out.ta < -TOL_DUTY || out.ta > 1.0f + TOL_DUTY ||
                out.tb < -TOL_DUTY || out.tb > 1.0f + TOL_DUTY ||
                out.tc < -TOL_DUTY || out.tc > 1.0f + TOL_DUTY) {
                printf("  FAIL  duty out of range at amp=%.3f θ=%d°  "
                       "ta=%.4f tb=%.4f tc=%.4f\n",
                       (double)amplitudes[ai], deg,
                       (double)out.ta, (double)out.tb, (double)out.tc);
                ok = 0; g_fail++;
            }

            /*
             * αβ reconstruction check.
             * Average phase voltages (relative to DC midpoint):
             *   Va = (ta − 0.5)·Vdc,  Vb = (tb − 0.5)·Vdc,  Vc = (tc − 0.5)·Vdc
             *
             * SVPWM injects zero-sequence (common-mode), so Va+Vb+Vc ≠ 0 in
             * general.  Use the full amplitude-invariant Clarke — NOT the
             * balanced shortcut α=Va — to recover α and β correctly.
             *
             *   α = (2/3)·(Va − Vb/2 − Vc/2)
             *   β = (2/3)·(√3/2)·(Vb − Vc)
             */
            float32_t Va = (out.ta - 0.5f) * vdc;
            float32_t Vb = (out.tb - 0.5f) * vdc;
            float32_t Vc = (out.tc - 0.5f) * vdc;
            float32_t alpha_out = (2.0f / 3.0f) * (Va - 0.5f * Vb - 0.5f * Vc);
            float32_t beta_out  = (2.0f / 3.0f) * (SQRT3_OVER_2 * (Vb - Vc));

            float32_t err_a = fabsf(alpha_out - valpha);
            float32_t err_b = fabsf(beta_out  - vbeta);
            float32_t tol_v = vdc * 0.001f;   /* 0.1 % of Vdc */

            if (err_a > tol_v || err_b > tol_v) {
                printf("  FAIL  αβ reconstruct at amp=%.3f θ=%d°  "
                       "err_α=%.4f err_β=%.4f\n",
                       (double)amplitudes[ai], deg,
                       (double)err_a, (double)err_b);
                ok = 0; g_fail++;
            }

            n_tested++;
        }
    }

    if (ok) {
        printf("  PASS  All %d vectors: T1+T2+T0=1, duty∈[0,1], αβ error < 0.1%%\n",
               n_tested);
        g_pass++;
    }
    return ok;
}

/* -------------------------------------------------------------------------
 * Overmodulation clamping: Vref = 0.9·Vdc (well outside the hexagon).
 * Expect T1+T2 = 1, T0 = 0, duty ∈ [0,1].
 * ------------------------------------------------------------------------- */
static int test_overmodulation(void)
{
    printf("\n--- Overmodulation: Vref=0.9·Vdc at θ=30° ---\n");

    const float32_t vdc  = 400.0f;
    const float32_t vref = 0.9f * vdc;
    const float32_t theta = DEG2RAD(30.0f);

    SVPWM_Output out;
    svpwm_compute(vref * cosf(theta), vref * sinf(theta), vdc, &out);

    /* After clamping, T0 should be 0 and T1+T2 should be 1. */
    check("T0/Tsw == 0",    out.t0, 0.0f, TOL_DUTY);
    check("T1+T2 == 1",     out.t1 + out.t2, 1.0f, TOL_DUTY);
    check("T1+T2+T0 == 1",  out.t1 + out.t2 + out.t0, 1.0f, TOL_SUM);

    float32_t duty_ok = (out.ta >= 0.0f && out.ta <= 1.0f &&
                         out.tb >= 0.0f && out.tb <= 1.0f &&
                         out.tc >= 0.0f && out.tc <= 1.0f) ? 1.0f : 0.0f;
    check("duty ∈ [0,1]", duty_ok, 1.0f, TOL_DUTY);

    return 1;
}

/* -------------------------------------------------------------------------
 * Duty cycle comparison table — printed for the deliverable report.
 * ------------------------------------------------------------------------- */
static void print_comparison_table(void)
{
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════════════╗\n");
    printf("║  DUTY CYCLE COMPARISON TABLE — Vref=0.5·Vdc, θ=30°, Vdc=400 V     ║\n");
    printf("╠══════════════════╦══════════════╦══════════════╦═══════════════════╣\n");
    printf("║ Parameter        ║ Hand-Calc    ║ Software     ║ Error             ║\n");
    printf("╠══════════════════╬══════════════╬══════════════╬═══════════════════╣\n");

    const float32_t vdc  = 400.0f;
    const float32_t vref = 0.5f * vdc;
    const float32_t theta = DEG2RAD(30.0f);

    SVPWM_Output out;
    svpwm_compute(vref * cosf(theta), vref * sinf(theta), vdc, &out);

    /* Exact symbolic values */
    const float32_t T1_hand = SQRT3 / 4.0f;
    const float32_t T2_hand = SQRT3 / 4.0f;
    const float32_t T0_hand = 1.0f - SQRT3 / 2.0f;
    const float32_t ta_hand = 0.5f + SQRT3 / 4.0f;
    const float32_t tb_hand = 0.5f;
    const float32_t tc_hand = 0.5f - SQRT3 / 4.0f;

    struct { const char *name; float32_t hand; float32_t sw; } rows[] = {
        { "Sector",    1.0f,    (float32_t)out.sector },
        { "T1/Tsw",    T1_hand, out.t1 },
        { "T2/Tsw",    T2_hand, out.t2 },
        { "T0/Tsw",    T0_hand, out.t0 },
        { "ta (A duty)", ta_hand, out.ta },
        { "tb (B duty)", tb_hand, out.tb },
        { "tc (C duty)", tc_hand, out.tc },
    };

    for (int i = 0; i < (int)(sizeof(rows)/sizeof(rows[0])); i++) {
        float32_t err_pct = 100.0f * fabsf(rows[i].sw - rows[i].hand);
        /* Avoid divide-by-zero for Sector row (both = 1). */
        if (fabsf(rows[i].hand) > 1e-6f)
            err_pct /= fabsf(rows[i].hand);
        printf("║ %-16s ║ %12.6f ║ %12.6f ║ %+.4f %%          ║\n",
               rows[i].name,
               (double)rows[i].hand,
               (double)rows[i].sw,
               (double)err_pct);
    }

    printf("╠══════════════════╬══════════════╬══════════════╬═══════════════════╣\n");
    printf("║ CCR1 (ARR=1000)  ║          933 ║ %12u ║ scope: fill in    ║\n",
           (unsigned)(out.ta * 1000.0f));
    printf("║ CCR2 (ARR=1000)  ║          500 ║ %12u ║ scope: fill in    ║\n",
           (unsigned)(out.tb * 1000.0f));
    printf("║ CCR3 (ARR=1000)  ║           67 ║ %12u ║ scope: fill in    ║\n",
           (unsigned)(out.tc * 1000.0f));
    printf("╚══════════════════╩══════════════╩══════════════╩═══════════════════╝\n");

    printf("\n  Note: 'scope: fill in' rows require hardware measurement.\n");
    printf("  Target spec: measured duty cycle error < 1 %%.\n");
}

/* ------------------------------------------------------------------ */

int main(void)
{
    printf("=== SVPWM Test Suite ===\n");

    test_ka_30deg();
    test_ka_90deg();
    test_ka_zero();
    test_sector_sweep();
    test_invariants();
    test_overmodulation();

    printf("\n==========================================\n");
    printf("PASS: %d   FAIL: %d\n", g_pass, g_fail);
    printf("Overall result: %s\n", g_fail == 0 ? "PASS" : "FAIL");
    printf("==========================================\n");

    print_comparison_table();

    return g_fail == 0 ? 0 : 1;
}
