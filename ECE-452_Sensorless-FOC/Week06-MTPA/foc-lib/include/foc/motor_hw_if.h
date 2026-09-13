/**
 * @file motor_hw_if.h
 * @brief Portable hardware interface.  Header lives with the core.
 *        Contains NO HAL types — only stdint and foc types.
 *
 * Layer: A (interface)
 * Layer C implements on target (motor_hw_if.c via board_support).
 * Host tests implement in test/sim/hw_if_mock.c.
 *
 * All functions [ISR] safe unless explicitly marked "Not ISR".
 */
#ifndef FOC_MOTOR_HW_IF_H
#define FOC_MOTOR_HW_IF_H

#include <stdint.h>
#include <stdbool.h>
#include "motor_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * ADC raw result (Decision D-016)
 * ---------------------------------------------------------------------- */
typedef struct {
    uint16_t ia;    /**< Phase A current raw ADC count */
    uint16_t ib;    /**< Phase B current raw ADC count */
    uint16_t ic;    /**< Phase C current raw ADC count */
    uint16_t vbus;  /**< Bus voltage raw ADC count      */
    uint16_t temp;  /**< Temperature raw ADC count      */
} hw_adc_raw_t;

/* -------------------------------------------------------------------------
 * Current/vbus/temp scaling calibration (Decision D-006, extended §5.8)
 *
 * Fill order (normative, §5.8):
 *   1. Compile-time defaults derived in Layer C from constants.h board
 *      values (BSP_SHUNT_MOHM, BSP_CSA_GAIN_DEFAULT, BSP_VBUS_DIVIDER_*,
 *      BSP_ADC_FULLSCALE) via drv8323_amps_per_lsb().
 *   2. User hardware-specific overrides via hw_cal_set()/hw_cal_get(),
 *      applied only in IDLE/FAULT (same commit discipline as
 *      foc_ctrl_apply_params) to account for measured board realities.
 *   3. Runtime offset calibration (§6.7, CALIBRATE state) refines
 *      i_offset_raw[] only; gain/polarity fields are never touched by
 *      CALIBRATE.
 * ---------------------------------------------------------------------- */
typedef struct {
    /* Per-phase current channels (index 0=a, 1=b, 2=c) */
    q16_t   i_gain[3];       /**< Current scaling [A/LSB] Q16.16, per phase  */
    int32_t i_offset_raw[3]; /**< Zero-current offset [ADC counts], per phase*/
    int8_t  i_polarity[3];   /**< +1/-1 — board layout may invert a sense pair*/

    /* Bus voltage */
    q16_t   vbus_gain;       /**< Vbus scaling [V/LSB] Q16.16                */
    q16_t   vbus_offset_v;   /**< Vbus offset  [V]     Q16.16 (divider/opamp
                               *   trim, default 0)                         */

    /* Optional temperature channel */
    q16_t   temp_gain;       /**< Temp scaling [degC/LSB] Q16.16 (0=unused)  */
    q16_t   temp_offset_c;   /**< Temp offset  [degC]     Q16.16             */
} hw_cal_t;

/* -------------------------------------------------------------------------
 * PWM control  [ISR]
 * ---------------------------------------------------------------------- */

/** Set PWM compare values (timer ticks, 0 = 0%, period = 100%). */
void hw_pwm_set_compare(uint16_t ca, uint16_t cb, uint16_t cc);

/** Return PWM counter period in ticks (constant after init). */
uint16_t hw_pwm_period_ticks(void);

/** Enable PWM complementary outputs (MOE set). */
void hw_pwm_outputs_enable(void);

/**
 * Disable PWM outputs — forces all outputs low/hi-Z.
 * Emergency-stop primitive; callable from any context including ISR.
 * Must complete within a single PWM period.
 */
void hw_pwm_outputs_disable(void);

/* -------------------------------------------------------------------------
 * ADC  [ISR]
 * ---------------------------------------------------------------------- */

/** Fill out with latest injected ADC results (single call per fast loop). */
void hw_adc_read_raw(hw_adc_raw_t *out);

/**
 * Scale Vbus ADC count to Q16.16 volts using cal->vbus_gain / vbus_offset_v.
 * @param raw  Raw Vbus ADC count.
 * @param cal  Calibration data.
 */
q16_t hw_vbus_scale(uint16_t raw, const hw_cal_t *cal);

/**
 * Convert 3 raw ADC counts to Q16.16 amps using cal offsets and gain.
 * @param raw    Raw ADC counts.
 * @param cal    Calibration data.
 * @param out    Output currents [A] Q16.16 for each phase.
 */
void hw_current_scale3(const hw_adc_raw_t *raw,
                       const hw_cal_t     *cal,
                       foc_abc_t          *out);

/* -------------------------------------------------------------------------
 * Timing  [ISR]
 * ---------------------------------------------------------------------- */

/**
 * Return current cycle counter value (DWT on Cortex-M, or platform clock).
 * Used for WCET measurement.
 */
uint32_t hw_cycles_now(void);

/* -------------------------------------------------------------------------
 * Critical section  [ISR] callable
 * ---------------------------------------------------------------------- */

/** Enter IRQ-mask critical section (disables interrupts on target). */
void hw_crit_enter(void);

/** Exit critical section (re-enables interrupts). */
void hw_crit_exit(void);

/* -------------------------------------------------------------------------
 * Hardware calibration config  (Not ISR — task context)
 *
 * Stateless-config pattern: caller owns the hw_cal_t storage (e.g.
 * foc_ctrl_t::cal) and passes it by pointer both ways, matching the
 * existing pattern of foc_ctrl_apply_params (§5.15) — no hidden singleton
 * state is introduced here.
 * ---------------------------------------------------------------------- */

/**
 * Validate and apply user-supplied hardware calibration overrides.
 * Guards: all gains > 0 (i_gain[], vbus_gain), polarity in {+1,-1},
 * offsets within int16 raw range.  On violation, *dst is left unchanged
 * and FOC_EINVAL is returned.  Caller must apply the same commit
 * discipline as foc_ctrl_apply_params (IDLE/FAULT only).
 * @param dst       Destination calibration (e.g. &ctx->cal).
 * @param user_cal  Candidate calibration to validate and copy in.
 */
foc_status_t hw_cal_set(hw_cal_t *dst, const hw_cal_t *user_cal);

/**
 * Copy the current calibration out to the caller.
 * @param src  Source calibration (e.g. &ctx->cal).
 * @param out  Destination to receive the copy.
 */
void hw_cal_get(const hw_cal_t *src, hw_cal_t *out);

/* -------------------------------------------------------------------------
 * GPIO / peripheral  (Not ISR — task context)
 * ---------------------------------------------------------------------- */

/** Enable or disable DRV8323 via its nEN/ENABLE GPIO. */
foc_status_t hw_gpio_drv_enable(bool on);

/** Read DRV8323 nFAULT GPIO (active low = fault asserted → returns false). */
bool hw_gpio_drv_nfault(void);

/** Blocking microsecond delay.  Not for ISR use. */
foc_status_t hw_delay_us(uint32_t us);

#ifdef __cplusplus
}
#endif

#endif /* FOC_MOTOR_HW_IF_H */
