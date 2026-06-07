/*
 * Author:      Kevin Fox
 * Book:        Bare-Metal Embedded C Programming
 *              by Israel Gbati — Packt, 2024
 * Description: Main application for Week 10 UART project.
 */
#include "main.h"
#include "gpio.h"
#include "uart.h"
#include "systick.h"
#include <stdio.h>

int main(void) {
    led_init();
    button_init();
    uart_init();

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
