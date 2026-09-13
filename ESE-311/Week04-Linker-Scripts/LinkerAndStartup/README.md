# LinkerAndStartup — Linker Script and Startup Code (STM32U575)

Minimal project that builds without any vendor framework: a hand-written startup
file and linker script, plus a `main.c` that never returns. Week context:
[../README.md](../README.md).

- [`STM32U575ZITXQ_FLASH.ld`](STM32U575ZITXQ_FLASH.ld) — memory regions, section
  placement, and the symbols the startup code copies `.data` and zeroes `.bss` with.
- [`startup_stm32u575zitxq.s`](startup_stm32u575zitxq.s) — vector table, reset
  handler, and the branch to `main`.

## Build

```bash
cmake -B build
cmake --build build
```

## Flash

```bash
cmake --build build --target flash
```
