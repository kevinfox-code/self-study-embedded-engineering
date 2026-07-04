/**
 * @file motor_hw_if.c
 * @brief Portable (Layer B, host-testable) part of the hardware boundary:
 *        hw_cal_t validation and copy.  No HAL, no board-config header.
 *
 * The PWM/ADC/GPIO primitives declared in motor_hw_if.h remain implemented
 * per-target (src/port/stm32u5/motor_hw_if.c) or per-mock
 * (test/sim/hw_if_mock.c); this file holds the pure config-validation logic
 * shared by both, since it has no HAL bridge dependency (§5.8).
 */
#include "foc/motor_hw_if.h"

/* Raw ADC offsets are validated against the int16 range even though the
 * struct stores them as int32_t, per the plan's plausibility guard
 * ("offsets within int16 raw range"). */
#define HW_CAL_OFFSET_RAW_MIN (-32768)
#define HW_CAL_OFFSET_RAW_MAX (32767)

foc_status_t hw_cal_set(hw_cal_t *dst, const hw_cal_t *user_cal)
{
    if ((dst == (hw_cal_t *)0) || (user_cal == (hw_cal_t *)0)) {
        return FOC_EINVAL;
    }

    for (uint32_t i = 0u; i < 3u; i++) {
        if (user_cal->i_gain[i] <= 0) {
            return FOC_EINVAL;
        }
        if ((user_cal->i_polarity[i] != (int8_t)1) &&
            (user_cal->i_polarity[i] != (int8_t)-1)) {
            return FOC_EINVAL;
        }
        if ((user_cal->i_offset_raw[i] < HW_CAL_OFFSET_RAW_MIN) ||
            (user_cal->i_offset_raw[i] > HW_CAL_OFFSET_RAW_MAX)) {
            return FOC_EINVAL;
        }
    }

    if (user_cal->vbus_gain <= 0) {
        return FOC_EINVAL;
    }

    /* temp_gain == 0 is the documented "channel unused" sentinel, so it is
     * the only field here allowed to be non-positive. */
    if (user_cal->temp_gain < 0) {
        return FOC_EINVAL;
    }

    *dst = *user_cal;
    return FOC_OK;
}

void hw_cal_get(const hw_cal_t *src, hw_cal_t *out)
{
    if ((src == (const hw_cal_t *)0) || (out == (hw_cal_t *)0)) {
        return;
    }
    *out = *src;
}
