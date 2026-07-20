# App/Drivers/System — system policy drivers (placeholder)

Future home of system-level policy drivers: watchdog kick strategy,
power/low-power mode policy, brown-out handling, boot reason reporting.

Same rules as `Drivers/Peripheral/`: portable code only; hardware is
reached through the BSP facade, never through HAL headers or
`constants.h`.
