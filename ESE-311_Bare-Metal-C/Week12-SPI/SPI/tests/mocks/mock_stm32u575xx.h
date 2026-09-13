#ifndef MOCK_STM32U575XX_H
#define MOCK_STM32U575XX_H

#include <stdint.h>

typedef struct
{
    uint32_t MODER;
    uint32_t AFR[2];
} GPIO_TypeDef;

typedef struct
{
    uint32_t AHB2ENR1;
    uint32_t APB2ENR;
} RCC_TypeDef;

typedef struct
{
    uint32_t CR1;
    uint32_t BRR;
    uint32_t ISR;
    uint32_t TDR;
    uint32_t RDR;
} USART_TypeDef;

extern RCC_TypeDef   mock_rcc_ns;
extern GPIO_TypeDef  mock_gpioa_ns;
extern USART_TypeDef mock_usart1_ns;

#define RCC_NS    (&mock_rcc_ns)
#define GPIOA_NS  (&mock_gpioa_ns)
#define USART1_NS (&mock_usart1_ns)

#define USART1_IRQn 37

extern unsigned int mock_nvic_set_priority_calls;
extern unsigned int mock_nvic_enable_irq_calls;
extern int          mock_last_irqn;
extern unsigned int mock_last_priority;
extern int          mock_last_enabled_irqn;

void NVIC_SetPriority(int irqn, unsigned int priority);
void NVIC_EnableIRQ(int irqn);
void mock_stm32_reset(void);

#endif /* MOCK_STM32U575XX_H */