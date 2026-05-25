/*
 * Author:      Kevin Fox
 * Description: Debug module — declares a fatal error handler that blinks all
 *              LEDs rapidly to signal an unrecoverable fault condition.
 */
#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>

/* Blinks all three LEDs together at ~10 Hz forever.  Never returns. */
void debug_error_handler(void) __attribute__((noreturn));

#endif /* DEBUG_H */
