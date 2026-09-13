# ESE-311 Week 08 — SysTick Timer

## Objectives

- Understand the Arm Cortex-M SysTick timer as a core resource, not an STM32
  peripheral.
- Configure `SYST_RVR` / `SYST_CSR` and poll `COUNTFLAG` to build a calibrated
  blocking delay.
- Replace the software busy-wait loops used in Week 7 with real timing.

## What Was Built

- [`Systick/Src/systick.c`](Systick/Src/systick.c) — `systick_init` and
  `systick_msec_delay`, driving SysTick from the internal 16 MHz processor clock
  and polling `COUNTFLAG` for millisecond ticks. No HAL, no interrupt.
- [`Systick/Src/main.c`](Systick/Src/main.c) — times LED toggling and button
  polling against the new delay.
- [`Systick/Src/debug.c`](Systick/Src/debug.c) — fatal error handler that blinks
  all LEDs, wired to `Error_Handler`.
- GPIO driver carried forward from Week 7.

## Key Concepts

- **Core peripheral, not vendor peripheral.** SysTick lives in the Cortex-M
  system address space, so the same code works on any Cortex-M part.
- **Reload value.** `SYST_RVR` holds *ticks − 1*; the counter is 24-bit, which
  caps a single reload at ~1 s at 16 MHz.
- **`COUNTFLAG` is read-to-clear.** Reading `SYST_CSR` clears it, so a stray
  debug read of the register inside the wait loop silently eats a tick.
- **Clock source select.** `SYST_CSR.CLKSOURCE` picks the processor clock or the
  implementation-defined reference clock; the delay calibration depends on it.

## Build and Run

```bash
cd ESE-311_Bare-Metal-C/Week08-SysTick/Systick
cmake -B build
cmake --build build
cmake --build build --target flash
```

## VS Code Debug

Requires the Cortex-Debug extension, with `arm-none-eabi-gdb` and `openocd` on
`PATH`. Open the `Systick/` folder as the workspace and launch
**Debug Systick (OpenOCD)**, which builds `build/systick.elf` via the
`cmake: build` task.

## References

- Israel Gbati, *Bare-Metal Embedded C Programming* — SysTick chapter
- Arm Cortex-M33 Devices Generic User Guide (Arm 100235) — SysTick registers
- See [ESE-311_References.md](../ESE-311_References.md) for download links

## AI Assistance

None.
