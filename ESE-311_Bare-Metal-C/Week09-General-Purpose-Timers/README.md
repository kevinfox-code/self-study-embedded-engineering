# ESE-311 Week 09 — General-Purpose Timers

## Objectives

- Move timing off the core (SysTick) and onto a general-purpose peripheral timer.
- Understand the prescaler / auto-reload relationship that sets the update rate.
- Poll the update event flag, and understand what would change to take it as an
  interrupt instead.

## What Was Built

- [`GTIM/Src/tim.c`](GTIM/Src/tim.c) — TIM2 set up as a periodic timer off the
  APB1 clock with `PSC = 4000 − 1` and `ARR = 1000 − 1`, using a forced update
  (`EGR.UG`) to load the shadow registers before the counter is enabled.
- [`GTIM/Inc/tim.h`](GTIM/Inc/tim.h) — `tim2_1hz_init`,
  `tim2_update_event_ready`, `tim2_clear_update_event`.
- [`GTIM/Src/main.c`](GTIM/Src/main.c) — paces LED toggling from the hardware
  update event rather than a software delay.
- SysTick, GPIO, and debug modules carried forward from Week 8.

## Key Concepts

- **Update rate.**

  ```
  f_update = f_timer_clock / ((PSC + 1) * (ARR + 1))
  ```

  Both registers are written as *value − 1*, which is the usual source of an
  off-by-one in the resulting period.

- **Shadow registers.** `PSC` and `ARR` are buffered. Setting `EGR.UG` forces an
  update event so the new values take effect immediately instead of at the end
  of the current period.
- **`UIF` is not self-clearing.** The poll loop has to clear `SR.UIF` explicitly,
  or the event appears to fire continuously.
- **APB timer clock doubling.** When the APB prescaler is greater than 1, the
  timer clock is twice the APB clock. That factor has to be accounted for when
  computing `PSC`.

## Build and Run

```bash
cd ESE-311_Bare-Metal-C/Week09-General-Purpose-Timers/GTIM
cmake -B build
cmake --build build
cmake --build build --target flash
```

## VS Code Debug

Requires the Cortex-Debug extension, with `arm-none-eabi-gdb` and `openocd` on
`PATH`. Open the `GTIM/` folder as the workspace and launch
**Debug GTIM (OpenOCD)**, which builds `build/gtim.elf` via the `cmake: build` task.

## References

- Israel Gbati, *Bare-Metal Embedded C Programming* — general-purpose timers chapter
- RM0456 §TIM2/TIM3/TIM4/TIM5 general-purpose timers
- See [ESE-311_References.md](../ESE-311_References.md) for download links

## AI Assistance

None.
