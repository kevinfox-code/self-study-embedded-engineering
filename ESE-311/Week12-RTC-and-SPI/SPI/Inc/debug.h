/*
 * Author:      Kevin Fox
 * Description: Debug module — declares a fatal error handler that blinks all
 *              LEDs rapidly to signal an unrecoverable fault condition.
 *
 * Provides assert-style macros for development (only active if DEBUG_ENABLED=1).
 * Use UART_ASSERT() to verify preconditions; assert failures trigger error handler.
 */
#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>
#include "config.h"

/* Blinks all three LEDs together at ~10 Hz forever. Never returns. */
void debug_error_handler(void) __attribute__((noreturn));

#if DEBUG_ENABLED

/* Assert that condition is true. If false, calls debug_error_handler(). */
#define UART_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            debug_error_handler(); \
        } \
    } while (0)

#else

/* Assertions disabled in release builds (no runtime overhead). */
#define UART_ASSERT(condition) do {} while (0)

#endif

#endif /* DEBUG_H */
