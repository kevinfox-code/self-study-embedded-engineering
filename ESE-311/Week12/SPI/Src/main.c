/*
 * Author:      Kevin Fox
 * Description: Main application entry point for the STM32U575 bare-metal starter project.
 */
#include "main.h"
#include "uart.h"
#include <stdio.h>

int main(void) {
    uart_status_t uart_status;
    
    // Initialize the MPU6050 accelerometer 
    
    uart_status = uart_init();
    if (uart_status != UART_OK) {
        Error_Handler();
    }

    while (1) {
        uart_send_string("Hello\r\n");


    }

    return 0;
}

void Error_Handler(void) {
    while (1) {
        /* Hang in error */
    }
}
