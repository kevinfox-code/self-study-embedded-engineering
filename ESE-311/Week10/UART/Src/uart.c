/*
 * Author:      Kevin Fox
 * Description: UART driver implementation for STM32U575 — basic character I/O.
 */
#include "uart.h"

void uart_init(void) {
    /* TODO: Implement UART initialization */
}

void uart_send_char(char c) {
    /* TODO: Implement character transmission */
}

void uart_send_string(const char *str) {
    if (!str) return;
    while (*str) {
        uart_send_char(*str++);
    }
}

char uart_recv_char(void) {
    /* TODO: Implement character reception */
    return 0;
}
