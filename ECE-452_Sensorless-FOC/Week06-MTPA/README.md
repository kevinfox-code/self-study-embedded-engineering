# ECE-452 Week 06 — MTPA and the FOC Firmware Stack

**Reading:** Krishnan — maximum torque per ampere and the salient-machine torque
equation.

**Lab deliverable:** the sensorless FOC library and the board-agnostic
application layer that will carry the rest of the course.

## Layout

| Path | What it is |
|---|---|
| [`foc-lib/`](foc-lib) | The FOC library: integer-only core plus an STM32U5 port layer. Has its own [README](foc-lib/README.md) and [DECISIONS.md](foc-lib/DECISIONS.md). |
| [`App/`](App) | Board-agnostic application layer that composes `foc-lib` with a CubeMX profile through a single bridge header. See [App/README.md](App/README.md). |
| [`Profiles/NucleoU575/`](Profiles/NucleoU575) | Reserved for CubeMX-generated output. See its [README](Profiles/NucleoU575/README.md). |
| [`docs/`](docs) | [Implementation plan](docs/foc_library_implementation_plan.md) the library was built against. |
| [`Src/`, `Inc/`](Src) | **Legacy.** The ESE-311 bare-metal starter this week was branched from — an ADXL345-over-SPI demo, unrelated to motor control. Builds as `mtpa_foc.elf`, but the real work is in `App/` and `foc-lib/`. |

## Key Concepts

- **MTPA is only interesting when Ld ≠ Lq.** For a surface-mount PMSM the
  reluctance term vanishes and MTPA degenerates to `id = 0`. The salient case is
  where a negative `id` buys torque.
- **The current-limit circle bounds the trajectory.** The MTPA locus is followed
  only until it meets the current limit; past that the operating point tracks the
  limit circle instead.
- **Integer-only core.** Keeping the control path in Q16/Q15 removes the FPU from
  the ISR timing budget and makes worst-case execution time predictable. The
  library enforces this with a build-time check rather than by convention.
- **One bridge header.** `App/constants.h` is the only file that includes CubeMX
  output, so the application and the library stay portable across board profiles.

## Build and Run

Library host tests:

```bash
cd ECE-452_Sensorless-FOC/Week06-MTPA/foc-lib
cmake -B build_host -DFOC_HOST=ON
cmake --build build_host
ctest --test-dir build_host --output-on-failure
```

Application layer (host stub until a CubeMX profile is generated):

```bash
cd ECE-452_Sensorless-FOC/Week06-MTPA
cmake -S App -B App/build
cmake --build App/build
```

See [App/README.md](App/README.md) for the full compose step once
`Profiles/NucleoU575/` has been generated.

## Open Items

- Pin macros `BSP_GPIO_DRV_EN_*`, `BSP_GPIO_NFAULT_*`, and `BSP_NFAULT_EXTI_IRQn`
  in `App/constants.h` are unresolved until the CubeMX pinout is fixed.
- The telemetry task body in `App/Tasks/app_tasks.c` is a stub.
- `Src/` and `Inc/` are legacy starter code and should be removed once nothing
  depends on them.

## References

- Krishnan, *Permanent Magnet Synchronous and Brushless DC Motor Drives* — MTPA
  and salient-machine torque
- DRV8323RS datasheet; RM0456 §TIM1 and §ADC for the PWM/ADC synchronisation
- See [ECE-452_References.md](../ECE-452_References.md) for download links

## AI Assistance

The FOC library and the application layer were implemented with Claude Code
against [`docs/foc_library_implementation_plan.md`](docs/foc_library_implementation_plan.md).
Decisions taken autonomously are logged in [`foc-lib/DECISIONS.md`](foc-lib/DECISIONS.md).
Reviewed and edited by the maintainer.
