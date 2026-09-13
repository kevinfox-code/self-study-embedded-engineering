/*
 * SPDX-License-Identifier: MIT
 * Author:      Kevin Fox
 * Description: UART driver API for the STM32U575 — blocking character and string I/O, with optional
 *              interrupt-driven operation behind FEATURE_UART_DMA.
 */
#ifndef UART_H
#define UART_H

#include "stm32u575xx.h"
#include "config.h"
#include <stdint.h>

/* UART operation status codes. */
typedef enum
{
    UART_OK                  = 0,
    UART_ERROR_INIT          = 1,
    UART_ERROR_BUSY          = 2,
    UART_ERROR_TIMEOUT       = 3,
    UART_ERROR_INVALID_PARAM = 4,
} uart_status_t;

/* Initialize UART peripheral. Returns UART_OK on success. */
uart_status_t uart_init(void);

/* Send a single character (blocking). Returns UART_OK on success. */
uart_status_t uart_send_char(char c);

/* Send a null-terminated string (blocking). Returns UART_OK on success. */
uart_status_t uart_send_string(const char *str);

/* Receive a single character (blocking). Returns character on success, or error code if status !=
 * NULL. */
char uart_recv_char(void);

#if FEATURE_UART_DMA

#include "ringbuffer.h"

/* Callback event types. */
typedef enum
{
    UART_EVENT_TX_COMPLETE = 0, /* All queued TX data sent. */
    UART_EVENT_RX_DATA     = 1, /* Data available in RX buffer. */
    UART_EVENT_ERROR       = 2, /* Transmission or buffer error. */
} uart_event_t;

/* Callback signature: called from ISR when event occurs. Keep handler short! */
typedef void (*uart_callback_t)(uart_event_t event, void *context);

/* Initialize interrupt-driven UART. Must be called after uart_init().
 * Enables RX interrupt and sets up internal buffers. */
uart_status_t uart_dma_init(void);

/* Register callback for async events. Context is user data passed to callback.
 * Only one callback active at a time. Pass NULL callback to disable. */
uart_status_t uart_set_callback(uart_callback_t callback, void *context);

/* Send data via interrupt (non-blocking). Queues to TX ring buffer.
 * Returns UART_OK if queued, UART_ERROR_BUSY if buffer full. */
uart_status_t uart_send_async(const uint8_t *data, uint16_t len);

/* Start receiving data via interrupt (non-blocking). RX data buffered internally. */
uart_status_t uart_recv_async_start(void);

/* Check if data available in RX ring buffer (call from main, not ISR). */
uint16_t uart_recv_available(void);

/* Read byte from RX ring buffer (non-blocking). Returns byte on success, -1 if empty. */
int uart_recv_async_byte(void);

/* Get number of bytes pending in TX queue. */
uint16_t uart_send_pending(void);

/* Interrupt handler (call from USART1_IRQHandler or weak override in main.c). */
void uart_isr_handler(void);

#endif /* FEATURE_UART_DMA */

#endif /* UART_H */
