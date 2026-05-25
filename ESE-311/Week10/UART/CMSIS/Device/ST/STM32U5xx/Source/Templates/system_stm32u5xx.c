/*
 * Minimal CMSIS system file for the STM32U575 Week 10 UART project.
 * The startup code only needs the symbols below to link successfully.
 */
#include "stm32u575xx.h"

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
    SystemCoreClock = 16000000U;
}

void SystemCoreClockUpdate(void) {
    SystemCoreClock = 16000000U;
}

uint32_t SECURE_SystemCoreClockUpdate(void) {
    SystemCoreClockUpdate();
    return SystemCoreClock;
}