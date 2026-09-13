# Self-Study Embedded Engineering

A self-directed, college-level embedded systems curriculum covering firmware
architecture, bare-metal ARM/C development, and sensorless motor control.

All firmware targets the **NUCLEO-U575ZI-Q** (STM32U575ZIT6Q, Arm Cortex-M33).

## Courses

| Course | Focus | Primary text |
|---|---|---|
| [ESE-301 — Embedded Systems Design](ESE-301_Embedded-Systems-Design) | Firmware architecture and engineering design patterns | Elecia White, *Making Embedded Systems* |
| [ESE-311 — Bare-Metal C](ESE-311_Bare-Metal-C) | Register-level embedded C on Arm Cortex-M | Israel Gbati, *Bare-Metal Embedded C Programming* |
| [ECE-452 — Sensorless FOC](ECE-452_Sensorless-FOC) | Electric motor drives and sensorless field-oriented control | R. Krishnan, *PMSM and Brushless DC Motor Drives* |

Each course directory holds its syllabus, a reference-documentation index, and
one folder per week. Each week folder has a README covering objectives, what was
built, key concepts, and how to build and run it.

## Course Materials

| Course | Syllabus | Vendor docs and datasheets |
|---|---|---|
| ESE-301 | [Markdown](ESE-301_Embedded-Systems-Design/ESE-301_Making-Embedded-Systems_Syllabus.md) · [.docx](ESE-301_Embedded-Systems-Design/ESE-301_Making-Embedded-Systems_Syllabus.docx) | [ESE-301_References.md](ESE-301_Embedded-Systems-Design/ESE-301_References.md) |
| ESE-311 | [Markdown](ESE-311_Bare-Metal-C/ESE-311_Bare-Metal-Embedded-C-Programming_Syllabus.md) · [.docx](ESE-311_Bare-Metal-C/ESE-311_Bare-Metal-Embedded-C-Programming_Syllabus.docx) | [ESE-311_References.md](ESE-311_Bare-Metal-C/ESE-311_References.md) |
| ECE-452 | [Markdown](ECE-452_Sensorless-FOC/ECE-452_Sensorless-FOC-STM32-Implementation-in-C_Syllabus.md) · [.docx](ECE-452_Sensorless-FOC/ECE-452_Sensorless-FOC-STM32-Implementation-in-C_Syllabus.docx) | [ECE-452_References.md](ECE-452_Sensorless-FOC/ECE-452_References.md) |

Vendor reference manuals and datasheets are **not stored in this repository** —
each course's references file links to the official download for every document.

## Highlights

- [ESE-311_Bare-Metal-C/Week12-SPI](ESE-311_Bare-Metal-C/Week12-SPI) — bare-metal STM32U575 SPI driver and
  an ADXL345 accelerometer driver, no HAL and no DMA.
- [ESE-311_Bare-Metal-C/Week11-ADC](ESE-311_Bare-Metal-C/Week11-ADC) — register-level ADC bring-up,
  including the STM32U5 analog-supply and calibration sequence.
- [ECE-452_Sensorless-FOC/Week06-MTPA/foc-lib](ECE-452_Sensorless-FOC/Week06-MTPA/foc-lib) — integer-only
  sensorless FOC library with an STM32U5 + DRV8323 port layer and a 12-test host
  suite.
- [ECE-452_Sensorless-FOC/Week05-dq-Model-FOC](ECE-452_Sensorless-FOC/Week05-dq-Model-FOC) — closed-loop FOC
  simulation in GNU Octave with a flux observer, PLL, and SVPWM.
- [ESE-301_Embedded-Systems-Design/Week06-State-Machines](ESE-301_Embedded-Systems-Design/Week06-State-Machines) — an LED FSM
  decoupled from its HAL, exercised against a mock in CTest and a host HAL in a
  sandbox.

## Toolchain

Firmware projects build with CMake and the Arm GNU toolchain, and flash with
OpenOCD:

```bash
cd <project directory>
cmake -B build
cmake --build build
cmake --build build --target flash
```

Requires `arm-none-eabi-gcc`, `cmake`, and `openocd` on `PATH`. A few projects
use a hand-written Makefile (`make`, `make load`) or STM32CubeIDE instead; each
project README says which. Host-side test harnesses build with the system
compiler.

Environment variables for the STM32CubeIDE debug flow are documented in
[ESE-301_Embedded-Systems-Design/Week01-Introduction/blinky/ESE-301_Environment-Setup.md](ESE-301_Embedded-Systems-Design/Week01-Introduction/blinky/ESE-301_Environment-Setup.md).

## Repository Conventions

- **Course directories**: `AAA-###_Subject-Name` — the identifier, then a short
  subject name (`ESE-311_Bare-Metal-C`)
- **Week directories**: `Week##-Topic-Name` (`Week12-SPI`)
- **Document files**: `COURSEID_Descriptive-Name.ext`
  (`ESE-311_References.md`)
- **Headings**: week READMEs open with `# COURSE-ID Week NN — Topic`
- **Source headers**: hand-written C files carry an SPDX line, author, the course
  text where relevant, and a description of what the file does. Week attribution
  lives in the week README, not in every driver header.
- **Vendor and generated code** (CMSIS, STM32 HAL, CubeMX output) is left in its
  original form and is not held to these conventions.

## License and Attribution

- Project license: [LICENSE](LICENSE) (MIT)
- AI transparency and attribution policy: [NOTICE.md](NOTICE.md)
- Contribution expectations and content policy: [CONTRIBUTING.md](CONTRIBUTING.md)

Third-party books, vendor hardware documentation, and course materials are
referenced for educational context only; rights remain with their owners.
