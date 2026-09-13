# gpio_example — GPIO Input/Output (STM32U575)

Bare-metal GPIO driver demo for the NUCLEO-U575ZI-Q. Polls the user button,
mirrors it to the blue LED, and toggles the green LED each alternate loop pass.
Week context and concept notes: [../../README.md](../../README.md).

## Build

```bash
cmake -B build
cmake --build build
```

## Flash

```bash
cmake --build build --target flash
```

Requires `arm-none-eabi-gcc` and `openocd` on `PATH`.
