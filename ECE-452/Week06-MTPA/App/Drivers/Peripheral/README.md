# App/Drivers/Peripheral — application peripheral drivers (placeholder)

Future home of app-level peripheral drivers that are not part of the
motor-control path — e.g. a UART telemetry/CLI transport or a board
temperature sensor reader.

Rules: drivers here consume `App/BSP/bsp.h` and portable interfaces
only — no HAL includes, no `constants.h`. Hardware access that needs
generated handles must be added to the port-layer BSP
(`foc-lib/src/port/stm32u5/board_support.c`) behind a portable API
first.
