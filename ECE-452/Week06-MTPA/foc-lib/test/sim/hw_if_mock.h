/**
 * @file hw_if_mock.h
 * @brief Mock hardware state for host tests.
 */
#ifndef FOC_HW_IF_MOCK_H
#define FOC_HW_IF_MOCK_H

#include "foc/motor_hw_if.h"
#include <stdbool.h>
#include <stdint.h>

#define HW_MOCK_LOG_DEPTH 256

typedef struct {
    /* PWM state */
    uint16_t duty[3];
    bool     pwm_enabled;
    uint32_t pwm_set_count;
    uint32_t enable_count;
    uint32_t disable_count;
    uint16_t pwm_log[HW_MOCK_LOG_DEPTH][3];
    uint16_t period_ticks;

    /* ADC */
    hw_adc_raw_t adc_raw;

    /* GPIO */
    bool drv_enabled;
    bool nfault;   /* true = no fault (nFAULT pin high) */

    /* Critical section */
    uint32_t crit_depth;

    /* Cycle counter */
    uint32_t mock_cycles;
} hw_mock_state_t;

void              hw_mock_init(void);
hw_mock_state_t  *hw_mock_get_state(void);

#endif /* FOC_HW_IF_MOCK_H */
