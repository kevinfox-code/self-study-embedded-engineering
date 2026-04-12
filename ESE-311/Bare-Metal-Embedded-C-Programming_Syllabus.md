# Bare-Metal Embedded C Programming
*Develop High-Performance Embedded Systems with C for ARM — 16-Week Syllabus*

---

## Course Overview

| | |
|---|---|
| **Textbook** | Israel Gbati, *Bare-Metal Embedded C Programming* (Packt, 2024) — ISBN 978-1-83546-081-8 |
| **Format** | 16-week self-study or structured course |
| **Pace** | ~1–2 chapters per week (short chapters paired); final weeks reserved for integration and projects |
| **Platform** | STM32 NUCLEO-F411RE (Arm Cortex-M4); STM32CubeIDE + GNU Arm Embedded Toolchain + OpenOCD |
| **Prerequisites** | Familiarity with C programming; basic electronics helpful |
| **Workload** | Estimated 6–10 hours/week (reading + homework + projects) |

---

## Assessment Breakdown

| Component | Details | Weight |
|---|---|---|
| Weekly Homework (HW1–HW14) | 14 assignments × 3 pts | 42% |
| Project 1 | Mid-course project | 25% |
| Project 2 | Capstone project | 33% |

---

## 16-Week Schedule

| Week | Topic | Reading | Assignment |
|---|---|---|---|
| **Wk 1** | Dev Environment & Register Fundamentals | Ch. 1 (pp. 1–26) + Ch. 2 (pp. 27–62) | HW1: Set up STM32CubeIDE + GNU toolchain; blink the NUCLEO-F411 LED by writing directly to GPIO registers (no HAL) |
| **Wk 2** | The Build Process & GNU Toolchain | Ch. 3 (pp. 63–82) | HW2: Build a bare-metal project from the command line; compare object files and map file output with arm-none-eabi-objdump; upload via OpenOCD |
| **Wk 3** | Linker Scripts & Startup Files | Ch. 4 (pp. 83–120) | HW3: Write a linker script and startup file from scratch; verify section placement (.text, .data, .bss) with objdump and size |
| **Wk 4** | Make Build System & CMSIS | Ch. 5 (pp. 121–135) + Ch. 6 (pp. 137–154) | HW4: Author a Makefile for a multi-file project; integrate CMSIS-Core headers and validate peripheral struct offsets against the reference manual |
| **Wk 5** | GPIO Peripheral Driver | Ch. 7 (pp. 155–171) | HW5: Implement an output driver using GPIOx_BSRR and an input driver using GPIOx_IDR; measure toggle frequency on an oscilloscope |
| **Wk 6** | SysTick Timer & General-Purpose Timers | Ch. 8 (pp. 173–181) + Ch. 9 (pp. 183–194) | HW6: Write a SysTick-based delay function; configure a general-purpose timer for time-interval measurement and delay generation; verify timing on a scope |
| **Wk 7** | UART From Scratch | Ch. 10 (pp. 195–216) | HW7: Implement a UART driver (polling mode) at register level; echo characters and stream formatted data to a terminal at a fixed baud rate |
| **Wk 8** | ADC Driver & Project 1 Kickoff | Ch. 11 (pp. 219–239) + Review Ch. 1–10 | **PROJECT 1 KICKOFF** — see project spec; begin reading and converting analog sensor data with your bare-metal ADC driver |
| **Wk 9** | SPI Protocol & Driver | Ch. 12 (pp. 241–268) | HW8: Write an SPI driver; interface the ADXL345 accelerometer; read and print X/Y/Z data over UART without using any HAL or vendor library |
| **Wk 10** | I2C Protocol & Driver | Ch. 13 (pp. 269–291) | HW9: Implement an I2C driver; communicate with an I2C sensor; compare polling overhead vs. SPI on the same MCU |
| **Wk 11** | External Interrupts (EXTI) — Project 1 Due | Ch. 14 (pp. 293–308) | **PROJECT 1 DUE** (end of week); HW10: Configure EXTI for a button interrupt; measure ISR entry latency with a logic analyzer or scope |
| **Wk 12** | Real-Time Clock (RTC) | Ch. 15 (pp. 309–339) | HW11: Implement an RTC driver; configure alarms and wakeup timer; demonstrate accurate timekeeping with BCD-formatted output over UART |
| **Wk 13** | Independent Watchdog (IWDG) & DMA | Ch. 16 (pp. 341–355) + Ch. 17 (pp. 357–387) | HW12: Set up and test the IWDG; then build an ADC DMA circular buffer and a UART DMA driver; profile CPU savings vs. polling |
| **Wk 14** | Power Management & Project 2 Kickoff | Ch. 18 (pp. 389–404) + Review Ch. 11–18 | **PROJECT 2 KICKOFF** — see project spec; HW13: Measure MCU current in Sleep, Stop, and Standby modes using a bench meter; implement a wakeup driver |
| **Wk 15** | Integration Week — Review & Refinement | Review all chapters as needed | HW14: Integrate all peripheral drivers into a cohesive embedded framework; write unit tests for at least two driver modules; finalize Project 2 firmware |
| **Wk 16** | Capstone Presentations & Project 2 Due | Review all | **PROJECT 2 DUE** — live demo + written report |

---

## Project Specifications

### Project 1 — Bare-Metal Peripheral Driver Stack
**Timeline: Weeks 8–11**

Without any HAL or vendor driver library, implement and integrate a complete set of peripheral drivers for the NUCLEO-F411RE: GPIO, SysTick, UART, ADC, and either SPI or I2C. Demonstrate reading a sensor and streaming data over UART at a fixed baud rate. Deliverables: fully commented source code with custom Makefile, working linker script and startup file, oscilloscope or logic-analyzer captures, and a brief (1–2 page) design rationale document.

### Project 2 — Integrated Bare-Metal Embedded Framework
**Timeline: Weeks 14–16**

Design and implement a cohesive bare-metal embedded framework incorporating all major peripherals covered in the textbook. Required components: GPIO with EXTI interrupt handling, general-purpose timer for PWM or timing, UART with DMA-based transfers, ADC with DMA circular buffer, SPI and I2C drivers, RTC with alarm support, and IWDG integration. Implement at least one low-power mode with a proper wakeup sequence. Deliverables: source code with Makefile, memory map, timing diagrams, measured ISR latency, power consumption data (Sleep/Stop/Standby), and a demo video or live demonstration.

---

## Homework Guidelines

- Submit source code in a ZIP with a brief README explaining what was implemented and any known issues.
- Include a short reflection (3–5 sentences) on what you learned and what was challenging.
- Late submissions accepted with a 10% per-day penalty up to 3 days; beyond that, contact the instructor.
- Collaboration is encouraged for discussion, but submitted code must be your own work.
- Test your code on real hardware (NUCLEO-F411RE) when possible; note any simulation-only testing.
- Hardware assignments requiring oscilloscope captures must include annotated screenshots or photos.
