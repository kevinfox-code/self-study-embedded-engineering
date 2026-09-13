# ESE-301 — Vendor And Reference Documentation

Vendor manuals and datasheets are **not stored in this repository** — they are
copyrighted and large. `ESE-301/books/` is git-ignored; download the documents
below into it if you want them locally.

## Target Hardware

ESE-301 work targets the **NUCLEO-U575ZI-Q** board (STM32U575ZIT6Q,
Arm Cortex-M33), driven through the STM32 HAL and STM32CubeMX-generated
projects rather than at the register level.

## Core Documents

| Document | ST/Arm ID | Covers |
|---|---|---|
| [STM32 Nucleo-144 Boards (MB1549)](https://www.st.com/resource/en/user_manual/um2861-stm32u5-nucleo144-board-mb1549-stmicroelectronics.pdf) | UM2861 | Board pinout, LEDs, user button, ST-LINK V3E |
| [STM32U575/585 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0456-stm32u5-series-armbased-32bit-mcus-stmicroelectronics.pdf) | RM0456 | Peripheral detail behind the HAL calls |
| [STM32U575xx Datasheet](https://www.st.com/en/microcontrollers-microprocessors/stm32u575ag.html) | DS13737 | Pinout and alternate-function tables |
| [Description of STM32U5 HAL and LL drivers](https://www.st.com/resource/en/user_manual/um2883-description-of-stm32u5-hal-and-lowlayer-drivers-stmicroelectronics.pdf) | UM2883 | HAL API reference for the generated code |
| [Arm Cortex-M33 Devices Generic User Guide](https://developer.arm.com/documentation/100235/latest/) | Arm 100235 | NVIC, SysTick, exception model |

## Course Text

- Elecia White, *Making Embedded Systems: Design Patterns for Great Software*, O'Reilly —
  the architecture and design-pattern material this course follows. See
  [ESE-301_Making-Embedded-Systems_Syllabus.md](ESE-301_Making-Embedded-Systems_Syllabus.md).

## Toolchain

- [STM32CubeIDE / STM32CubeMX](https://www.st.com/en/development-tools/stm32cubeide.html) —
  generates the `blinky` and `ESE-301_hw4` project trees
- [Arm GNU Toolchain (arm-none-eabi)](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
- Environment variables required by the VS Code debug flow are documented in
  [Week01-Introduction/blinky/ESE-301_Environment-Setup.md](Week01-Introduction/blinky/ESE-301_Environment-Setup.md)
