/*
 * Author:      Kevin Fox
 * Book:        Bare-Metal Embedded C Programming
 *              by Israel Gbati — Packt, 2024
 * Description: GPIO driver header for the Week 8 SysTick project — re-exports the same LED and button API as Week 7 so the SysTick delay module can drive the LEDs independently of GPIO details.
 */
#ifndef GPIO_H
#define GPIO_H

#include "stm32u575xx.h"
#include <stdbool.h>

void led_init(void);
void blue_led_on(void);
void blue_led_off(void);
void blue_led_toggle(void);
void green_led_on(void);
void green_led_off(void);
void green_led_toggle(void);
void red_led_on(void);
void red_led_off(void);
void red_led_toggle(void);

void button_init(void);
bool get_button_state(void);

#endif /* GPIO_H */
