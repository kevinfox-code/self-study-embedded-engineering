# ESE-301 Week 01 — Introduction

## Objectives

- Establish a working STM32CubeIDE/CubeMX toolchain and a VS Code debug flow
  against the NUCLEO-U575ZI-Q.
- Produce a first HAL-based project and confirm the flash-and-debug loop end to end.

## What Was Built

- [`blinky`](blinky) — a CubeMX-generated STM32U575 project that toggles the
  on-board LED. Serves as the reference project layout for the rest of the course.

## Setup

Environment variables required by the VS Code debug configuration (macOS,
Windows, and Linux) are documented in
[`blinky/ESE-301_Environment-Setup.md`](blinky/ESE-301_Environment-Setup.md).

## Build and Run

```bash
cd ESE-301_Embedded-Systems-Design/Week01-Introduction/blinky
cmake --preset Debug
cmake --build build/Debug
```

Debug with the **STM32Cube: Launch ST-Link GDB Server** VS Code configuration.

## References

- Elecia White, *Making Embedded Systems* — Ch. 1
- UM2861 for the Nucleo-144 board layout
- See [ESE-301_References.md](../ESE-301_References.md) for download links

## AI Assistance

None.
