/*
 * SPDX-License-Identifier: MIT
 * Author:      Kevin Fox
 * Book:        Bare-Metal Embedded C Programming
 *              by Israel Gbati — Packt, 2024
 * Description: Implements the SysTick millisecond delay — configures the ARM Cortex-M SysTick timer
 *              against the internal 16 MHz processor clock and polls COUNTFLAG, with no HAL dependency.
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
