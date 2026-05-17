/*
 * Test suite for the LED state machine pattern (Making Embedded Systems p. 279).
 *
 * Build and run on host:
 *   cmake -S .. -B build && cmake --build build && ctest --test-dir build -V
 *
 * Or directly:
 *   gcc -Wall -Wextra -std=c11 -I../src \
 *       test_led_fsm.c hal_mock.c ../src/led_fsm.c -o test_led_fsm && ./test_led_fsm
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "led_fsm.h"

/* Mock control functions defined in hal_mock.c */
extern bool hal_mock_led_get(void);
extern int  hal_mock_led_set_count(void);
extern void hal_mock_reset(void);

/* ------------------------------------------------------------------ */
/* Minimal test framework (same style as Week05)                      */
/* ------------------------------------------------------------------ */

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, name)                                                \
    do {                                                                 \
        if (cond) {                                                      \
            printf("PASS  %s\n", (name));                               \
            g_pass++;                                                    \
        } else {                                                         \
            printf("FAIL  %s  (line %d)\n", (name), __LINE__);         \
            g_fail++;                                                    \
        }                                                                \
    } while (0)

static void reset(void)
{
    hal_mock_reset();
    led_fsm_init();
}

/* ------------------------------------------------------------------ */
/* Tests                                                              */
/* ------------------------------------------------------------------ */

static int test_initial_state(void)
{
    reset();
    CHECK(led_fsm_get_state() == LED_STATE_OFF, "initial state is OFF");
    CHECK(hal_mock_led_get() == false,          "LED off after init");
    return 1;
}

static int test_off_to_on(void)
{
    reset();
    led_fsm_update(LED_EVENT_BUTTON_PRESS, 0);
    CHECK(led_fsm_get_state() == LED_STATE_ON, "OFF + press -> ON");
    CHECK(hal_mock_led_get() == true,          "LED on after OFF->ON");
    return 1;
}

static int test_on_to_blinking(void)
{
    reset();
    led_fsm_update(LED_EVENT_BUTTON_PRESS, 0);
    led_fsm_update(LED_EVENT_BUTTON_PRESS, 0);
    CHECK(led_fsm_get_state() == LED_STATE_BLINKING, "ON + press -> BLINKING");
    CHECK(hal_mock_led_get() == true, "LED on immediately on entering BLINKING");
    return 1;
}

static int test_blinking_to_off(void)
{
    reset();
    led_fsm_update(LED_EVENT_BUTTON_PRESS, 0);
    led_fsm_update(LED_EVENT_BUTTON_PRESS, 0);
    led_fsm_update(LED_EVENT_BUTTON_PRESS, 0);
    CHECK(led_fsm_get_state() == LED_STATE_OFF, "BLINKING + press -> OFF");
    CHECK(hal_mock_led_get() == false,          "LED off after BLINKING->OFF");
    return 1;
}

static int test_no_event_holds_state(void)
{
    reset();
    led_fsm_update(LED_EVENT_NONE, 0);
    CHECK(led_fsm_get_state() == LED_STATE_OFF, "no event in OFF keeps OFF");

    led_fsm_update(LED_EVENT_BUTTON_PRESS, 0);  /* -> ON */
    led_fsm_update(LED_EVENT_NONE, 0);
    CHECK(led_fsm_get_state() == LED_STATE_ON, "no event in ON keeps ON");
    return 1;
}

static int test_blink_toggles_at_500ms(void)
{
    reset();
    led_fsm_update(LED_EVENT_BUTTON_PRESS, 0);   /* OFF -> ON    */
    led_fsm_update(LED_EVENT_BUTTON_PRESS, 0);   /* ON  -> BLINKING, anchor t=0, LED on */

    led_fsm_update(LED_EVENT_NONE, 499);
    CHECK(hal_mock_led_get() == true, "LED still on before 500 ms");

    led_fsm_update(LED_EVENT_NONE, 500);
    CHECK(hal_mock_led_get() == false, "LED off at 500 ms");

    led_fsm_update(LED_EVENT_NONE, 1000);
    CHECK(hal_mock_led_get() == true, "LED on at 1000 ms");

    led_fsm_update(LED_EVENT_NONE, 1500);
    CHECK(hal_mock_led_get() == false, "LED off at 1500 ms");
    return 1;
}

static int test_blink_timer_anchored_to_entry_tick(void)
{
    /*
     * Enter BLINKING at t=1000 ms.  The 500 ms period must be measured
     * from that entry tick, not from t=0.
     */
    reset();
    led_fsm_update(LED_EVENT_BUTTON_PRESS, 1000);  /* OFF -> ON      */
    led_fsm_update(LED_EVENT_BUTTON_PRESS, 1000);  /* ON  -> BLINKING, anchor t=1000 */

    led_fsm_update(LED_EVENT_NONE, 1499);
    CHECK(hal_mock_led_get() == true, "no toggle before 500 ms from BLINKING entry");

    led_fsm_update(LED_EVENT_NONE, 1500);
    CHECK(hal_mock_led_get() == false, "toggle exactly 500 ms after BLINKING entry");
    return 1;
}

static int test_full_cycle_repeats(void)
{
    reset();

    for (int cycle = 0; cycle < 3; cycle++) {
        led_fsm_update(LED_EVENT_BUTTON_PRESS, 0);
        led_fsm_update(LED_EVENT_BUTTON_PRESS, 0);
        led_fsm_update(LED_EVENT_BUTTON_PRESS, 0);
        CHECK(led_fsm_get_state() == LED_STATE_OFF, "cycle ends at OFF");
    }
    return 1;
}

static int test_button_in_blinking_stops_blink_timer(void)
{
    /*
     * After BLINKING -> OFF the LED must be off and stay off even if
     * further tick calls arrive that would have crossed the 500 ms boundary.
     */
    reset();
    led_fsm_update(LED_EVENT_BUTTON_PRESS, 0);   /* -> ON      */
    led_fsm_update(LED_EVENT_BUTTON_PRESS, 0);   /* -> BLINKING, anchor t=0 */
    led_fsm_update(LED_EVENT_BUTTON_PRESS, 0);   /* -> OFF     */

    led_fsm_update(LED_EVENT_NONE, 500);         /* would have been a toggle */
    CHECK(led_fsm_get_state() == LED_STATE_OFF, "state stays OFF after exit from BLINKING");
    CHECK(hal_mock_led_get() == false,           "LED off on tick after BLINKING->OFF");
    return 1;
}

/* ------------------------------------------------------------------ */
/* Main                                                               */
/* ------------------------------------------------------------------ */

int main(void)
{
    printf("=== LED State Machine — Test Suite ===\n\n");

    test_initial_state();
    test_off_to_on();
    test_on_to_blinking();
    test_blinking_to_off();
    test_no_event_holds_state();
    test_blink_toggles_at_500ms();
    test_blink_timer_anchored_to_entry_tick();
    test_full_cycle_repeats();
    test_button_in_blinking_stops_blink_timer();

    int total = g_pass + g_fail;
    printf("\n======================================\n");
    printf("Results: %d/%d passed", g_pass, total);
    if (g_fail > 0) {
        printf("   *** %d FAILED ***", g_fail);
    }
    printf("\n======================================\n");

    return (g_fail == 0) ? 0 : 1;
}
