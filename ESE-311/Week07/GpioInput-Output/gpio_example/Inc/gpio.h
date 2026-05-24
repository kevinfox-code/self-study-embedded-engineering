/*
 * Author:      Kevin Fox
 * Book:        Bare-Metal Embedded C Programming
 *              by Israel Gbati — Packt, 2024
 * Description: Declares the bare-metal GPIO driver API for Week 7 — led_init, button_init, individual LED on/off functions, and get_button_state, all implemented without the STM32 HAL.
 */
#ifndef GPIO_H
#define GPIO_H

#include "STM32U575xx.h"
#include <stdbool.h>

void led_init(void);
void blue_led_on(void);
void blue_led_off(void);
void green_led_on(void);
void green_led_off(void);
void red_led_on(void);
void red_led_off(void);

void button_init(void);
bool get_button_state(void);

#endif /* GPIO_H */
