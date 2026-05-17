/*
 * LED state machine sandbox — runs on host, no hardware required.
 *
 * Simulates three button presses that cycle:
 *   OFF  ->  ON  ->  BLINKING  ->  OFF
 *
 * Then re-enters the cycle and fast-forwards the simulated clock to
 * show the LED toggling every 500 ms while in the BLINKING state.
 */

#include <stdio.h>
#include <stdint.h>
#include "led_fsm.h"

static const char *state_name(LedState_t s)
{
    switch (s) {
        case LED_STATE_OFF:      return "OFF";
        case LED_STATE_ON:       return "ON";
        case LED_STATE_BLINKING: return "BLINKING";
        default:                 return "UNKNOWN";
    }
}

int main(void)
{
    printf("=== LED State Machine Sandbox ===\n\n");

    led_fsm_init();
    printf("Initial state : %s\n\n", state_name(led_fsm_get_state()));

    printf("--- Button press 1 ---\n");
    led_fsm_update(LED_EVENT_BUTTON_PRESS, 0);
    printf("State: %s\n\n", state_name(led_fsm_get_state()));

    printf("--- Button press 2 ---\n");
    led_fsm_update(LED_EVENT_BUTTON_PRESS, 0);
    printf("State: %s\n\n", state_name(led_fsm_get_state()));

    printf("--- Simulating blink ticks (t = 0 .. 2000 ms, step 500 ms) ---\n");
    for (uint32_t t = 0; t <= 2000u; t += 500u) {
        printf("  tick %4u ms -> ", (unsigned)t);
        led_fsm_update(LED_EVENT_NONE, t);
        printf("state %s\n", state_name(led_fsm_get_state()));
    }
    printf("\n");

    printf("--- Button press 3 (exits BLINKING) ---\n");
    led_fsm_update(LED_EVENT_BUTTON_PRESS, 2000);
    printf("State: %s\n\n", state_name(led_fsm_get_state()));

    printf("=== Done ===\n");
    return 0;
}
