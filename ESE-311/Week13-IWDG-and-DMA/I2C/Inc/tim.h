/*
 * Author:      Kevin Fox
 * Description: TIM2 driver header for STM32U575 — declares 1 Hz update event functions.
 */
#ifndef TIMERS_H
#define TIMERS_H

#include "stm32u575xx.h"
#include <stdbool.h>

void tim2_1hz_init(void);
bool tim2_update_event_ready(void);
void tim2_clear_update_event(void);

#endif // TIMERS_H