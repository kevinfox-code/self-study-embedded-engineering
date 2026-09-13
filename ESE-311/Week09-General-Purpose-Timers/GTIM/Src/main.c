/*
 * SPDX-License-Identifier: MIT
 * Author:      Kevin Fox
 * Book:        Bare-Metal Embedded C Programming
 *              by Israel Gbati — Packt, 2024
 * Description: Application entry point — uses TIM2 as a general-purpose timer to pace LED toggling from
 *              a hardware update event rather than a software delay.
 */
#include "main.h"
#include "gpio.h"
#include "tim.h"
#include "debug.h"
#include <stdbool.h>
#include <stdint.h>   

void SystemInit(void) {}

uint8_t loop_counter = 0;

int main(void) {
  led_init();
  tim2_1hz_init();

  while (1) {
    while (!tim2_update_event_ready()) {
    }

    tim2_clear_update_event();
    blue_led_toggle();

    loop_counter++;
  }
}

void Error_Handler(void) {
    debug_error_handler();
}
