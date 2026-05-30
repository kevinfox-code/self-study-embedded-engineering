/*
 * Author:      Kevin Fox
 * Description: TIM2 driver for STM32U575 — configures TIM2 as a 1 Hz update event source.
 *
 * Assumptions:
 * - APB1 clock is 16 MHz (HSI16, no prescaler)
 * - TIM2 is not used by any other module
 */
#include "tim.h"

void tim2_1hz_init(void) {
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

    TIM2->CR1 &= ~TIM_CR1_CEN;
    TIM2->PSC = 4000U - 1U;
    TIM2->ARR = 1000U - 1U;
    TIM2->CNT = 0;

    TIM2->EGR = TIM_EGR_UG;
    TIM2->SR &= ~TIM_SR_UIF;
    TIM2->CR1 |= TIM_CR1_CEN;
}

bool tim2_update_event_ready(void) {
    return (TIM2->SR & TIM_SR_UIF) != 0;
}

void tim2_clear_update_event(void) {
    TIM2->SR &= ~TIM_SR_UIF;
}