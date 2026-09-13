/**
 * @file hw_if_mock.c
 * @brief Host implementation of motor_hw_if.h for unit/module tests.
 *        Records hw_* calls into ring buffers.
 *        Provides deterministic hw_cycles_now (increments per call).
 */
#include "hw_if_mock.h"
#include "foc/motor_hw_if.h"
#include "foc/motor_math.h"

#include <string.h>
#include <stdio.h>

/* -------------------------------------------------------------------------
 * Mock state
 * ---------------------------------------------------------------------- */
static hw_mock_state_t s_state;

void hw_mock_init(void)
{
    memset(&s_state, 0, sizeof(s_state));
    s_state.period_ticks = 4000u; /* Default: 20 kHz @ 80 MHz */
    s_state.nfault = true;        /* No fault by default */
}

hw_mock_state_t *hw_mock_get_state(void)
{
    return &s_state;
}

/* -------------------------------------------------------------------------
 * motor_hw_if.h implementations
 * ---------------------------------------------------------------------- */
void hw_pwm_set_compare(uint16_t ca, uint16_t cb, uint16_t cc)
{
    s_state.duty[0] = ca;
    s_state.duty[1] = cb;
    s_state.duty[2] = cc;
    if (s_state.pwm_set_count < HW_MOCK_LOG_DEPTH) {
        s_state.pwm_log[s_state.pwm_set_count][0] = ca;
        s_state.pwm_log[s_state.pwm_set_count][1] = cb;
        s_state.pwm_log[s_state.pwm_set_count][2] = cc;
    }
    s_state.pwm_set_count++;
}

uint16_t hw_pwm_period_ticks(void)
{
    return s_state.period_ticks;
}

void hw_pwm_outputs_enable(void)
{
    s_state.pwm_enabled = true;
    s_state.enable_count++;
}

void hw_pwm_outputs_disable(void)
{
    s_state.pwm_enabled = false;
    s_state.disable_count++;
}

void hw_adc_read_raw(hw_adc_raw_t *out)
{
    *out = s_state.adc_raw;
}

q16_t hw_vbus_scale(uint16_t raw, const hw_cal_t *cal)
{
    /* result = raw * vbus_gain + vbus_offset_v, Q16.16 */
    return q16_from_raw_scaled((int32_t)raw, cal->vbus_gain, cal->vbus_offset_v);
}

void hw_current_scale3(const hw_adc_raw_t *raw,
                       const hw_cal_t     *cal,
                       foc_abc_t          *out)
{
    /* result = (raw - offset) * gain * polarity, Q16.16 */
    int32_t ra = (int32_t)raw->ia - cal->i_offset_raw[0];
    int32_t rb = (int32_t)raw->ib - cal->i_offset_raw[1];
    int32_t rc = (int32_t)raw->ic - cal->i_offset_raw[2];
    out->a = (q16_t)((int64_t)q16_from_raw_scaled(ra, cal->i_gain[0], 0)
                      * (int64_t)cal->i_polarity[0]);
    out->b = (q16_t)((int64_t)q16_from_raw_scaled(rb, cal->i_gain[1], 0)
                      * (int64_t)cal->i_polarity[1]);
    out->c = (q16_t)((int64_t)q16_from_raw_scaled(rc, cal->i_gain[2], 0)
                      * (int64_t)cal->i_polarity[2]);
}

uint32_t hw_cycles_now(void)
{
    /* Deterministic mock: increment by 100 per call. */
    s_state.mock_cycles += 100u;
    return s_state.mock_cycles;
}

void hw_crit_enter(void)
{
    s_state.crit_depth++;
}

void hw_crit_exit(void)
{
    if (s_state.crit_depth > 0u) s_state.crit_depth--;
}

foc_status_t hw_gpio_drv_enable(bool on)
{
    s_state.drv_enabled = on;
    return FOC_OK;
}

bool hw_gpio_drv_nfault(void)
{
    return s_state.nfault;
}

foc_status_t hw_delay_us(uint32_t us)
{
    (void)us; /* No-op on host */
    return FOC_OK;
}
