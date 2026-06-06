---
applyTo: "**/*.c,**/*.h"
---

# C Firmware Rules

These rules apply to all C source and header files in the project.

## HAL Usage Patterns

- Initialize peripheral handle structs (e.g., `UART_HandleTypeDef`) as `static` globals in the owning module — one module owns one peripheral instance.
- Call all `HAL_*_Init()` functions during system startup (`SystemInit` / `MX_*_Init`), never dynamically at runtime unless reinit is architecturally justified and documented.
- Use HAL callbacks (`HAL_*_CpltCallback`, `HAL_*_ErrorCallback`) for asynchronous operations. Prefer `HAL_*_RegisterCallback()` over weak-symbol overrides when the HAL version supports it (≥ HAL 1.16).
- For DMA transfers: verify buffer alignment (4-byte for word transfers), cache coherency on F7/H7 (`SCB_CleanDCache_by_Addr` / `SCB_InvalidateDCache_by_Addr`), and correct DMA stream/channel mapping.
- When configuring clocks, always document the full clock tree inline:
  ```c
  // Clock tree: HSE 8MHz → PLL (M=8, N=336, P=2) → SYSCLK 168MHz
  // AHB=168MHz, APB1=42MHz, APB2=84MHz
  ```

## Interrupt Safety

- ISR functions use the `void <Peripheral>_IRQHandler(void)` naming convention matching the vector table.
- ISRs must complete within their documented timing budget. Add a comment at the top of each ISR with the target maximum duration.
- Only call reentrant, ISR-safe functions from within ISR context.
- All variables shared between ISR and main/task context must be declared `volatile` and accessed within a critical section or via atomic operations.
- Never use `HAL_Delay()`, `printf()`, `sprintf()`, floating-point math, or any function that calls `malloc` from ISR context.
- Keep ISRs short: acknowledge the interrupt source, set a flag or write to a queue, and return. Do heavy processing in the main loop or an RTOS task.

## Register Access

- Always use CMSIS-defined peripheral structures and bit masks:
  ```c
  SET_BIT(TIM2->CR1, TIM_CR1_CEN);        // Preferred
  TIM2->CR1 |= TIM_CR1_CEN;               // Acceptable
  *((volatile uint32_t*)0x40000000) = 0x1; // PROHIBITED
  ```
- Comment non-obvious bit manipulations with a reference to the Reference Manual:
  ```c
  // RM0090 §18.3.1: Enable counter
  SET_BIT(TIM2->CR1, TIM_CR1_CEN);
  ```
- Use `__IO` (volatile) qualifiers for memory-mapped registers — never cast away volatile.

## Memory & Stack

- All buffers must be statically allocated with explicit, named sizes:
  ```c
  #define UART_RX_BUFFER_SIZE 256U
  static uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE];
  ```
- No variable-length arrays (VLAs). Array sizes must be compile-time constants.
- Use `__attribute__((section(".noinit")))` for data that must persist across software resets.
- Avoid large local arrays in functions — use static or module-scope buffers to keep stack usage predictable.

## Error Handling

- Every HAL function call must check the return value:
  ```c
  if (HAL_UART_Transmit(&huart2, data, len, UART_TIMEOUT_MS) != HAL_OK)
  {
      error_handler(ERROR_UART_TX_FAILED);
  }
  ```
- Implement a centralized `error_handler()` or equivalent that logs the error source, optionally triggers a watchdog reset, and never silently ignores failures.
- Use `assert_param()` (STM32 HAL built-in) during development for parameter validation.
- Implement IWDG (Independent Watchdog) as a last-resort recovery for hung states.

## Header File Structure

- Every `.h` file must have include guards:
  ```c
  #ifndef MODULE_NAME_H
  #define MODULE_NAME_H

  #ifdef __cplusplus
  extern "C" {
  #endif

  /* Public API declarations */

  #ifdef __cplusplus
  }
  #endif

  #endif /* MODULE_NAME_H */
  ```
- Keep headers minimal: only public types, function declarations, and constants. Implementation details stay in `.c` files.
- Use forward declarations where possible to reduce header coupling.

## MISRA-C Key Rules

- **Rule 8.7**: Functions and variables used only in one translation unit must be `static`.
- **Rule 10.3**: Explicit casts required for all narrowing or sign-changing type conversions.
- **Rule 14.4**: All `if`/`else if` chains must terminate with a final `else` clause (even if it only contains a comment or assertion).
- **Rule 15.7**: All `switch` statements must have a `default` case.
- **Rule 17.7**: Return values of non-void functions must not be discarded.
- **Rule 21.3**: `<stdlib.h>` memory allocation functions (`malloc`, `calloc`, `realloc`, `free`) are prohibited.
