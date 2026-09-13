/*
 * SPDX-License-Identifier: MIT
 * Author:      Kevin Fox
 * Description: Debug module — declares the fatal error handler and the assert-style macros that are
 *              active only when DEBUG_ENABLED is set.
 */
#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>
#include "config.h"

/* Blinks all three LEDs together at ~10 Hz forever. Never returns. */
void debug_error_handler(void) __attribute__((noreturn));

#if DEBUG_ENABLED

/* Assert that condition is true. If false, calls debug_error_handler(). */
#define UART_ASSERT(condition)                                                                     \
    do                                                                                             \
    {                                                                                              \
        if (!(condition))                                                                          \
        {                                                                                          \
            debug_error_handler();                                                                 \
        }                                                                                          \
    } while (0)

#else

/* Assertions disabled in release builds (no runtime overhead). */
#define UART_ASSERT(condition)                                                                     \
    do                                                                                             \
    {                                                                                              \
    } while (0)

#endif

#endif /* DEBUG_H */
