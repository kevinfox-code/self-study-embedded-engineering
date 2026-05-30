/*
 * Author:      Kevin Fox
 * Description: Main application entry point for the STM32U575 bare-metal starter project.
 */
#include "main.h"
#include "gpio.h"
#include "uart.h"
#include "systick.h"
#include <stdio.h>

int main(void) {
    uart_status_t uart_status;
    gpio_status_t gpio_status;
    
    gpio_status = led_init();
    if (gpio_status != GPIO_OK) {
        Error_Handler();
    }
    
    gpio_status = button_init();
    if (gpio_status != GPIO_OK) {
        Error_Handler();
    }
    
    uart_status = uart_init();
    if (uart_status != UART_OK) {
        Error_Handler();
    }

    while (1) {
        green_led_toggle();
        uart_send_string("Hello\r\n");
        systick_msec_delay(250);
    }

    return 0;
}

void Error_Handler(void) {
    while (1) {
        /* Hang in error */
    }
}
