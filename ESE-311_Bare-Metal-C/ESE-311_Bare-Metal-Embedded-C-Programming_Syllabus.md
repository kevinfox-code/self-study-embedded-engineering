# Bare-Metal Embedded C Programming

*Develop High-Performance Embedded Systems with C for ARM — 20-Week Syllabus*

---

## Course Overview

| | |
| --- | --- |
| **Textbook** | Israel Gbati, *Bare-Metal Embedded C Programming* (Packt, 2024) — ISBN 978-1-83546-081-8 |
| **Format** | 20-week self-study |
| **Pace** | 1 chapter per week; final weeks reserved for integration and projects |
| **Platform** | STM32 NUCLEO-U575ZI-Q (Arm Cortex-M33, STM32U575ZIT6Q); GNU Arm Embedded Toolchain + CMake + OpenOCD |
| **Prerequisites** | Familiarity with C programming; basic electronics helpful |
| **Workload** | Estimated 6–10 hours/week (reading + exercises + projects) |
| **Note** | Book examples use the NUCLEO-F411RE (Cortex-M4). Register names and peripheral layouts differ for the U575 — cross-reference RM0456 and the Cortex-M33 TRM throughout |

---

## Assessment Breakdown

| Component | Details | Weight |
| --- | --- | --- |
| Weekly Exercises (HW1–HW18) | 18 assignments × 3 pts | 54% |
| Project 1 | Mid-course driver stack | 20% |
| Project 2 | Capstone framework | 26% |

---

## 20-Week Schedule

| Week | Folder | Status | Topic | Reading | Assignment |
| --- | --- | --- | --- | --- | --- |
| **Wk 1** | `Week01-Dev-Environment` | ✅ Complete | Dev Environment & Tools | Ch. 1 (pp. 1–26) | HW1: Set up GNU Arm toolchain, CMake, and OpenOCD; build and flash a minimal bare-metal project to the NUCLEO-U575ZI-Q; blink LED by writing directly to GPIOC registers without HAL |
| **Wk 2** | `Week02-Register-Manipulation` | ✅ Complete | Constructing Peripheral Registers | Ch. 2 (pp. 27–62) | HW2: Define memory-mapped peripheral structs from scratch using RM0456; locate GPIO PORTC, AHB2ENR clock enable, MODER, ODR, and BSRR; blink LED and read a button entirely from register definitions |
| **Wk 3** | *(no dedicated folder — covered in Wk 4 setup)* | ⬜ Light | Build Process & GNU Toolchain | Ch. 3 (pp. 63–82) | HW3: Build a bare-metal project from the command line with arm-none-eabi-gcc; inspect the ELF with objdump; explore section headers and symbol table; upload via OpenOCD |
| **Wk 4** | `Week04-Linker-Scripts` | ✅ Complete | Linker Scripts & Startup Files | Ch. 4 (pp. 83–120) | HW4: Write a linker script and startup file for the U575; verify Flash/SRAM layout against RM0456; confirm .text, .data, and .bss placement with objdump and size |
| **Wk 5** | `Week05-Makefiles-CMSIS` | ✅ Complete | Make Build System | Ch. 5 (pp. 121–135) | HW5: Author a Makefile for a multi-file bare-metal project targeting the U575; add pattern rules, a phony clean target, and an OpenOCD flash target |
| **Wk 6** | `Week06-GPIO-CMSIS` | ✅ Complete | CMSIS & Peripheral Structs | Ch. 6 (pp. 137–154) | HW6: Integrate CMSIS-Core device headers for the U575; define peripheral structs using base addresses and offsets from RM0456; validate struct field alignment against the reference manual; exercise GPIO via CMSIS types |
| **Wk 7** | `Week07-GPIO-Driver` | ✅ Complete | GPIO Peripheral Driver | Ch. 7 (pp. 155–171) | HW7: Implement GPIO output (BSRR) and input (IDR) drivers; add software debounce for a button; measure toggle frequency on an oscilloscope |
| **Wk 8** | `Week08-SysTick` | ✅ Complete | SysTick Timer | Ch. 8 (pp. 173–181) | HW8: Write a SysTick-based millisecond delay and tick counter; verify timing accuracy with a scope; drive a periodic LED blink without busy-wait |
| **Wk 9** | `Week09-General-Purpose-Timers` | ✅ Complete | General-Purpose Timers (TIM) | Ch. 9 (pp. 183–194) | HW9: Configure a general-purpose timer in up-count mode with a prescaler and auto-reload; use the update event interrupt to drive a periodic callback; confirm period on a scope |
| **Wk 10** | `Week10-UART` | ✅ Complete | UART Driver | Ch. 10 (pp. 195–216) | HW10: Implement a polled UART driver at register level; echo characters; stream formatted data to a terminal at a fixed baud rate; verify with a USB-to-serial adapter |
| **Wk 11** | `Week11-ADC` | ✅ Complete | ADC Driver | Ch. 11 (pp. 219–239) | HW11: Configure the STM32U575 ADC for single-conversion and continuous modes; calibrate the ADC; read a potentiometer and print scaled voltage over UART |
| **Wk 12** | `Week12-SPI` | ✅ Complete | SPI Driver & ADXL345 Accelerometer | Ch. 12 (pp. 241–268) | HW12: Implement a polled SPI1 driver (Mode 3, TSIZE/CSTART, software CS); write an ADXL345 accelerometer driver; burst-read all six axis registers in one CS-low transaction; stream g-values over UART every 500 ms |
| **Wk 13** | `Week13-I2C` | 🔄 In Progress | I2C Driver | Ch. 13 (pp. 269–291) | HW13: Implement a bare-metal I2C driver; communicate with an I2C sensor; compare polling overhead vs. the SPI approach used in Wk 12 |
| **Wk 14** | *(planned)* | ⬜ Planned | External Interrupts (EXTI) — Project 1 Due | Ch. 14 (pp. 293–308) | **PROJECT 1 DUE** (end of week); HW14: Configure EXTI for a button interrupt; measure ISR entry latency with a logic analyzer or scope |
| **Wk 15** | *(planned)* | ⬜ Planned | Real-Time Clock (RTC) | Ch. 15 (pp. 309–339) | HW15: Implement an RTC driver; configure alarms and wakeup timer; demonstrate accurate timekeeping with BCD-formatted output over UART |
| **Wk 16** | *(planned)* | ⬜ Planned | Independent Watchdog (IWDG) | Ch. 16 (pp. 341–355) | HW16: Set up the IWDG; deliberately trigger a reset; pet the watchdog in the main loop and verify no spurious resets under normal operation |
| **Wk 17** | *(planned)* | ⬜ Planned | Direct Memory Access (DMA) — Project 2 Kickoff | Ch. 17 (pp. 357–387) | **PROJECT 2 KICKOFF** — see project spec; HW17: Build an ADC DMA circular buffer and a UART DMA driver; profile CPU load savings vs. polling |
| **Wk 18** | *(planned)* | ⬜ Planned | Power Management | Ch. 18 (pp. 389–404) | HW18: Measure MCU supply current in Sleep, Stop, and Standby modes using a bench meter; implement a wakeup driver and verify correct resume behavior |
| **Wk 19** | *(planned)* | ⬜ Planned | Integration Week | Review all chapters | Integrate all peripheral drivers into a cohesive bare-metal framework; write unit tests for at least two driver modules; finalize Project 2 firmware |
| **Wk 20** | *(planned)* | ⬜ Planned | Capstone Presentations | Review all | **PROJECT 2 DUE** — live demo + written report |

