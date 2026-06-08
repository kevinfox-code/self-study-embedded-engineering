# UART Module

## Objectives

- Implement UART driver for STM32U575
- Character-based serial communication
- Blocking send/receive operations

## What Was Built

- UART initialization and clock configuration
- Character transmit/receive functions
- String transmission helper

## Key Concepts

- USART peripheral configuration
- Baud rate and clock calculations
- TX/RX buffer handling

## Build

```bash
cd ESE-311/Week10-UART-Driver/UART
rm -rf build
cmake -B build
cmake --build build
```

## Build and Flash

```bash
cmake --build build --target flash
```

## VS Code Debug

1. Install the Cortex-Debug extension.
2. Make sure `arm-none-eabi-gdb` and `openocd` are on your `PATH`.
3. Connect the board with ST-LINK.
4. Open the Run and Debug view and select `Debug UART (OpenOCD)`.

The debug launch uses `build/uart.elf` and the same OpenOCD setup as the `flash` target.

## References

- Bare-Metal Embedded C Programming, Ch. 10
- STM32U575 Reference Manual (USART section)
