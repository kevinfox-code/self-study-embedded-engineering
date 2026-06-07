/*
 * SPDX-License-Identifier: MIT
 * Author:      Kevin Fox
 * Book:        Bare-Metal Embedded C Programming
 *              by Israel Gbati — Packt, 2024
 * Description: Main application for Week 10 UART project.
 */
#include "main.h"
#include "gpio.h"
#include "uart.h"
#include "systick.h"
#include "adc.h"
#include "debug.h"
#include <stdio.h>

int main(void) {
    led_init();
    uart_init();
    systick_init();
    ADC_Init();

    Start_Conversion();

    while (1) {
        green_led_toggle();

        uint32_t adc_value = ADC_Read();
        uart_send_string("ADC Value: ");
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%lu\r\n", adc_value);
        uart_send_string(buffer);


        systick_msec_delay(250);
    }

    return 0;
}

void Error_Handler(void) {
    debug_error_handler();
}
