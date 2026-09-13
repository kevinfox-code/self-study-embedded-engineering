/*
 * Host-runnable unit tests for the UART driver.
 *
 * Build with:
 *   cc -std=c11 -Wall -Wextra -DFEATURE_UART_DMA=1 -DDEBUG_ENABLED=0 \
 *      -Itests/mocks -IInc \
 *      tests/test_uart.c tests/mocks/mock_stm32u575xx.c Src/uart.c Src/ringbuffer.c \
 *      -o test_uart
 * Run with:
 *   ./test_uart
 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "mock_stm32u575xx.h"
#include "uart.h"

#define ANSI_GREEN "\x1b[32m"
#define ANSI_RED   "\x1b[31m"
#define ANSI_RESET "\x1b[0m"

static int tests_passed = 0;

typedef struct
{
    uart_event_t events[4];
    void        *context;
    size_t       count;
} callback_log_t;

static callback_log_t callback_log;

#define RUN_TEST(fn, description)                                                                  \
    do                                                                                             \
    {                                                                                              \
        printf("Running: %s ... ", description);                                                   \
        fflush(stdout);                                                                            \
        fn();                                                                                      \
        printf(ANSI_GREEN "PASS" ANSI_RESET "\n");                                                 \
        tests_passed++;                                                                            \
    } while (0)

static void reset_callback_log(void)
{
    callback_log.count   = 0;
    callback_log.context = NULL;
    for (size_t i = 0; i < (sizeof(callback_log.events) / sizeof(callback_log.events[0])); ++i)
    {
        callback_log.events[i] = UART_EVENT_ERROR;
    }
}

static void record_uart_event(uart_event_t event, void *context)
{
    if (callback_log.count < (sizeof(callback_log.events) / sizeof(callback_log.events[0])))
    {
        callback_log.events[callback_log.count] = event;
        callback_log.context                    = context;
        callback_log.count++;
    }
}

static void prepare_uart_async(void)
{
    mock_stm32_reset();
    reset_callback_log();
    assert(uart_init() == UART_OK);
    assert(uart_dma_init() == UART_OK);
    assert(uart_set_callback(NULL, NULL) == UART_OK);
}

static void test_uart_init_configures_gpio_and_usart(void)
{
    mock_stm32_reset();

    assert(uart_init() == UART_OK);

    assert((RCC_NS->AHB2ENR1 & (1U << 0)) != 0U);
    assert((RCC_NS->APB2ENR & (1U << 14)) != 0U);
    assert(GPIOA_NS->MODER == ((2U << 18) | (2U << 20)));
    assert(GPIOA_NS->AFR[1] == ((7U << 4) | (7U << 8)));
    assert(USART1_NS->BRR == 139U);
    assert((USART1_NS->CR1 & ((1U << 3) | (1U << 2) | (1U << 0))) ==
           ((1U << 3) | (1U << 2) | (1U << 0)));
}

static void test_uart_send_char_writes_tdr_when_txe_set(void)
{
    mock_stm32_reset();
    USART1_NS->ISR = (1U << 7);

    assert(uart_send_char((char)0xAB) == UART_OK);
    assert(USART1_NS->TDR == 0xABU);
}

static void test_uart_send_string_transmits_each_character(void)
{
    mock_stm32_reset();
    USART1_NS->ISR = (1U << 7);

    assert(uart_send_string("Hi") == UART_OK);
    assert(USART1_NS->TDR == (uint32_t)'i');
}

static void test_uart_send_string_null_returns_invalid_param(void)
{
    mock_stm32_reset();

    assert(uart_send_string(NULL) == UART_ERROR_INVALID_PARAM);
}

static void test_uart_recv_char_reads_rdr_when_rxne_set(void)
{
    mock_stm32_reset();
    USART1_NS->ISR = (1U << 5);
    USART1_NS->RDR = (uint32_t)'Z';

    assert(uart_recv_char() == 'Z');
}

static void test_uart_dma_init_enables_rx_interrupt_and_nvic(void)
{
    mock_stm32_reset();
    assert(uart_init() == UART_OK);

    assert(uart_dma_init() == UART_OK);
    assert((USART1_NS->CR1 & (1U << 5)) != 0U);
    assert(mock_nvic_set_priority_calls == 1U);
    assert(mock_last_irqn == USART1_IRQn);
    assert(mock_last_priority == 1U);
    assert(mock_nvic_enable_irq_calls == 1U);
    assert(mock_last_enabled_irqn == USART1_IRQn);
}

static void test_uart_send_async_rejects_invalid_inputs(void)
{
    prepare_uart_async();

    assert(uart_send_async(NULL, 4) == UART_ERROR_INVALID_PARAM);
    assert(uart_send_async((const uint8_t *)"", 0) == UART_ERROR_INVALID_PARAM);
}

static void test_uart_send_async_queues_bytes_and_enables_txe_irq(void)
{
    prepare_uart_async();

    const uint8_t payload[] = {0x11U, 0x22U, 0x33U};
    assert(uart_send_async(payload, (uint16_t)sizeof(payload)) == UART_OK);
    assert(uart_send_pending() == (uint16_t)sizeof(payload));
    assert((USART1_NS->CR1 & (1U << 7)) != 0U);
}

static void test_uart_send_async_reports_busy_when_buffer_full(void)
{
    prepare_uart_async();

    uint8_t payload[255];
    for (size_t i = 0; i < sizeof(payload); ++i)
    {
        payload[i] = (uint8_t)i;
    }

    assert(uart_send_async(payload, (uint16_t)sizeof(payload)) == UART_OK);
    assert(uart_send_async((const uint8_t *)"!", 1) == UART_ERROR_BUSY);
}

static void test_uart_recv_async_byte_returns_minus_one_when_empty(void)
{
    prepare_uart_async();

    assert(uart_recv_async_byte() == -1);
    assert(uart_recv_available() == 0U);
}

static void test_uart_isr_handler_pushes_rx_byte_and_calls_callback(void)
{
    prepare_uart_async();

    void *context = &callback_log;
    assert(uart_set_callback(record_uart_event, context) == UART_OK);

    USART1_NS->ISR = (1U << 5);
    USART1_NS->RDR = (uint32_t)'Q';
    uart_isr_handler();

    assert(uart_recv_available() == 1U);
    assert(uart_recv_async_byte() == 'Q');
    assert(callback_log.count == 1U);
    assert(callback_log.events[0] == UART_EVENT_RX_DATA);
    assert(callback_log.context == context);
}

static void test_uart_isr_handler_completes_tx_and_clears_txe_irq(void)
{
    prepare_uart_async();

    void         *context   = &callback_log;
    const uint8_t payload[] = {0x5AU};
    assert(uart_set_callback(record_uart_event, context) == UART_OK);
    assert(uart_send_async(payload, (uint16_t)sizeof(payload)) == UART_OK);

    USART1_NS->ISR = (1U << 7);
    uart_isr_handler();
    assert(uart_send_pending() == 0U);
    assert((USART1_NS->CR1 & (1U << 7)) != 0U);

    uart_isr_handler();
    assert(uart_send_pending() == 0U);
    assert((USART1_NS->CR1 & (1U << 7)) == 0U);
    assert(callback_log.count == 1U);
    assert(callback_log.events[0] == UART_EVENT_TX_COMPLETE);
    assert(callback_log.context == context);
    assert(USART1_NS->TDR == 0x5AU);
}

int main(void)
{
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║            UART Host Unit Test Suite                       ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    RUN_TEST(test_uart_init_configures_gpio_and_usart,
             "uart_init configures GPIOA, USART1, and BRR");
    RUN_TEST(test_uart_send_char_writes_tdr_when_txe_set,
             "uart_send_char writes a byte when TXE is set");
    RUN_TEST(test_uart_send_string_transmits_each_character,
             "uart_send_string pushes all characters");
    RUN_TEST(test_uart_send_string_null_returns_invalid_param,
             "uart_send_string rejects NULL input");
    RUN_TEST(test_uart_recv_char_reads_rdr_when_rxne_set, "uart_recv_char reads a received byte");
    RUN_TEST(test_uart_dma_init_enables_rx_interrupt_and_nvic,
             "uart_dma_init configures RX interrupt and NVIC");
    RUN_TEST(test_uart_send_async_rejects_invalid_inputs,
             "uart_send_async rejects NULL and empty payloads");
    RUN_TEST(test_uart_send_async_queues_bytes_and_enables_txe_irq,
             "uart_send_async queues data and enables TXE IRQ");
    RUN_TEST(test_uart_send_async_reports_busy_when_buffer_full,
             "uart_send_async reports buffer full");
    RUN_TEST(test_uart_recv_async_byte_returns_minus_one_when_empty,
             "uart_recv_async_byte reports empty buffer");
    RUN_TEST(test_uart_isr_handler_pushes_rx_byte_and_calls_callback,
             "uart_isr_handler stores RX bytes and triggers RX callback");
    RUN_TEST(test_uart_isr_handler_completes_tx_and_clears_txe_irq,
             "uart_isr_handler drains TX data and triggers completion callback");

    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║ Test Summary                                               ║\n");
    printf("║ Passed: %-3d   Total: %-3d                                  ║\n", tests_passed,
           tests_passed);
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    printf(ANSI_GREEN "SUCCESS: All %d tests passed!\n" ANSI_RESET, tests_passed);
    return 0;
}
