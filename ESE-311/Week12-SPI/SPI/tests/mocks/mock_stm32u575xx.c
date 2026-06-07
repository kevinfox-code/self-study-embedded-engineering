#include "mock_stm32u575xx.h"

#include <string.h>

RCC_TypeDef mock_rcc_ns;
GPIO_TypeDef mock_gpioa_ns;
USART_TypeDef mock_usart1_ns;

unsigned int mock_nvic_set_priority_calls;
unsigned int mock_nvic_enable_irq_calls;
int mock_last_irqn;
unsigned int mock_last_priority;
int mock_last_enabled_irqn;

void NVIC_SetPriority(int irqn, unsigned int priority)
{
    mock_nvic_set_priority_calls++;
    mock_last_irqn = irqn;
    mock_last_priority = priority;
}

void NVIC_EnableIRQ(int irqn)
{
    mock_nvic_enable_irq_calls++;
    mock_last_enabled_irqn = irqn;
}

void mock_stm32_reset(void)
{
    memset(&mock_rcc_ns, 0, sizeof(mock_rcc_ns));
    memset(&mock_gpioa_ns, 0, sizeof(mock_gpioa_ns));
    memset(&mock_usart1_ns, 0, sizeof(mock_usart1_ns));
    mock_nvic_set_priority_calls = 0U;
    mock_nvic_enable_irq_calls = 0U;
    mock_last_irqn = -1;
    mock_last_priority = 0U;
    mock_last_enabled_irqn = -1;
}