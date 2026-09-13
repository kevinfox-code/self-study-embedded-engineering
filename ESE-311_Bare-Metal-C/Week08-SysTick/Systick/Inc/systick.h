/*
 * SPDX-License-Identifier: MIT
 * Author:      Kevin Fox
 * Book:        Bare-Metal Embedded C Programming
 *              by Israel Gbati — Packt, 2024
 * Description: SysTick timer driver API — declares init and blocking millisecond delay functions.
 */
#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>  // For uint32_t from standard integer types
#include "stm32u575xx.h"

void systick_msec_delay(uint32_t msec);

#endif /* SYSTICK_H */