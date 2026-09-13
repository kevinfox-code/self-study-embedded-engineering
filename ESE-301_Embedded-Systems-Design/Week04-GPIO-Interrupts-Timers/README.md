# ESE-301 Week 04 — GPIO, Interrupts, and Timers

## Objectives

- Drive GPIO through the STM32 HAL rather than at the register level.
- Handle a button press as an EXTI interrupt instead of polling it.
- Use a hardware timer to pace periodic work, and see what belongs in an ISR.

## What Was Built

- [`ESE-301_hw4`](ESE-301_hw4) — a CubeMX-generated STM32U575 project with the
  application split into focused modules alongside the generated code:
  - [`Core/Src/led.c`](ESE-301_hw4/Core/Src/led.c) — LED abstraction
  - [`Core/Src/button.c`](ESE-301_hw4/Core/Src/button.c) — button handling
  - [`Core/Src/tim.c`](ESE-301_hw4/Core/Src/tim.c) — timer setup
  - [`Core/Src/main.c`](ESE-301_hw4/Core/Src/main.c) — initialises the HAL and
    peripherals, then blinks LEDs and responds to button presses via interrupt
    callbacks.

## Key Concepts

- **Keep the ISR short.** The interrupt sets state; the main loop acts on it.
- **HAL callbacks are weak symbols.** Overriding `HAL_GPIO_EXTI_Callback` in
  application code is how the generated layer hands control back to you.
- **Generated code has edit regions.** Everything outside the `USER CODE`
  markers is overwritten the next time CubeMX regenerates the project.
- **Debounce belongs in software.** A mechanical button produces several edges
  per press; the timer tick is a convenient place to filter them.

## Build and Run

Open `ESE-301_hw4` in STM32CubeIDE, or build the CMake project directly and flash
with the **STM32Cube: Launch ST-Link GDB Server** VS Code configuration.

## References

- Elecia White, *Making Embedded Systems* — interrupts and timers chapters
- RM0456 §EXTI and §TIM; UM2883 for the HAL APIs used
- See [ESE-301_References.md](../ESE-301_References.md) for download links

## AI Assistance

None.
