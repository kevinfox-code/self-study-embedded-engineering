# ESE-311 Week 07 — GPIO Driver

## Objectives

- Configure STM32U5 GPIO at the register level: mode, output type, speed, and
  pull-up/pull-down.
- Read the user button as an input and drive the on-board RGB LEDs as outputs.
- Package the register work behind a small driver API instead of poking
  registers from `main`.

## What Was Built

- [`GpioInput-Output/gpio_example`](GpioInput-Output/gpio_example) — a minimal
  CMake STM32U575 project with a hand-written GPIO driver
  ([`Src/gpio.c`](GpioInput-Output/gpio_example/Src/gpio.c),
  [`Inc/gpio.h`](GpioInput-Output/gpio_example/Inc/gpio.h)) exposing `led_init`,
  `button_init`, per-LED on/off/toggle helpers, and `get_button_state`.
- [`Src/main.c`](GpioInput-Output/gpio_example/Src/main.c) — polls the button,
  mirrors its state to the blue LED, and toggles the green LED every other loop
  iteration.

## Key Concepts

- **`MODER` is two bits per pin.** Clear both bits before writing the new mode,
  or a pin that was in an alternate-function state keeps a stale bit set.
- **Pull resistors decide the idle level.** An input with no pull and nothing
  driving it floats, and the reading is noise.
- **`BSRR` over read-modify-write.** Writing `BSRR` sets or clears a pin
  atomically; `ODR |= ...` is a three-step sequence that an interrupt can
  interleave with.
- **Clock before configuration.** A GPIO port ignores every register write until
  its `RCC_AHB2ENR1` bit is set, which makes the failure look like dead code.

## Build and Run

```bash
cd ESE-311/Week07-GPIO-Driver/GpioInput-Output/gpio_example
cmake -B build
cmake --build build
cmake --build build --target flash
```

Flashing uses the OpenOCD-based `flash` target; STM32CubeProgrammer works too.

## References

- Israel Gbati, *Bare-Metal Embedded C Programming* — GPIO chapter
- RM0456 §GPIO; UM2861 for the Nucleo LED and user-button pin assignments
- See [ESE-311_References.md](../ESE-311_References.md) for download links

## AI Assistance

GitHub Copilot assisted with drafting this README. Reviewed and edited by the
maintainer.
