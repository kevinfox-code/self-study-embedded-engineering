# ESE-311 Week 04 — Linker Scripts and Startup Code

## Objectives

- Write a linker script that describes the STM32U575 flash and SRAM regions.
- Understand how `.text`, `.rodata`, `.data`, and `.bss` are placed, and which
  of them need runtime work.
- Write the reset handler that copies `.data` from flash to SRAM, zeroes `.bss`,
  sets the stack pointer, and branches to `main`.

## What Was Built

- [`LinkerAndStartup`](LinkerAndStartup) — a project with no vendor framework at
  all: a hand-written [`STM32U575ZITXQ_FLASH.ld`](LinkerAndStartup/STM32U575ZITXQ_FLASH.ld)
  and [`startup_stm32u575zitxq.s`](LinkerAndStartup/startup_stm32u575zitxq.s),
  linked against a `main.c` that never returns.

## Key Concepts

- **`.data` has two addresses.** Its load address (LMA) is in flash and its
  virtual address (VMA) is in SRAM; the reset handler is what bridges them.
- **`.bss` costs nothing in the image.** It is described by symbols only, and
  zeroed at startup — which is why an uninitialised global reads as 0.
- **The vector table comes first.** The first word is the initial stack pointer,
  the second is the reset handler address. Getting the order wrong faults before
  a single instruction of C runs.
- **Linker symbols are addresses, not values.** `_sdata` and friends are used by
  taking their address, never by reading them.

## Build and Run

```bash
cd ESE-311_Bare-Metal-C/Week04-Linker-Scripts/LinkerAndStartup
cmake -B build
cmake --build build
cmake --build build --target flash
```

## References

- Israel Gbati, *Bare-Metal Embedded C Programming* — linker and startup chapter
- Arm Cortex-M33 Devices Generic User Guide — vector table and reset behaviour
- See [ESE-311_References.md](../ESE-311_References.md) for download links

## AI Assistance

None.
