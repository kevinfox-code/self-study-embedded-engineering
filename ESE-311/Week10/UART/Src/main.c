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

int main(void) {
    led_init();
    button_init();
    systick_init();
    uart_init();

    while (1) {
        /* Main application loop */
    }

    return 0;
}

void Error_Handler(void) {
    while (1) {
        /* Hang in error */
    }
}
