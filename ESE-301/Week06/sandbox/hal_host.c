/*
 * Host HAL — runs on a development machine, no real hardware.
 * The sandbox main.c drives events directly, so button/tick stubs
 * are never called during simulation; they exist to satisfy the linker.
 */

#include "hal.h"
#include <stdio.h>

static bool g_led = false;

void hal_led_set(bool on)
{
    if (on != g_led) {
        printf("[HAL] LED %s\n", on ? "ON" : "OFF");
        g_led = on;
    }
}

bool hal_button_pressed(void)
{
    return false;
}

uint32_t hal_tick_ms(void)
{
    return 0u;
}
