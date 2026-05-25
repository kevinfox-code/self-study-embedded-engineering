/*
 * Author:      Kevin Fox
 * Book:        Bare-Metal Embedded C Programming
 *              by Israel Gbati — Packt, 2024
 * Description: UART driver header for the Week 10 UART project.
 */
#ifndef UART_H
#define UART_H

#include "stm32u575xx.h"
#include <stdint.h>

void uart_init(void);
static uint16_t compute_uart_bd(uint32_t peripheral_clock, uint32_t baudrate);
static void uart_set_baudrate(uint32_t peripheral_clock, uint32_t baudrate);
void uart_send_char(char c);
void uart_send_string(const char *str);
char uart_recv_char(void);

#endif /* UART_H */
