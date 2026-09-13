/*
 * Author:      Kevin Fox
 * Book:        Making Embedded Systems: Design Patterns for Great Software
 *              by Elecia White — O'Reilly Media
 * Description: Declares the LED finite state machine interface — init, update (takes an event and current tick), and state query. Supports OFF, ON, and BLINKING states driven by button press events.
 */

#pragma once

#include <stdint.h>

typedef enum {
    LED_STATE_OFF      = 0,
    LED_STATE_ON,
    LED_STATE_BLINKING,
} LedState_t;

typedef enum {
    LED_EVENT_NONE         = 0,
    LED_EVENT_BUTTON_PRESS,
} LedEvent_t;

void       led_fsm_init(void);
void       led_fsm_update(LedEvent_t event, uint32_t tick_ms);
LedState_t led_fsm_get_state(void);
