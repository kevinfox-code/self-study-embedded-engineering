/*
 * Author:      Kevin Fox
 * Description: GPIO driver header for STM32U575 — register-level LED and user button control.
 *
 * Provides status codes for all GPIO operations (init, state changes).
 * All functions return GPIO_OK on success or error code on failure.
 */
#ifndef GPIO_H
#define GPIO_H

#include "stm32u575xx.h"
#include <stdbool.h>

/* GPIO operation status codes. */
typedef enum {
    GPIO_OK = 0,
    GPIO_ERROR_INIT = 1,
    GPIO_ERROR_INVALID_PIN = 2,
} gpio_status_t;

gpio_status_t led_init(void);
gpio_status_t blue_led_on(void);
gpio_status_t blue_led_off(void);
gpio_status_t blue_led_toggle(void);
gpio_status_t green_led_on(void);
gpio_status_t green_led_off(void);
gpio_status_t green_led_toggle(void);
gpio_status_t red_led_on(void);
gpio_status_t red_led_off(void);
gpio_status_t red_led_toggle(void);

gpio_status_t button_init(void);
gpio_status_t get_button_state(bool *state);

#endif /* GPIO_H */
