/*
 * Author:      Kevin Fox
 * Book:        Permanent Magnet Synchronous and Brushless DC Motor Drives
 *              by R. Krishnan — CRC Press, 2010
 * Description: Implements SVPWM sector detection, active-vector dwell time calculation, and duty cycle generation via midpoint common-mode injection. Includes an optional STM32 TIM1 CCR write function gated by the STM32_TARGET compile flag.
 */

#include "svpwm.h"

#ifdef STM32_TARGET
#  include "stm32f4xx.h"   /* or whichever STM32 HAL header is in use */
#endif

/* √3 and √3/2 — used in projection calculations */
#define SQRT3         1.7320508075688773f
#define SQRT3_OVER_2  0.8660254037844386f

static inline float32_t clampf(float32_t x, float32_t lo, float32_t hi)
{
    return x < lo ? lo : (x > hi ? hi : x);
}

static inline float32_t fmax2(float32_t a, float32_t b) { return a > b ? a : b; }
static inline float32_t fmin2(float32_t a, float32_t b) { return a < b ? a : b; }

/* ---------------------------------------------------------------------------
 * svpwm_compute
 *
 * SECTOR DETECTION
 * ----------------
 * Project the reference vector onto three axes separated by 60°:
 *
 *   Vr1 =  Vβ
 *   Vr2 =  (√3·Vα − Vβ) / 2
 *   Vr3 = −(√3·Vα + Vβ) / 2
 *
 * Sign-encode the three results into a 3-bit index N and look up sector:
 *
 *   N bit-pattern → sector  (0 and 7 are degenerate / zero vector)
 *   { 0→0,  1→2,  2→6,  3→1,  4→4,  5→3,  6→5,  7→0 }
 *
 * DWELL TIME INTERMEDIATES
 * ------------------------
 * Three normalised projection values (divide by Vdc so they are in [0,1]
 * for vectors inside the linear hexagon):
 *
 *   X =  √3·Vβ / Vdc
 *   Y = (3·Vα + √3·Vβ) / (2·Vdc)
 *   Z =  Y − X  =  (3·Vα − √3·Vβ) / (2·Vdc)
 *
 * Per-sector active-vector dwell times (both positive inside the sector):
 *
 *   Sector 1: T1=Z  T2=X    Sector 4: T1=-Z  T2=-X
 *   Sector 2: T1=Y  T2=-Z   Sector 5: T1=-Y  T2= Z
 *   Sector 3: T1=X  T2=-Y   Sector 6: T1=-X  T2= Y
 *
 * DUTY CYCLES — min-max (midpoint) common-mode injection
 * -------------------------------------------------------
 * Equivalent to the symmetrical 7-segment sequence without per-sector
 * branching:
 *
 *   va_ref =  Vα
 *   vb_ref = −Vα/2 + (√3/2)·Vβ
 *   vc_ref = −Vα/2 − (√3/2)·Vβ
 *
 *   v_cm   = −(max + min) / 2        (zero-sequence injection)
 *
 *   duty_x = 0.5 + (vx_ref + v_cm) / Vdc
 * ---------------------------------------------------------------------------
 */
void svpwm_compute(float32_t valpha, float32_t vbeta, float32_t vdc,
                   SVPWM_Output *out)
{
    /* --- Sector detection ------------------------------------------- */
    float32_t Vr1 =  vbeta;
    float32_t Vr2 =  SQRT3_OVER_2 * valpha - 0.5f * vbeta;
    float32_t Vr3 = -SQRT3_OVER_2 * valpha - 0.5f * vbeta;

    int N = (Vr1 > 0.0f ? 1 : 0)
          | (Vr2 > 0.0f ? 2 : 0)
          | (Vr3 > 0.0f ? 4 : 0);

    static const int sector_lut[8] = { 0, 2, 6, 1, 4, 3, 5, 0 };
    out->sector = sector_lut[N];

    /* --- Active-vector dwell times ----------------------------------- */
    float32_t inv_vdc = 1.0f / vdc;
    float32_t X =  SQRT3 * vbeta * inv_vdc;
    float32_t Y = (1.5f * valpha + SQRT3_OVER_2 * vbeta) * inv_vdc;
    float32_t Z =  Y - X;

    float32_t t1, t2;
    switch (out->sector) {
        case 1: t1 =  Z; t2 =  X; break;
        case 2: t1 =  Y; t2 = -Z; break;
        case 3: t1 =  X; t2 = -Y; break;
        case 4: t1 = -Z; t2 = -X; break;
        case 5: t1 = -Y; t2 =  Z; break;
        case 6: t1 = -X; t2 =  Y; break;
        default: t1 = 0.0f; t2 = 0.0f; break;
    }

    /* Clamp individual times to [0, 1]. */
    t1 = clampf(t1, 0.0f, 1.0f);
    t2 = clampf(t2, 0.0f, 1.0f);

    /* If overmodulation pushes T1+T2 > 1, scale back to the hexagon boundary. */
    float32_t sum = t1 + t2;
    if (sum > 1.0f) {
        float32_t inv_sum = 1.0f / sum;
        t1 *= inv_sum;
        t2 *= inv_sum;
    }

    out->t1 = t1;
    out->t2 = t2;
    out->t0 = 1.0f - t1 - t2;

    /* --- Duty cycles via midpoint common-mode injection -------------- */
    float32_t va = valpha;
    float32_t vb = -0.5f * valpha + SQRT3_OVER_2 * vbeta;
    float32_t vc = -0.5f * valpha - SQRT3_OVER_2 * vbeta;

    float32_t v_max = fmax2(fmax2(va, vb), vc);
    float32_t v_min = fmin2(fmin2(va, vb), vc);
    float32_t v_cm  = -0.5f * (v_max + v_min);

    out->ta = clampf(0.5f + (va + v_cm) * inv_vdc, 0.0f, 1.0f);
    out->tb = clampf(0.5f + (vb + v_cm) * inv_vdc, 0.0f, 1.0f);
    out->tc = clampf(0.5f + (vc + v_cm) * inv_vdc, 0.0f, 1.0f);
}

/* ---------------------------------------------------------------------------
 * svpwm_write_ccr
 *
 * STM32 TIM1 center-aligned PWM: CCR = duty × ARR.
 * The high-side output of each half-bridge is active when the up-down
 * counter value is below CCR, producing a symmetrical pulse centred in
 * the period.
 *
 *   TIM1->CCR1 ← Phase A
 *   TIM1->CCR2 ← Phase B
 *   TIM1->CCR3 ← Phase C
 * ---------------------------------------------------------------------------
 */
void svpwm_write_ccr(const SVPWM_Output *out, uint32_t arr)
{
#ifdef STM32_TARGET
    TIM1->CCR1 = (uint32_t)(out->ta * (float32_t)arr);
    TIM1->CCR2 = (uint32_t)(out->tb * (float32_t)arr);
    TIM1->CCR3 = (uint32_t)(out->tc * (float32_t)arr);
#else
    (void)out;
    (void)arr;
#endif
}
