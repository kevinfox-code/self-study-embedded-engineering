# ESE-311 — Vendor And Reference Documentation

Reference manuals and datasheets are **not stored in this repository**. They are
vendor-copyrighted and large (the STM32U5 reference manual alone is ~70 MB), so
`ESE-311/books/` is git-ignored. Download the documents below into that folder
if you want them locally.

## Target Hardware

All ESE-311 work targets the **NUCLEO-U575ZI-Q** board (STM32U575ZIT6Q,
Arm Cortex-M33, 160 MHz).

## Core Documents

| Document | ST/Arm ID | Covers |
|---|---|---|
| [STM32U575/585 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0456-stm32u5-series-armbased-32bit-mcus-stmicroelectronics.pdf) | RM0456 | Register maps for RCC, GPIO, TIM, USART, ADC, SPI, I2C — the primary source for every driver in this course |
| [STM32U575xx Datasheet](https://www.st.com/en/microcontrollers-microprocessors/stm32u575ag.html) | DS13737 | Pinout, alternate-function tables, electrical characteristics |
| [STM32 Nucleo-144 Boards (MB1549)](https://www.st.com/resource/en/user_manual/um2861-stm32u5-nucleo144-board-mb1549-stmicroelectronics.pdf) | UM2861 | LED and user-button pin assignments, ST-LINK V3E virtual COM port, Zio/morpho connector maps |
| [Arm Cortex-M33 Devices Generic User Guide](https://developer.arm.com/documentation/100235/latest/) | Arm 100235 | SysTick, NVIC, core registers, instruction set |
| [Armv8-M Architecture Reference Manual](https://developer.arm.com/documentation/ddi0553/latest/) | Arm DDI 0553 | Exception model and memory ordering background |

## Course Text

- Israel Gbati, *Bare-Metal Embedded C Programming*, Packt, 2024 —
  the chapter sequence this course follows. See
  [ESE-311_Bare-Metal-Embedded-C-Programming_Syllabus.md](ESE-311_Bare-Metal-Embedded-C-Programming_Syllabus.md).

## Component Datasheets

| Part | Used in | Link |
|---|---|---|
| Analog Devices ADXL345 | [Week12-SPI](Week12-SPI) accelerometer driver | [Datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/adxl345.pdf) |

## Toolchain

- [Arm GNU Toolchain (arm-none-eabi)](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
- [OpenOCD](https://openocd.org/) — used by every `cmake --build build --target flash` target
- [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html)
