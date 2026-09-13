# ESE-311 Week 01 — Development Environment

## Objectives

- Install and verify the bare-metal toolchain: `arm-none-eabi-gcc`, CMake,
  OpenOCD, and the ST-LINK tooling.
- Understand the STM32U575 memory map before writing any code against it.
- Establish where flash, SRAM, and the peripheral buses live, and why the
  peripheral base addresses in later weeks are what they are.

## Contents

- [`ESE-311_MemoryMap.html`](ESE-311_MemoryMap.html) — annotated STM32U575
  memory map notes (exported from a word processor; open in a browser).
  Supporting images live in `ESE-311_MemoryMap.fld/`.

## Key Concepts

- **Bus decides the base address.** GPIO sits on AHB2, RCC on AHB3, timers and
  USARTs on the APB buses. Every register macro in later weeks is
  `bus base + peripheral offset + register offset`.
- **Secure and non-secure aliases.** The U5 is a TrustZone part, so peripherals
  appear at two addresses. The projects in this course use the non-secure alias
  (the `_NS` CMSIS symbols).
- **Flash vs SRAM placement.** The linker script in
  [Week04-Linker-Scripts](../Week04-Linker-Scripts) targets these regions
  directly.

## References

- RM0456 §Memory and bus architecture
- STM32U575xx datasheet — memory map table
- See [ESE-311_References.md](../ESE-311_References.md) for download links

## AI Assistance

None.
