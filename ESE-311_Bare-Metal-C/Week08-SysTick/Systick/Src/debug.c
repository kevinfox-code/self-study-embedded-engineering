/*
 * SPDX-License-Identifier: MIT
 * Author:      Kevin Fox
 * Book:        Bare-Metal Embedded C Programming
 *              by Israel Gbati — Packt, 2024
 * Description: Debug module — fatal error handler that blinks all LEDs rapidly to signal an
 *              unrecoverable fault. Called from Error_Handler() or hard fault ISRs; never returns.
 */
#include "debug.h"
#include "gpio.h"
#include "systick.h"

#define ERROR_BLINK_PERIOD_MS 50U

void debug_error_handler(void) {
    while (1) {
        blue_led_on();
        green_led_on();
        red_led_on();
        systick_msec_delay(ERROR_BLINK_PERIOD_MS);

        blue_led_off();
        green_led_off();
        red_led_off();
        systick_msec_delay(ERROR_BLINK_PERIOD_MS);
    }
}
