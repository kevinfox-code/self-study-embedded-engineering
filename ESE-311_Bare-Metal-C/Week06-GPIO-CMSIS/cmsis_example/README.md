# cmsis_example — GPIO via CMSIS Device Headers (STM32U575)

Same LED toggle as the earlier weeks, but reaching the peripherals through the
CMSIS device header (`GPIOA->MODER`) instead of hand-rolled address macros.
Week context: [../README.md](../README.md).

## Build

```bash
cmake -B build
cmake --build build
```

## Flash

```bash
cmake --build build --target flash
```
