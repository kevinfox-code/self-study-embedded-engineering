/*
 * Author:      Kevin Fox
 * Book:        Bare-Metal Embedded C Programming
 *              by Israel Gbati — Packt, 2024
 * Description: Application for Week 8 — uses the SysTick-based delay to time LED toggling and button polling, demonstrating how to replace busy-wait loops with a calibrated hardware timer delay function.
 */
#include "main.h"
#include "gpio.h"
#include "systick.h"
#include "debug.h"
#include <stdbool.h>
#include <stdint.h>   

void SystemInit(void) {}

#define DELAY_COUNT 100000
uint8_t loop_counter = 0;

bool button_state = false;

int main(void) {
  led_init();

  while (1) {
    /* Delay for 500 ms */
    systick_msec_delay(50);
    /* Toggle the LEDs */
    blue_led_toggle();
    green_led_toggle();
    red_led_toggle();

    loop_counter++;
  }
}

void Error_Handler(void) {
    debug_error_handler();
}
