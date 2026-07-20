# App/ — modular FOC application layer

Board-agnostic application tree for the sensorless FOC motor controller
(STM32U575 Nucleo + DRV8323), scaffolded per
[`.github/prompts/scaffold-cubemx-app.prompt.md`](../.github/prompts/scaffold-cubemx-app.prompt.md).
Hand-written application code is fully decoupled from CubeMX-generated
code through a single bridge file.

## Decoupling contract

1. **Generated code is untouchable.** CubeMX output lives only under
   the workspace-root `Profiles/<Board>/` (a sibling of `App/` and
   `foc-lib/`, not nested inside `App/` — one profile tree can back
   multiple app variants); never hand-edit it outside `USER CODE`
   markers, never `#include` it from App code.
2. **`constants.h` is the only bridge.** It includes the generated
   `main.h` once (gated on `APP_HAVE_CUBEMX_PROFILE`) and maps abstract
   `BSP_*` names onto concrete handles, guarded per board variant with
   an `#error` fallback.
3. **App code depends on abstractions.** Everything else under `App/`
   references only `BSP/bsp.h`, `foc/*.h` and App headers.
4. **`SystemInit.c` is the glue layer.** Init *policy* lives here;
   `SystemClock_Config()`/`MX_*_Init()` stay in the generated `main.c`.

## Layer map

```
CubeMX profile (../Profiles/NucleoU575)   generated, Layer: hardware init
        │  calls CoreAppMain() + ISR hooks (USER CODE only)
        ▼
App/Entry/CoreAppMain.c                   portable entry
App/SystemInit.c                          glue (only App file incl. constants.h)
App/Tasks, App/RTOS, App/Drivers          portable app code
        │
        ▼
foc-lib port layer (src/port/stm32u5)     Layer C/D: BSP, ISRs, RTOS tasks
        │  includes App/constants.h  ◄─── THE bridge
        ▼
foc-lib core (src/core, include/foc)      Layers A/B: portable, integer-only
```

## Tree

```
App/
  README.md                  this file
  constants.h                bridge: BSP_* → CubeMX handles (moved from foc-lib port dir)
  SystemInit.h/.c            System_* init-policy API
  CMakeLists.txt             standalone build (gated on APP_HAVE_CUBEMX_PROFILE)
  BSP/bsp.h                  facade over foc-lib board_support.h
  Drivers/Peripheral/        placeholder: app peripheral drivers
  Drivers/System/            placeholder: watchdog/power policy drivers
  Entry/CoreAppMain.h/.c     entry point called from generated main.c
  Tasks/task_config.h        priorities, stacks, NVIC map
  Tasks/app_tasks.h/.c       app-level task creation (telemetry stub)
  RTOS/rtos_hooks.c          stack-overflow/malloc-failed safe-stop hooks
```

`Profiles/NucleoU575/` lives at the workspace root (sibling of `App/`
and `foc-lib/`) — reserved for CubeMX output, see its own README.

## Peripheral mapping (mirrors `constants.h`, BOARD_NUCLEO_U575)

| Abstract name | Concrete mapping | Status |
|---|---|---|
| `BSP_TIM_PWM_HANDLE` | `&htim1` — TIM1 center-aligned 20 kHz, CH1/1N–CH3/3N PE9/8, PE11/10, PE13/12, BKIN PE15, ARR=4000 | gated on profile |
| `BSP_TIM_CHANNEL_A/B/C` | `TIM_CHANNEL_1/2/3` | active |
| `BSP_ADC_HANDLE` | `&hadc1` — injected IN1–IN4 PC0–PC3, TIM1 TRGO | gated on profile |
| `BSP_ADC_RANK_IA/IB/IC/VBUS` | `ADC_INJECTED_RANK_1..4` | active |
| `BSP_SPI_DRV_HANDLE` | `&hspi1` — mode 1, 16-bit, ≤10 MHz | gated on profile |
| `BSP_GPIO_DRV_CS_PORT/PIN` | `GPIOA` / `GPIO_PIN_4` (nSCS, guide §5) | gated on profile |
| `BSP_GPIO_DRV_EN_PORT/PIN` | `GPIOA` / `GPIO_PIN_8` | **TODO — pin not final** |
| `BSP_GPIO_NFAULT_PORT/PIN` | `GPIOB` / `GPIO_PIN_0` | **TODO — pin not final** |
| `BSP_NFAULT_EXTI_IRQn` | `EXTI0_IRQn` | **TODO — follows nFAULT pin** |

## Build compose step

The App build is standalone (the root `CMakeLists.txt` is untouched):

1. Generate the CubeMX project into the root-level `Profiles/NucleoU575/`
   per [`foc-lib/docs/cubemx_setup_guide.md`](../foc-lib/docs/cubemx_setup_guide.md) §1–9.
2. Resolve the `TODO` macros in `constants.h`; add profile source/include
   dirs and the linker script at the `TODO` markers in `App/CMakeLists.txt`.
3. Make the three USER-CODE insertions (see [`../Profiles/NucleoU575/README.md`](../Profiles/NucleoU575/README.md)).
4. `cmake -S App -B App/build -DAPP_HAVE_CUBEMX_PROFILE=ON && cmake --build App/build`
   — this injects `-DBOARD_NUCLEO_U575 -DAPP_HAVE_CUBEMX_PROFILE` and links `libfoc.a`.

Until then, `cmake -S App -B App/build && cmake --build App/build` builds
the host stub check (`app_stub_check`) + `libfoc.a`.

## TODOs / open items

- `BSP_GPIO_DRV_EN_*`, `BSP_GPIO_NFAULT_*`, `BSP_NFAULT_EXTI_IRQn`: pins
  unassigned until the CubeMX pinout is fixed.
- Optional debug/WCET scope pin (guide §6) has no macro yet.
- `drv8323_transport.c` HAL body is still commented; confirm it consumes
  `BSP_GPIO_DRV_CS_PORT/PIN` when uncommented.
- Telemetry task body in `Tasks/app_tasks.c` is a stub.

## Notes

- `Q16()` used in `constants.h` is defined in `foc/motor_math.h`; any
  `.c` file that *evaluates* those macros must include a foc header
  that defines `Q16` before use (macros expand lazily, so the bridge
  itself compiles fine without it).
- Pre-existing, unrelated: the root `CMakeLists.txt` lists `Src/i2c.c`,
  which does not exist on disk (root build breaks at link/compile);
  `foc-lib/examples/stm32u5_nucleo/` is empty and could later point to
  this App as the reference integration.