---

## Project Specifications

### Project 1 — Bare-Metal Peripheral Driver Stack

**Timeline: Weeks 12–14**

Without any HAL or vendor driver library, integrate a complete set of peripheral drivers for the NUCLEO-U575ZI-Q: GPIO, SysTick, UART, ADC, and either SPI or I2C. Demonstrate reading a sensor and streaming data over UART at a fixed baud rate. Deliverables: source code in a dedicated repo folder with CMakeLists.txt, working linker script and startup file, oscilloscope or logic-analyzer captures, and a brief (1–2 page) design rationale document.

### Project 2 — Integrated Bare-Metal Embedded Framework

**Timeline: Weeks 17–20**

Design and implement a cohesive bare-metal embedded framework incorporating all major peripherals covered in the textbook. Required components: GPIO with EXTI interrupt handling, general-purpose timer for PWM or timing, UART with DMA-based transfers, ADC with DMA circular buffer, SPI and I2C drivers, RTC with alarm support, and IWDG integration. Implement at least one low-power mode with a proper wakeup sequence. Deliverables: source code with CMakeLists.txt, memory map, timing diagrams, measured ISR latency, power consumption data (Sleep/Stop/Standby), and a demo video or live demonstration.

---

## Exercise Guidelines

- Submit source code in a Git commit with a brief README explaining what was implemented and any known issues.
- Include a short reflection (3–5 sentences) on what you learned and what was challenging.
- Test code on real hardware (NUCLEO-U575ZI-Q) whenever possible; note any simulation-only testing.
- Assignments requiring timing measurements must include annotated oscilloscope screenshots or logic-analyzer captures.
- When book examples use F411RE register names that differ on the U575, cross-reference RM0456 and update accordingly.

---

## Reference Documents

| Document | Location | Purpose |
| --- | --- | --- |
| Israel Gbati, *Bare-Metal Embedded C Programming* (Packt, 2024) | Primary textbook |
| RM0456 — STM32U5 Series Reference Manual | `ESE-311_Bare-Metal-C/books/rm0456-stm32u5-series-armbased-32bit-mcus-stmicroelectronics.pdf` | Peripheral register reference for U575 |
| STM32U575AG Datasheet | `ESE-311_Bare-Metal-C/books/stm32u575ag.pdf` | Electrical specs and pin assignments |
| UM2861 — NUCLEO-U575ZI Board Manual | `ESE-311_Bare-Metal-C/books/um2861-stm32u5-nucleo144-board-mb1549-stmicroelectronics.pdf` | Board schematic and jumper settings |
| Arm Cortex-M33 Generic User Guide | `ESE-311_Bare-Metal-C/books/arm_cortex_m33_dgug_100235_06_en.pdf` | Core architecture: SysTick, NVIC, MPU, FPU |
