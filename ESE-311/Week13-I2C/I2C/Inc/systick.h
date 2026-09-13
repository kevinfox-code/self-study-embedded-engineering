/*
 * SPDX-License-Identifier: MIT
 * Author:      Kevin Fox
 * Book:        Bare-Metal Embedded C Programming
 *              by Israel Gbati — Packt, 2024
 * Description: SysTick timer driver API — declares init and blocking millisecond delay functions.
 */
#ifndef SYSTICK_H
#define SYSTICK_H

#include "stm32u575xx.h"
#include <stdint.h>

void systick_init(void);
void systick_msec_delay(uint32_t msec);

#endif /* SYSTICK_H */
