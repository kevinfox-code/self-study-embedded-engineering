/*
 * Author:      Kevin Fox
 * Description: Compile-time feature toggles and configuration defaults.
 *
 * Use -DDEBUG_ENABLED=1 or -DDEBUG_ENABLED=0 at compile time to enable/disable.
 * Use -DFEATURE_UART_DMA=1 or -DFEATURE_UART_DMA=0 to toggle interrupt-driven UART.
 */
#ifndef CONFIG_H
#define CONFIG_H

/* Debug output enable: set to 1 for debug messages, 0 for silent operation. */
#ifndef DEBUG_ENABLED
#define DEBUG_ENABLED 0
#endif

/* UART DMA/interrupt support: set to 1 to enable non-blocking interrupt-driven UART. */
#ifndef FEATURE_UART_DMA
#define FEATURE_UART_DMA 0
#endif

#endif /* CONFIG_H */
