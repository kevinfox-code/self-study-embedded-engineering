/*
 * Author:      Kevin Fox
 * Book:        Bare-Metal Embedded C Programming
 *              by Israel Gbati — Packt, 2024
 * Description: Declares systick_msec_delay — a blocking millisecond delay function implemented using the ARM Cortex-M SysTick timer configured at the core clock frequency, with no HAL dependency.
 */
#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>  // For uint32_t from standard integer types
#include "stm32u575xx.h"

void systick_msec_delay(uint32_t msec);

#endif /* SYSTICK_H */