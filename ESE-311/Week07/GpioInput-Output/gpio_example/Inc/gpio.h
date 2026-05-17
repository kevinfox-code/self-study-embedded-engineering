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
