# ECE-452 — Vendor And Reference Documentation

Vendor manuals, datasheets, and the course text are **not stored in this
repository**. `ECE-452_Sensorless-FOC/books/` is git-ignored; download the documents below into
it if you want them locally.

## Target Hardware

The firmware work targets a **NUCLEO-U575ZI-Q** (STM32U575ZIT6Q) driving a
**TI DRV8323RS** three-phase gate driver into a PMSM.

## Motor Control

| Document | ID | Covers |
|---|---|---|
| [DRV8323RS Three-Phase Gate Driver](https://www.ti.com/lit/ds/symlink/drv8323r.pdf) | TI SLVSDJ4 | SPI register map, current-sense amplifiers, fault reporting, nFAULT behavior |
| [STM32 PMSM FOC SDK / MC application notes](https://www.st.com/en/embedded-software/x-cube-mcsdk.html) | X-CUBE-MCSDK | ST's reference FOC implementation, useful for cross-checking |
| [AN4013 — STM32 timer overview](https://www.st.com/resource/en/application_note/an4013-stm32-crossseries-timer-overview-stmicroelectronics.pdf) | AN4013 | Center-aligned PWM and complementary outputs with dead time |

### Sensorless FOC application notes

The syllabus names these three as secondary reading alongside Krishnan:

- ST **AN4220** — Sensorless FOC for PMSM motor drives
- Microchip **AN1078** — Sensorless field-oriented control of a PMSM
- NXP **DRM148** — Sensorless PMSM FOC design reference manual

## MCU

| Document | ID | Covers |
|---|---|---|
| [STM32U575/585 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0456-stm32u5-series-armbased-32bit-mcus-stmicroelectronics.pdf) | RM0456 | TIM1 PWM generation, injected ADC conversions triggered from TIM1 TRGO, SPI |
| [STM32U575xx Datasheet](https://www.st.com/en/microcontrollers-microprocessors/stm32u575ag.html) | DS13737 | ADC timing and pin mapping |
| [Arm Cortex-M33 Devices Generic User Guide](https://developer.arm.com/documentation/100235/latest/) | Arm 100235 | Core, NVIC, and FPU behaviour relevant to ISR timing budgets |

## Course Text

- R. Krishnan, *Permanent Magnet Synchronous and Brushless DC Motor Drives*,
  CRC Press, 2010 — the machine-physics and FOC derivations this course follows.
- R. Krishnan, *Electric Motor Drives: Modeling, Analysis, and Control*,
  Prentice Hall, 2001 — secondary text.

  See [ECE-452_Sensorless-FOC-STM32-Implementation-in-C_Syllabus.md](ECE-452_Sensorless-FOC-STM32-Implementation-in-C_Syllabus.md).

## Tools

- [GNU Octave](https://octave.org/) — runs the Week 5 closed-loop simulation
- [Arm GNU Toolchain (arm-none-eabi)](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
- [STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html) —
  generates the board profile consumed by `Week06-MTPA/App`
