/*
 * SPDX-License-Identifier: MIT
 * Author:      Kevin Fox
 * Book:        Bare-Metal Embedded C Programming
 *              by Israel Gbati — Packt, 2024
 * Description: Debug module — declares the fatal error handler and the assert-style macros that are
 *              active only when DEBUG_ENABLED is set.
 */
#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>

/* Blinks all three LEDs together at ~10 Hz forever.  Never returns. */
void debug_error_handler(void) __attribute__((noreturn));

#endif /* DEBUG_H */
