/*
 * SPDX-License-Identifier: MIT
 * Author:      Kevin Fox
 * Book:        Making Embedded Systems: Design Patterns for Great Software
 *              by Elecia White — O'Reilly Media
 * Description: Implements a three-state LED FSM (OFF → ON → BLINKING → OFF) driven by BUTTON_PRESS events and a millisecond tick for blink timing, using the HAL interface for all hardware output.
 */

/*
 * LED state machine — Elecia White pattern (Making Embedded Systems, p. 279):
 *
 *   case (state):
 *     make sure current state is actively doing what it needs
 *     if event valid for this state
 *       call next state function
 *
 * States:   OFF -> ON -> BLINKING -> OFF  (each button press advances one step)
 * Blinking: LED toggles every BLINK_PERIOD_MS milliseconds.
 */

#include "led_fsm.h"
#include "hal.h"

#define BLINK_PERIOD_MS 500u

static LedState_t g_state;
static uint32_t   g_blink_last_ms;
static bool       g_blink_led_on;

/* ------------------------------------------------------------------ */
/* State entry functions — called once on transition                  */
/* ------------------------------------------------------------------ */

static void enter_off(void)
{
    hal_led_set(false);
    g_state = LED_STATE_OFF;
}

static void enter_on(void)
{
    hal_led_set(true);
    g_state = LED_STATE_ON;
}

static void enter_blinking(uint32_t tick_ms)
{
    g_blink_last_ms = tick_ms;
    g_blink_led_on  = true;
    hal_led_set(true);
    g_state = LED_STATE_BLINKING;
}

/* ------------------------------------------------------------------ */
/* Public API                                                         */
/* ------------------------------------------------------------------ */

void led_fsm_init(void)
{
    enter_off();
}

void led_fsm_update(LedEvent_t event, uint32_t tick_ms)
{
    switch (g_state) {

        case LED_STATE_OFF:
            hal_led_set(false);                            /* ongoing: keep LED off  */
            if (event == LED_EVENT_BUTTON_PRESS) {
                enter_on();
            }
            break;

        case LED_STATE_ON:
            hal_led_set(true);                             /* ongoing: keep LED on   */
            if (event == LED_EVENT_BUTTON_PRESS) {
                enter_blinking(tick_ms);
            }
            break;

        case LED_STATE_BLINKING:
            if ((tick_ms - g_blink_last_ms) >= BLINK_PERIOD_MS) { /* ongoing: toggle */
                g_blink_led_on  = !g_blink_led_on;
                hal_led_set(g_blink_led_on);
                g_blink_last_ms = tick_ms;
            }
            if (event == LED_EVENT_BUTTON_PRESS) {
                enter_off();
            }
            break;
    }
}

LedState_t led_fsm_get_state(void)
{
    return g_state;
}
