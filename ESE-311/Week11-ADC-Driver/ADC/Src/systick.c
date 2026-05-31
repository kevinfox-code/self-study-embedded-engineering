/*
 * Author:      Kevin Fox
 * Description: Interrupt-driven SysTick background tick (1 ms).
 * Replaces the previous blocking implementation with an ISR-driven millisecond
 * tick counter. Keeps the same API: systick_init() and systick_msec_delay().
 */

#include "systick.h"

/* Default CPU clock assumed by this project: 16 MHz. Keep minimal changes.
 * If SystemCoreClock is available, this can be adapted later.
 */
#define ONE_MSEC_LOAD   16000U          /* number of clock cycles per millisecond */

static volatile uint32_t ms_ticks = 0U;

void systick_init(void) {
    /* Configure SysTick for 1 ms interrupts */
    SysTick->LOAD = ONE_MSEC_LOAD - 1U;
    SysTick->VAL  = 0U; /* clear current value */
    /* Use core clock, enable SysTick interrupt, enable counter */
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler(void) {
    ms_ticks++;
}

void systick_msec_delay(uint32_t msec) {
    uint32_t target = ms_ticks + msec;
    /* Wait until ms_ticks reaches target; use WFI to sleep between ticks */
    while (ms_ticks < target) {
        __WFI();
    }
}
