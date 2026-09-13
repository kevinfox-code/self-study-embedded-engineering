# App/BSP — board support facade

`bsp.h` is a thin facade over the real BSP implementation,
[`foc-lib/src/port/stm32u5/board_support.c`](../../foc-lib/src/port/stm32u5/board_support.c).
**Do not duplicate BSP logic here** — new board operations belong in the
port layer, exposed through `board_support.h`'s portable API.

## Include discipline (who may include what)

| Code | May include |
|---|---|
| foc-lib core (`src/core`, `include/foc`) | portable foc headers only (CI-enforced: no `constants.h`, no floats) |
| foc-lib port layer (`src/port/stm32u5/*.c`) + `App/SystemInit.c` | `constants.h` (→ generated `main.h`), HAL, RTOS |
| Everything else under `App/` | `App/BSP/bsp.h`, `foc/*.h`, other App headers — **never** HAL headers, **never** `constants.h` |
| CubeMX profile (root-level `Profiles/…`) | generated code only; calls `CoreAppMain()` + ISR hooks inside USER CODE markers |
