/**
 * @file board_support.h
 * @brief BSP public API for motor_hw_if.c and drv8323_transport.c.
 *        Exposes only portable types (no HAL types in this header).
 *
 * Layer: C  Includes constants.h internally (never exposed upward).
 */
#ifndef FOC_BOARD_SUPPORT_H
#define FOC_BOARD_SUPPORT_H

#include <stdint.h>
#include <stdbool.h>
#include "foc/motor_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Initialise DWT cycle counter and bind EXTI nFAULT callback. */
void bsp_init(void (*nfault_cb)(void));

/** Set PWM compare registers for all 3 channels. */
void bsp_pwm_set_compare(uint16_t ca, uint16_t cb, uint16_t cc);

/** Return PWM period ticks (constant = BSP_PWM_PERIOD_TICKS). */
uint16_t bsp_pwm_period_ticks(void);

/** Enable complementary PWM outputs (MOE set). */
void bsp_pwm_outputs_enable(void);

/** Disable PWM outputs (break/MOE cleared) — emergency stop. */
void bsp_pwm_outputs_disable(void);

/** Read injected ADC result registers into user-provided uint16 array [5]. */
void bsp_adc_read_injected(uint16_t results[5]);

/** Read current cycle counter (DWT CYCCNT). */
uint32_t bsp_cycles_now(void);

/** Microsecond blocking delay. */
void bsp_delay_us(uint32_t us);

/** Drive DRV8323 ENABLE GPIO. */
void bsp_gpio_drv_enable(bool on);

/** Read DRV8323 nFAULT GPIO. */
bool bsp_gpio_drv_nfault(void);

/** Perform one 16-bit SPI transfer to DRV8323.  Returns 0 on success. */
int bsp_spi_xfer16(uint16_t tx, uint16_t *rx);

/**
 * Measure current-sensor offsets with PWM disabled.
 * Averages n_samples raw ADC readings per phase.
 * @param n_samples  Number of ADC readings to average.
 * @param offsets    Output: raw ADC count averages for [ia, ib, ic].
 */
void bsp_measure_current_offsets(uint32_t n_samples, int32_t offsets[3]);

#ifdef __cplusplus
}
#endif

#endif /* FOC_BOARD_SUPPORT_H */
