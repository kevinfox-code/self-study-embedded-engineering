/*
 * SPDX-License-Identifier: MIT
 * Author:      Kevin Fox
 * Book:        Bare-Metal Embedded C Programming
 *              by Israel Gbati — Packt, 2024
 * Description: TIM2 general-purpose timer API — 1 Hz init plus update-event poll and clear helpers.
 */
#ifndef TIM_H
#define TIM_H

#include "stm32u575xx.h"
#include <stdbool.h>

void tim2_1hz_init(void);
bool tim2_update_event_ready(void);
void tim2_clear_update_event(void);

#endif // TIM_H