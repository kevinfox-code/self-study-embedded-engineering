#pragma once

#include <stdbool.h>
#include <stdint.h>

/*
 * Hardware Abstraction Layer interface.
 *
 * Production code is in target/hal_target.c (or equivalent board bring-up).
 * Sandbox code is in sandbox/hal_host.c (printf stubs).
 * Tests swap in test/hal_mock.c (record calls, controllable state).
 */

void     hal_led_set(bool on);
bool     hal_button_pressed(void);
uint32_t hal_tick_ms(void);
