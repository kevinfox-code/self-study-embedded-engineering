# UART — USART1 Driver (STM32U575)

Blocking USART1 driver on PA9/PA10 (AF7), routed to the ST-LINK V3E virtual COM
port at 115200 8N1. Week context and concept notes: [../README.md](../README.md).

## Build

```bash
cmake -B build
cmake --build build
```

## Flash

```bash
cmake --build build --target flash
```

## VS Code Debug

1. Install the Cortex-Debug extension.
2. Put `arm-none-eabi-gdb` and `openocd` on your `PATH`.
3. Connect the board over ST-LINK.
4. Open this folder as the workspace and run **Debug UART (OpenOCD)**.

The launch configuration uses `build/uart.elf` and the same OpenOCD setup as the
`flash` target.
