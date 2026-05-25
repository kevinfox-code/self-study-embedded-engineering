/*
 * Author:      Kevin Fox
 * Description: UART driver implementation for STM32U575 — basic character I/O.
 *
 * Added USART1 PA9/PA10 (AF7)connected to STLINK-V3E VCP -KF 25MAY2026
 */
#include "uart.h"

#define USART1_EN (1U << 11)    // APB2ENR
#define DBG_UART_BAUDRATE 115200
#define SYS_FREQ_HZ 16000000 // 16 MHz system clock
#define APB1_CLK_Hz (SYS_FREQ_HZ)
#define CR1_TE (1U << 3) // Transmitter enable
#define CR1_RE (1U << 2) // Receiver enable 
#define CR1_UE (1U << 0) // USART enable
#define SR_TXE (1U << 7) // Transmit data register empty
#define SR_RXNE (1U << 5) // Read data register not empty

// GPIOA - USART1 PA9 (TX) and PA10 (RX)
#define GPIOA_EN 	(1U << 0)

int __io_putchar(int ch) {
    uart_send_char((char)ch);
    return ch;
}
void uart_init(void) {
    /*Enable clock access to GPIOA */
    RCC_NS->AHB2ENR1 |= GPIOA_EN;
    /*Set PA9 and PA10 as alternate function */
    GPIOA_NS->MODER &= ~(3U << 18); // Clear bits [19:18] for PA9
    GPIOA_NS->MODER |= (2U << 18);  // Set bits [19:18] to alternate function for PA9
    GPIOA_NS->MODER &= ~(3U << 20); // Clear bits [21:20] for PA10
    GPIOA_NS->MODER |= (2U << 20);  // Set bits [21:20] to alternate function for PA10
    /*Set alternate function for PA9 and PA10 */
    GPIOA_NS->AFR[1] &= ~(0xFU << 4);  // Clear bits [7:4] for PA9
    GPIOA_NS->AFR[1] |= (7U << 4);   // Set bits [7:4] to USART1 for PA9
    GPIOA_NS->AFR[1] &= ~(0xFU << 8);  // Clear bits [11:8] for PA10
    GPIOA_NS->AFR[1] |= (7U << 8);   // Set bits [11:8] to USART1 for PA10
    /*Enable clock access to USART1 */
    RCC_NS->APB2ENR |= USART1_EN;
    /*Configure baud rate */
    uart_set_baudrate(APB1_CLK_Hz, DBG_UART_BAUDRATE);
    /*Configure transfer direction */
    USART1_NS->CR1 |= CR1_TE;
    /*Enable USART1 */
    USART1_NS->CR1 |= CR1_UE;
}

static uint16_t compute_uart_bd(uint32_t peripheral_clock, uint32_t baudrate) {
    return (peripheral_clock + (baudrate / 2U)) / baudrate;
}

static void uart_set_baudrate(uint32_t peripheral_clock, uint32_t baudrate) {
    uint16_t ubrr = compute_uart_bd(peripheral_clock, baudrate);
    USART1_NS->BRR = ubrr;
}

void uart_send_char(char c) {
    while(!(USART1_NS->ISR & SR_TXE)); // Wait until transmit data register is empty
    USART1_NS->RDR = c; // Write character to data register
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
