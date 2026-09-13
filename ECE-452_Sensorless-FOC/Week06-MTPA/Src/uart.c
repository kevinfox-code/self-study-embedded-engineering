/*
 * Author:      Kevin Fox
 * Description: UART driver implementation for STM32U575 — basic character I/O.
 *
 * Added USART1 PA9/PA10 (AF7) connected to STLINK-V3E VCP - KF 25MAY2026
 * Feature toggles: DEBUG_ENABLED, FEATURE_UART_DMA (see Inc/config.h)
 *
 * Assumptions:
 * - System clock (HSI16) is 16 MHz at runtime
 * - USART1 PA9 (TX) and PA10 (RX) are available
 * - No other code modifies USART1 or GPIO clock/config
 * - Blocking functions have bounded execution time; caller provides timeouts
 */
#include "uart.h"
#include "config.h"
#include "debug.h"

#define USART1_EN         (1U << 14) // APB2ENR (USART1 clock enable)
#define DBG_UART_BAUDRATE 115200UL
#define SYS_FREQ_HZ       16000000UL // 16 MHz HSI16 system clock
#define APB2_CLK_Hz       (SYS_FREQ_HZ)
#define CR1_TE            (1U << 3) // Transmitter enable
#define CR1_RE            (1U << 2) // Receiver enable
#define CR1_UE            (1U << 0) // USART enable
#define SR_TXE            (1U << 7) // Transmit data register empty
#define SR_RXNE           (1U << 5) // Read data register not empty

// GPIOA - USART1 PA9 (TX) and PA10 (RX)
#define GPIOA_EN (1U << 0)

static uint32_t compute_uart_bd(uint32_t peripheral_clock, uint32_t baudrate);
static void     uart_set_baudrate(uint32_t peripheral_clock, uint32_t baudrate);

int __io_putchar(int ch)
{
    uart_send_char((char)ch);
    return ch;
}
uart_status_t uart_init(void)
{
    /*Enable clock access to GPIOA */
    RCC_NS->AHB2ENR1 |= GPIOA_EN;
    /*Set PA9 and PA10 as alternate function */
    GPIOA_NS->MODER &= ~(3U << 18); // Clear bits [19:18] for PA9
    GPIOA_NS->MODER |= (2U << 18);  // Set bits [19:18] to alternate function for PA9
    GPIOA_NS->MODER &= ~(3U << 20); // Clear bits [21:20] for PA10
    GPIOA_NS->MODER |= (2U << 20);  // Set bits [21:20] to alternate function for PA10
    /*Set alternate function for PA9 and PA10 */
    GPIOA_NS->AFR[1] &= ~(0xFU << 4); // Clear bits [7:4] for PA9
    GPIOA_NS->AFR[1] |= (7U << 4);    // Set bits [7:4] to USART1 for PA9
    GPIOA_NS->AFR[1] &= ~(0xFU << 8); // Clear bits [11:8] for PA10
    GPIOA_NS->AFR[1] |= (7U << 8);    // Set bits [11:8] to USART1 for PA10
    /*Enable clock access to USART1 */
    RCC_NS->APB2ENR |= USART1_EN;
    /*Configure baud rate */
    uart_set_baudrate(APB2_CLK_Hz, DBG_UART_BAUDRATE);
    /*Configure transfer direction: enable transmitter and receiver */
    USART1_NS->CR1 |= (CR1_TE | CR1_RE);
    /*Enable USART1 */
    USART1_NS->CR1 |= CR1_UE;
    return UART_OK;
}

static uint32_t compute_uart_bd(uint32_t peripheral_clock, uint32_t baudrate)
{
    /* STM32U5 BRR = USARTDIV = fCK / baudrate (plain integer, oversampling by 16).
     * Add baudrate/2 for rounding.
     */
    return (peripheral_clock + (baudrate / 2U)) / baudrate;
}

static void uart_set_baudrate(uint32_t peripheral_clock, uint32_t baudrate)
{
    uint32_t ubrr  = compute_uart_bd(peripheral_clock, baudrate);
    USART1_NS->BRR = ubrr;
}

uart_status_t uart_send_char(char c)
{
    /* Busy-wait for transmit buffer: bounded by CR1_TE check before entry. */
    while (!(USART1_NS->ISR & SR_TXE))
        ;
    USART1_NS->TDR = (uint32_t)(uint8_t)c;
    return UART_OK;
}

