/*
 * Mock HAL — records LED state so tests can assert against it.
 * Call hal_mock_reset() between tests to clear recorded state.
 */

#include "hal.h"

static bool g_led_on        = false;
static int  g_led_set_count = 0;

void hal_led_set(bool on)
{
    g_led_on = on;
    g_led_set_count++;
}

bool     hal_button_pressed(void) { return false; }
uint32_t hal_tick_ms(void)        { return 0u; }

/* Test control API — forward-declared in test_led_fsm.c */
bool hal_mock_led_get(void)       { return g_led_on; }
int  hal_mock_led_set_count(void) { return g_led_set_count; }
void hal_mock_reset(void)         { g_led_on = false; g_led_set_count = 0; }
