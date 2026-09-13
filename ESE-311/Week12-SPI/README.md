# ESE-311 Week 12 — SPI Driver and ADXL345 Accelerometer

## Objectives

- Understand the STM32U575 SPI peripheral register model (SPI_CR1, SPI_CFG1, SPI_CFG2, SPI_SR)
- Implement a bare-metal SPI1 driver in master mode using polling (no HAL, no DMA)
- Write an ADXL345 accelerometer driver that communicates over SPI to read X/Y/Z acceleration data
- Practice proper SPI Mode 3 configuration (CPOL=1, CPHA=1) and software chip-select management

## What Was Built

- **SPI1 driver** (`Src/spi.c`, `Inc/spi.h`): initializes GPIO pins PA5 (SCK), PA6 (MISO), PA7 (MOSI) to AF5 and PD14 as a GPIO chip-select output; configures SPI1 for 8-bit full-duplex master mode at fPCLK/4 in SPI Mode 3; implements polled `spi1_transmit()` and `spi1_receive()` using TSIZE/CSTART and the TXP, RXP, and EOT status flags
- **ADXL345 driver** (`Src/adxl345.c`, `Inc/adxl345.h`): `adxl_init()` configures the sensor for ±4g range and enables measurement mode; `adxl_read()` performs a multi-byte SPI read of the six acceleration data registers (0x32–0x37) in a single CS-asserted transaction
- **Main loop** (`Src/main.c`): reads raw X/Y/Z counts every 500 ms, converts to g-values (4 mg/LSB for ±4g range), and prints both raw hex bytes and decimal axis values over UART

## Key Concepts

- **SPI Mode 3**: CPOL=1 (clock idles high) and CPHA=1 (data sampled on the second/falling edge) — required by the ADXL345
- **STM32U575 SPI register differences from F4**: configuration lives in SPI_CFG1/CFG2 (write-protected while SPE=1); TSIZE must be written with SPE=0; CSTART triggers the transfer; EOT replaces TXC for transfer completion
- **Software chip select (SSM=1)**: the NSS pin is managed manually via GPIO rather than by hardware, giving explicit control over the CS assert/deassert timing around multi-byte transactions
- **TSIZE-based transfers**: writing the frame count to CR2 before asserting CSTART lets hardware track end-of-transfer automatically and raise the EOT flag
- **Multi-byte read protocol**: the ADXL345 requires bit 7 (READ) and bit 6 (MB) of the address byte to be set for a burst read; six bytes (X_L, X_H, Y_L, Y_H, Z_L, Z_H) are read in one CS-low window
- **Pre-loading TXDR before CSTART**: writing the first byte into TXDR before asserting CSTART ensures TXFIFO is non-empty when the clock starts, preventing an underrun on the first frame

## Build and Run

```bash
cd ESE-311/Week12-SPI/SPI
cmake -B build
cmake --build build
cmake --build build --target flash
```

Open a serial terminal on the ST-LINK virtual COM port at 115200 8N1 to see the
raw bytes and the decoded axis values.

## VS Code Debug

Requires the Cortex-Debug extension, with `arm-none-eabi-gdb` and `openocd` on
`PATH`. Open the `SPI/` folder as the workspace and launch **Debug (OpenOCD)**,
which builds `build/spi.elf` via the `build-spi` task.

## Contents

- [`SPI/`](SPI) — the buildable STM32U575 project.
- [`ESE-311_SPI-Peripherals.md`](ESE-311_SPI-Peripherals.md) — chapter notes on
  the three STM32U5 SPI instances and their register model.

## References

- [ESE-311_References.md](../ESE-311_References.md) — download links for everything below
- [ADXL345 Datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/adxl345.pdf)
- [STM32U575 Reference Manual RM0456 (SPI chapter)](https://www.st.com/resource/en/reference_manual/rm0456-stm32u575585-armbased-32bit-mcus-stmicroelectronics.pdf)

## AI Assistance

Claude Code assisted with register-level SPI configuration details, TSIZE/CSTART
sequencing, and debugging the chip-select glitch on init. Reviewed and edited by
the maintainer.
