/*
 * SPDX-License-Identifier: MIT
 * Author:      Kevin Fox
 * Description: Debug module — fatal error handler that blinks all LEDs rapidly to signal an
 *              unrecoverable fault. Called from Error_Handler() or hard fault ISRs; never returns.
 */
#include "debug.h"
#include "gpio.h"
#include "systick.h"

#define ERROR_BLINK_PERIOD_MS 50U

void debug_error_handler(void)
{
    while (1)
    {
        (void)blue_led_on();
        (void)green_led_on();
        (void)red_led_on();

        systick_msec_delay(ERROR_BLINK_PERIOD_MS);

        (void)blue_led_off();
        (void)green_led_off();
        (void)red_led_off();

        systick_msec_delay(ERROR_BLINK_PERIOD_MS);
    }
}
