# ESE-311 Week 13 — I2C

## Objectives

- Configure STM32U575 I2C1 directly through CMSIS registers.
- Write to a 7-bit addressed sensor and burst-read registers using repeated START.
- Read the Week 12 ADXL345 over I2C and report acceleration through UART.

## What Was Built

- [I2C1 driver](I2C/Src/i2c.c): blocking writes and register reads, with bounded
  polling and explicit NACK, timeout, bus-error, arbitration-loss and overrun status.
- [ADXL345 driver](I2C/Src/adxl345.c): verifies device ID `0xE5`, selects full
  resolution at ±4 g and 100 Hz, then enables measurement mode.
- [Application](I2C/Src/main.c): prints raw bytes and signed XYZ counts every
  500 ms. Debugger globals also expose acceleration in g, using 0.0039 g/LSB.
  Initialization retries on sensor errors; failed samples are not printed.
- [Host tests](I2C/tests/test_adxl345.c): configuration order, identity mismatch,
  transport failures, and preservation of the previous sample on a failed read.

The ADXL345 is an accelerometer; this hardware has no gyroscope. The SPI files
remain as reference material but are no longer part of the firmware build.

## Wiring

Power the sensor with 3.3 V and connect its ground to the NUCLEO-U575ZI-Q ground.

| STM32 / supply | ADXL345 |
|---|---|
| PB8, AF4 (I2C1 SCL) | SCL |
| PB9, AF4 (I2C1 SDA) | SDA |
| 3.3 V | CS (selects I2C mode) |
| GND | SDO / ALT ADDRESS (selects `0x53`) |

Disconnect the old SPI wiring. SCL and SDA require external pull-ups to 3.3 V;
check whether the breakout already provides them. Choose resistance for the bus
capacitance so measured rise and fall times are at most 100 ns for these timing
settings. Do not leave CS or ALT ADDRESS floating. For ALT ADDRESS high, define
`ADXL345_I2C_ADDRESS=0x1d` when compiling the sensor driver.

## Key Concepts

- The driver explicitly selects HSI16 for the I2C1 kernel clock. PB8/PB9 are
  open-drain alternate-function outputs with internal pull-ups disabled.
- Timing fields are `(PRESC, SCLDEL, SDADEL, SCLH, SCLL)`:
  standard mode uses `(3,4,2,19,19)` (`0x30421313`); fast mode uses
  `(0,7,3,15,23)` (`0x00730F17`). At 16 MHz, each prescaled tick is
  `(PRESC+1)*62.5 ns`. The programmed high/low intervals are 5/5 µs
  and 1/1.5 µs respectively, before synchronization, filter and edge delays.
  Actual bus rates are therefore below 100/400 kHz. Analog filtering is enabled
  (50–260 ns assumed); digital filtering is disabled. These are conservative
  settings for edges up to 100 ns, not measured board timing. Select fast mode
  by passing `400000U` to `i2c1_init`; the application defaults to `100000U`.
- A read sends the register pointer without AUTOEND, waits for TC, then issues
  a repeated START with RD_WRN and AUTOEND. The peripheral NACKs the last byte
  and generates STOP. No SPI command bits are added to the register address.
- APIs accept unshifted 7-bit addresses and 1–255 bytes. They are blocking and
  intended for a single caller. Poll limits bound CPU iterations, not milliseconds.
- Errors clear sticky flags and reset the transfer state. Arbitration loss does
  not issue STOP. An initially busy bus is left alone. A slave holding SDA low
  physically is not recovered by peripheral reset and needs hardware recovery.
- Sensor reads use a temporary buffer, so partial failed transfers do not replace
  the application's last valid sample. Numeric UART errors match `i2c_status_t`
  in [i2c.h](I2C/Inc/i2c.h).
- A six-byte register read uses nine address/data byte slots (81 SCL pulses),
  versus seven bytes (56 SCK pulses) for the Week 12 SPI command plus payload.
  Both drivers occupy the CPU while polling; measured overhead remains a board
  exercise and depends on configured bus rates.

## Build and Run

Requires CMake and `arm-none-eabi-gcc` on PATH. From the repository root:

```bash
cd ESE-311_Bare-Metal-C/Week13-I2C/I2C
cmake -B build -DENABLE_LINT=ON
cmake --build build
cmake --build build --target flash
```

The flash target requires OpenOCD and a connected board. Open the board's UART
console at 115200 baud, 8N1. Successful initialization prints `ADXL345 I2C ready`.
With the board stationary, acceleration magnitude should be approximately 1 g.

Host tests, from the same `I2C/` directory:

```bash
cc -std=c11 -Wall -Wextra -Werror -I Inc tests/test_adxl345.c Src/adxl345.c -o /tmp/test-adxl345
/tmp/test-adxl345
```

Validation performed: ARM firmware build with strict warnings and host sensor
protocol tests. Hardware flashing, I2C waveforms, disconnect/reconnect behavior,
and acceleration measurements have not been validated. On hardware, check the
address ACK, repeated START, final NACK/STOP, edge timing at both rates, and
error reporting with the sensor disconnected.

## References

- Israel Gbati, *Bare-Metal Embedded C Programming* — I2C chapter.
- [STM32U5 reference manual RM0456](https://www.st.com/resource/en/reference_manual/rm0456-stm32u5-series-armbased-32bit-mcus-stmicroelectronics.pdf).
- [STM32U575 datasheet](https://www.st.com/resource/en/datasheet/stm32u575vg.pdf), alternate-function mapping.
- [ADXL345 datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/adxl345.pdf), I2C protocol and register map.

## AI Assistance

OpenAI Codex implemented the I2C transport, adapted the ADXL345 driver and demo,
and added host tests and documentation. Hardware validation remains outstanding.
