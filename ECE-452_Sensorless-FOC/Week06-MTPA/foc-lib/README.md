# foc-lib — Sensorless FOC Library

Portable, integer-only field-oriented control library for PMSM drives, with an
STM32U5 + DRV8323 port layer. No HAL and no RTOS dependency in the core; the
application supplies hardware through a single interface.

## Layering

| Layer | Location | Depends on |
|---|---|---|
| A/B — core control | [`src/core`](src/core), [`include/foc`](include/foc) | nothing but C99 and `stdint.h` |
| C/D — port | [`src/port/stm32u5`](src/port/stm32u5) | STM32 HAL, FreeRTOS, `App/constants.h` |

The core is integer-only by construction: [`cmake/check_no_float.cmake`](cmake/check_no_float.cmake)
fails the build if a `float` or `double` type or literal appears anywhere in
`src/core` or `include/foc`, and [`cmake/check_no_constants_h.cmake`](cmake/check_no_constants_h.cmake)
fails it if core code reaches for board constants. Both run as CTest tests.

## Modules

| Header | Responsibility |
|---|---|
| [`motor_math.h`](include/foc/motor_math.h) | Q16/Q15 fixed-point kernel, Clarke/Park, `foc_atan2`, trig LUTs |
| [`motor_filter.h`](include/foc/motor_filter.h) | First-order and biquad filters |
| [`motor_pi.h`](include/foc/motor_pi.h) | PI regulator with anti-windup |
| [`motor_foc.h`](include/foc/motor_foc.h) | Current-loop FOC step |
| [`motor_observer.h`](include/foc/motor_observer.h) | Back-EMF flux observer and PLL |
| [`motor_sm.h`](include/foc/motor_sm.h) | Drive state machine |
| [`motor_faults.h`](include/foc/motor_faults.h) | Fault latching and clearing |
| [`motor_limits.h`](include/foc/motor_limits.h) | Protection thresholds and saturation policy constants |
| [`motor_ident.h`](include/foc/motor_ident.h) | Rs / Ls / λm parameter identification |
| [`motor_modeler.h`](include/foc/motor_modeler.h) | Machine model and parameter sourcing |
| [`motor_tune.h`](include/foc/motor_tune.h) | Gain tuning |
| [`motor_ctrl.h`](include/foc/motor_ctrl.h) | Top-level controller composition |
| [`motor_types.h`](include/foc/motor_types.h) | Fixed-point typedefs, common value structs, status codes |
| [`motor_params.h`](include/foc/motor_params.h) | Runtime parameter set structs and defaults |
| [`motor_hw_if.h`](include/foc/motor_hw_if.h) | The hardware interface the port layer implements |
| [`drv8323.h`](include/foc/drv8323.h) | DRV8323 register model and transport hooks |

## Units

Parameters are stored in the units the identification routines produce, not SI:
`Ls` in mH and `λm` in mWb. Conversion to SI happens once at module-init time —
see [D-003 in `DECISIONS.md`](DECISIONS.md).

## Host Build and Tests

```bash
cd ECE-452_Sensorless-FOC/Week06-MTPA/foc-lib
cmake -B build_host -DFOC_HOST=ON
cmake --build build_host
ctest --test-dir build_host --output-on-failure
```

The host build compiles with `-Wall -Wextra -Werror -Wconversion -Wshadow -Wundef -pedantic -std=c99`.
Tests cover the fixed-point kernel, transforms, trig accuracy, filters, PI,
saturation limits, the DRV8323 register map, hardware calibration, the modeler,
and faults — plus the two static rules above.

## Target Build

```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-gcc.cmake
cmake --build build
```

Produces `libfoc.a` for the application to link. The integration is
[`../App`](../App), which composes this library with a CubeMX board profile —
see [`../App/README.md`](../App/README.md) and
[`docs/cubemx_setup_guide.md`](docs/cubemx_setup_guide.md).

## Design Record

[`DECISIONS.md`](DECISIONS.md) logs each ambiguity resolved during
implementation, with the reasoning. Read it before changing a type definition or
a units convention.

## AI Assistance

This library was implemented with Claude Code against the plan in
[`../docs/foc_library_implementation_plan.md`](../docs/foc_library_implementation_plan.md).
Decisions taken without a human in the loop are recorded in `DECISIONS.md`.
Reviewed and edited by the maintainer.