uart_status_t uart_send_string(const char *str)
{
    if (!str)
    {
        UART_ASSERT(0); /* Caller error: NULL pointer passed. */
        return UART_ERROR_INVALID_PARAM;
    }
    while (*str)
    {
        uart_send_char(*str++);
    }
    return UART_OK;
}

char uart_recv_char(void)
{
    /* Wait until a character is received */
    while (!(USART1_NS->ISR & SR_RXNE))
        ;
    /* Read lower 8 bits from the RDR */
    return (char)(USART1_NS->RDR & 0xFF);
}

#if FEATURE_UART_DMA

#define UART_RX_BUFFER_SIZE 256U
#define UART_TX_BUFFER_SIZE 256U

static uint8_t      uartRxBuffer[UART_RX_BUFFER_SIZE];
static uint8_t      uartTxBuffer[UART_TX_BUFFER_SIZE];
static ringbuffer_t rxRingBuffer;
static ringbuffer_t txRingBuffer;

/* Callback registration. */
static uart_callback_t uartCallback        = NULL;
static void           *uartCallbackContext = NULL;

uart_status_t uart_dma_init(void)
{
    /* Initialize ring buffers for interrupt-driven operation. */
    if (ringbuffer_init(&rxRingBuffer, uartRxBuffer, UART_RX_BUFFER_SIZE) != 0)
    {
        return UART_ERROR_INIT;
    }
    if (ringbuffer_init(&txRingBuffer, uartTxBuffer, UART_TX_BUFFER_SIZE) != 0)
    {
        return UART_ERROR_INIT;
    }

    /* Enable RX interrupt (RXNE). */
    USART1_NS->CR1 |= (1U << 5);

    /* Configure NVIC for USART1_IRQn (priority 1). */
    NVIC_SetPriority(USART1_IRQn, 1);
    NVIC_EnableIRQ(USART1_IRQn);

    return UART_OK;
}

uart_status_t uart_set_callback(uart_callback_t callback, void *context)
{
    uartCallback        = callback;
    uartCallbackContext = context;
    return UART_OK;
}

uart_status_t uart_send_async(const uint8_t *data, uint16_t len)
{
    if (!data || len == 0)
    {
        return UART_ERROR_INVALID_PARAM;
    }

    for (uint16_t i = 0; i < len; i++)
    {
        if (ringbuffer_write(&txRingBuffer, data[i]) != 0)
        {
            return UART_ERROR_BUSY; /* TX buffer full */
        }
    }

    /* Enable TX interrupt to drain buffer. */
    USART1_NS->CR1 |= (1U << 7);

    return UART_OK;
}

uart_status_t uart_recv_async_start(void)
{
    /* RX interrupt already enabled in uart_dma_init(). */
    return UART_OK;
}

uint16_t uart_recv_available(void)
{
    return ringbuffer_available(&rxRingBuffer);
}

int uart_recv_async_byte(void)
{
    return ringbuffer_read(&rxRingBuffer);
}

uint16_t uart_send_pending(void)
{
    return ringbuffer_available(&txRingBuffer);
}

void uart_isr_handler(void)
{
    uint32_t isrStatus       = USART1_NS->ISR;
    uint8_t  rxDataAvailable = 0;
    uint8_t  txComplete      = 0;

    /* RX: read character and buffer it */
    if (isrStatus & SR_RXNE)
    {
        uint8_t byte = (uint8_t)(USART1_NS->RDR & 0xFF);
        if (ringbuffer_write(&rxRingBuffer, byte) == 0)
        {
            rxDataAvailable = 1;
        }
    }

    /* TX: send next buffered character */
    if ((isrStatus & SR_TXE) && (USART1_NS->CR1 & (1U << 7)))
    {
        int byte = ringbuffer_read(&txRingBuffer);
        if (byte >= 0)
        {
            USART1_NS->TDR = (uint32_t)(uint8_t)byte;
        }
        else
        {
            /* TX buffer empty: disable TX interrupt */
            USART1_NS->CR1 &= ~(1U << 7);
            txComplete = 1;
        }
    }

    /* Invoke callbacks if registered. */
    if (uartCallback)
    {
        if (rxDataAvailable)
        {
            uartCallback(UART_EVENT_RX_DATA, uartCallbackContext);
        }
        if (txComplete)
        {
            uartCallback(UART_EVENT_TX_COMPLETE, uartCallbackContext);
        }
    }
}

#endif /* FEATURE_UART_DMA */
