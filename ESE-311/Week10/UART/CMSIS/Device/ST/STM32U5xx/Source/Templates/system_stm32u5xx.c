/*
 * Minimal CMSIS system file for the STM32U575 Week 10 UART project.
 * Uses stm32u575xx.h directly (no STM32U575xx define needed).
 */
#include "stm32u575xx.h"

#ifndef VECT_TAB_OFFSET
#define VECT_TAB_OFFSET 0x00000000UL
#endif

uint32_t SystemCoreClock = 16000000U;

const uint8_t AHBPrescTable[16] = {
    0U, 0U, 0U, 0U, 1U, 2U, 3U, 4U,
    6U, 7U, 8U, 9U, 10U, 11U, 12U, 13U
};

const uint8_t APBPrescTable[8] = {
    0U, 0U, 0U, 0U, 1U, 2U, 3U, 4U
};

const uint32_t MSIRangeTable[16] = {
    100000U, 200000U, 400000U, 800000U,
    1000000U, 2000000U, 4000000U, 8000000U,
    16000000U, 24000000U, 32000000U, 48000000U,
    64000000U, 80000000U, 96000000U, 120000000U
};

void SystemInit(void) {
    /* Enable FPU (Cortex-M33 has FPU) */
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 20U) | (3UL << 22U));
#endif

    /* Reset RCC to default state (matches Cube-generated init) */
    RCC_NS->CR |= RCC_CR_MSISON;
    RCC_NS->CFGR1 = 0U;
    RCC_NS->CFGR2 = 0U;
    RCC_NS->CFGR3 = 0U;
    RCC_NS->CR &= ~(RCC_CR_HSEON | RCC_CR_CSSON | RCC_CR_PLL1ON | RCC_CR_PLL2ON | RCC_CR_PLL3ON);
    RCC_NS->PLL1CFGR = 0U;
    RCC_NS->CR &= ~RCC_CR_HSEBYP;
    RCC_NS->CIER = 0U;

    /* Relocate vector table to flash */
    SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET;

    /* Switch SYSCLK to HSI16 (16 MHz) so UART BRR and SysTick are correct */
    RCC_NS->CR |= RCC_CR_HSION;
    while (!(RCC_NS->CR & RCC_CR_HSIRDY));
    RCC_NS->CFGR1 = (RCC_NS->CFGR1 & ~RCC_CFGR1_SW_Msk) | RCC_CFGR1_SW_0;
    while ((RCC_NS->CFGR1 & RCC_CFGR1_SWS_Msk) != RCC_CFGR1_SWS_0);
    SystemCoreClock = 16000000U;
}

void SystemCoreClockUpdate(void) {
    SystemCoreClock = 16000000U;
}
