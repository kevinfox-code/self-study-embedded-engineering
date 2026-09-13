/*
 * SPDX-License-Identifier: MIT
 * Author:      Kevin Fox
 * Book:        Bare-Metal Embedded C Programming
 *              by Israel Gbati — Packt, 2024
 * Description: Application entry point — polls the user button and mirrors its state to the blue LED
 *              while toggling the green LED every other loop iteration, demonstrating GPIO input
 *              reading and output control at the register level.
 */
#include "main.h"
#include "gpio.h"
#include <stdbool.h>
#include <stdint.h>

void SystemInit(void) {}

#define DELAY_COUNT 100000
uint8_t loop_counter = 0;

bool button_state = false;

int main(void) {
  led_init();
  button_init();

  while (1) {
    button_state = get_button_state();
    if (button_state) {
      blue_led_on();
    } else {
      blue_led_off();
    }

    if (loop_counter % 2 == 0) {
      green_led_on();
      // red_led_off();
    } else {
      green_led_off();
      // red_led_on();
    }

    loop_counter++;

    for (volatile uint32_t i = 0; i < DELAY_COUNT; i++)
      ;
  }
}

void Error_Handler(void) {}
