# ESE-311 Week 13 — I2C

> **Status: scaffold.** This week's project tree is a copy of the Week 12 SPI
> project, taken as the starting point for the I2C work. No I2C driver has been
> written yet — [`I2C/Src/main.c`](I2C/Src/main.c) still drives the ADXL345 over
> SPI. The build is green so the tree can be developed incrementally.

## Objectives

- Configure I2C1 at the register level on the STM32U575.
- Implement 7-bit addressed master write and read using `CR2` transfer control
  (`SADD`, `NBYTES`, `RD_WRN`, `START`, `AUTOEND`).
- Re-target the ADXL345 driver from SPI to I2C so the same application code
  reads the same sensor over a different bus.

## Planned Work

| Item | State |
|---|---|
| `Src/i2c.c` / `Inc/i2c.h` — I2C1 master driver | not started |
| ADXL345 transport switch (SPI → I2C) | not started |
| Bus timing (`TIMINGR`) for 100 kHz and 400 kHz | not started |
| Application read loop over I2C | not started |

## Carried Forward From Week 12

The GPIO, SysTick, UART, ring-buffer, debug, ADC, TIM, SPI, and ADXL345 modules
are unchanged from [Week12-SPI](../Week12-SPI). Only the CMake project name
(`i2c`) and the VS Code task labels differ.

## Key Concepts (to cover)

- **`TIMINGR` is computed, not guessed.** The prescaler and the four timing
  fields come from the kernel clock and the target bus speed; ST's CubeMX timing
  tool or the RM0456 tables are the practical source.
- **`AUTOEND` vs software STOP.** Repeated-start register reads need `AUTOEND`
  off so a `RESTART` can be issued between the address write and the data read.
- **Open-drain and pull-ups.** Both SCL and SDA must be configured open-drain;
  the bus does not work without pull-ups, on-board or external.
- **`NACK` handling.** A missing or mis-addressed device sets `NACKF`, which has
  to be cleared before the peripheral will accept another transfer.

## Build and Run

```bash
cd ESE-311_Bare-Metal-C/Week13-I2C/I2C
cmake -B build
cmake --build build
cmake --build build --target flash
```

## VS Code Debug

Requires the Cortex-Debug extension, with `arm-none-eabi-gdb` and `openocd` on
`PATH`. Open the `I2C/` folder as the workspace and launch **Debug (OpenOCD)**,
which builds `build/i2c.elf` via the `build-i2c` task.

## References

- Israel Gbati, *Bare-Metal Embedded C Programming* — I2C chapter
- RM0456 §I2C; ADXL345 datasheet for the I2C address and register map
- See [ESE-311_References.md](../ESE-311_References.md) for download links

## AI Assistance

None.
