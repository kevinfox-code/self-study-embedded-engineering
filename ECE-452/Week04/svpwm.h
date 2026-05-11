#ifndef SVPWM_H
#define SVPWM_H

#include <stdint.h>

/* Matches ARM CMSIS-DSP; plain float on host. */
typedef float float32_t;

typedef struct {
    float32_t ta;      /* Phase A duty cycle [0, 1]          */
    float32_t tb;      /* Phase B duty cycle [0, 1]          */
    float32_t tc;      /* Phase C duty cycle [0, 1]          */
    float32_t t1;      /* Active vector 1 dwell, fraction of Tsw */
    float32_t t2;      /* Active vector 2 dwell, fraction of Tsw */
    float32_t t0;      /* Zero vector dwell,   fraction of Tsw */
    int       sector;  /* Space-vector sector [1..6]          */
} SVPWM_Output;

/*
 * Compute symmetrical 7-segment SVPWM duty cycles from a stationary-frame
 * reference voltage vector (Vα, Vβ).
 *
 *   valpha  – α-axis reference voltage (same units as vdc)
 *   vbeta   – β-axis reference voltage
 *   vdc     – DC bus voltage
 *   out     – populated with duty cycles, dwell times, and sector number
 *
 * Linear modulation range: |Vref| ≤ Vdc / √3 ≈ 0.577 · Vdc.
 * Above that limit t1+t2 is clamped to 1 (six-step boundary).
 */
void svpwm_compute(float32_t valpha, float32_t vbeta, float32_t vdc,
                   SVPWM_Output *out);

/*
 * Write the computed duty cycles to TIM1 CCR1/CCR2/CCR3.
 * Compiled as a no-op on the host; define STM32_TARGET to activate.
 *
 *   out – result from svpwm_compute()
 *   arr – TIM1 Auto-Reload Register value (sets PWM period)
 *
 * Assumes center-aligned (up-down) PWM mode; CCR = duty × ARR.
 */
void svpwm_write_ccr(const SVPWM_Output *out, uint32_t arr);

#endif /* SVPWM_H */
