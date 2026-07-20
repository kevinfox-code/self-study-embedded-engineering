/**
 * @file bsp.h
 * @brief App-facing BSP facade.
 *
 * Layer: C  board_support.c in foc-lib/src/port/stm32u5 IS the BSP
 *            implementation — do not duplicate it here.
 *
 * Decoupling rule upheld: App code includes this header (or portable
 * foc/ headers), never HAL headers and never constants.h.  Only the
 * port layer and SystemInit.c may include constants.h.
 */
#ifndef APP_BSP_H
#define APP_BSP_H

#include "board_support.h"

#endif /* APP_BSP_H */
