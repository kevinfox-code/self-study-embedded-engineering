/*
 * Author:      Kevin Fox
 * Description: SysTick driver for STM32U575 — configures the ARM Cortex-M SysTick timer for
 *              millisecond blocking delays using the 16 MHz HSI16 processor clock.
 *
 * Assumptions:
 * - System clock (HSI16) is 16 MHz at runtime
 * - Blocking delay; caller must account for execution time
 */
#include "systick.h"
/*By default the CPU is 16MHz*/
#define ONE_MSEC_LOAD 16000 // number of clock cycles per milisecond
void systick_init(void)
{
    /*No initialization needed for this simple delay implementation*/
}
void systick_msec_delay(uint32_t msec)
{
    /*Load number of clock cycles per milisecond*/
    SysTick->LOAD = ONE_MSEC_LOAD - 1;
    /*Clear the current value register*/
    SysTick->VAL = 0;
    /*Select internal clock source*/
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;
    /*Enable the counter*/
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    for (uint32_t i = 0; i < msec; i++)
    {
        /*Wait until the COUNTFLAG is set*/
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0)
            ;
    }
    /*Disable the counter*/
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}