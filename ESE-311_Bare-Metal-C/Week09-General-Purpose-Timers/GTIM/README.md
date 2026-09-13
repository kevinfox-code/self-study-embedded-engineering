# GTIM — General-Purpose Timer (STM32U575)

Paces the LED from a TIM2 hardware update event instead of a software delay.
Week context and concept notes: [../README.md](../README.md).

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
4. Open this folder as the workspace and run **Debug GTIM (OpenOCD)**.

The launch configuration uses `build/gtim.elf` and the same OpenOCD setup as the
`flash` target.
