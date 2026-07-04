/**
 * @file motor_hw_if.c
 * @brief Target implementation of motor_hw_if.h via board_support.
 *
 * Layer: C  Includes constants.h (through board_support.h).
 */

#include "constants.h"
#include "board_support.h"
#include "foc/motor_hw_if.h"
#include "foc/motor_math.h"

void hw_pwm_set_compare(uint16_t ca, uint16_t cb, uint16_t cc)
{
    bsp_pwm_set_compare(ca, cb, cc);
}

uint16_t hw_pwm_period_ticks(void)
{
    return bsp_pwm_period_ticks();
}

void hw_pwm_outputs_enable(void)
{
    bsp_pwm_outputs_enable();
}

void hw_pwm_outputs_disable(void)
{
    bsp_pwm_outputs_disable();
}

void hw_adc_read_raw(hw_adc_raw_t *out)
{
    uint16_t r[5];
    bsp_adc_read_injected(r);
    out->ia   = r[0];
    out->ib   = r[1];
    out->ic   = r[2];
    out->vbus = r[3];
    out->temp = r[4];
}

q16_t hw_vbus_scale(uint16_t raw, const hw_cal_t *cal)
{
    /* Vbus = raw × cal->vbus_gain + cal->vbus_offset_v
     * cal->vbus_gain seeded at init from Vref × divider_ratio / adc_fs:
     * divider_ratio = (R1 + R2) / R2 = (51000 + 3300) / 3300 ≈ 16.45
     * Range proof: raw ≤ 4096, ratio ≈ 16.45, Vref=3.3 V
     *   raw × (3.3/4096) ≈ 3.3; × 16.45 ≈ 54.3 V — fits Q16 range.
     */
    return q16_from_raw_scaled((int32_t)raw, cal->vbus_gain, cal->vbus_offset_v);
}

void hw_current_scale3(const hw_adc_raw_t *raw,
                       const hw_cal_t     *cal,
                       foc_abc_t          *out)
{
    int32_t ra = (int32_t)raw->ia - cal->i_offset_raw[0];
    int32_t rb = (int32_t)raw->ib - cal->i_offset_raw[1];
    int32_t rc = (int32_t)raw->ic - cal->i_offset_raw[2];
    /* result = (raw - offset) * gain * polarity, Q16.16 (§5.8).
     * gain is Q16.16 A/LSB; (raw - offset) is a plain integer count, so
     * q16_from_raw_scaled (integer x Q16.16 -> Q16.16, no shift) applies,
     * same as hw_vbus_scale above. */
    out->a = (q16_t)((int64_t)q16_from_raw_scaled(ra, cal->i_gain[0], 0)
                      * (int64_t)cal->i_polarity[0]);
    out->b = (q16_t)((int64_t)q16_from_raw_scaled(rb, cal->i_gain[1], 0)
                      * (int64_t)cal->i_polarity[1]);
    out->c = (q16_t)((int64_t)q16_from_raw_scaled(rc, cal->i_gain[2], 0)
                      * (int64_t)cal->i_polarity[2]);
}

uint32_t hw_cycles_now(void)
{
    return bsp_cycles_now();
}

void hw_crit_enter(void)
{
    /* Target: __disable_irq() or BASEPRI mask. */
    /* __disable_irq(); */
}

void hw_crit_exit(void)
{
    /* Target: __enable_irq() or BASEPRI restore. */
    /* __enable_irq(); */
}

foc_status_t hw_gpio_drv_enable(bool on)
{
    bsp_gpio_drv_enable(on);
    return FOC_OK;
}

bool hw_gpio_drv_nfault(void)
{
    return bsp_gpio_drv_nfault();
}

foc_status_t hw_delay_us(uint32_t us)
{
    bsp_delay_us(us);
    return FOC_OK;
}
