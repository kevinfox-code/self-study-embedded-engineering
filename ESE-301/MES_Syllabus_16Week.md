# Making Embedded Systems
## Design Patterns for Great Software
### 16-Week Self-Study Syllabus
*by Elecia White — O'Reilly Media*

---

| Duration | Primary Text | Format | Projects |
|----------|-------------|--------|----------|
| 16 Weeks | Making Embedded Systems | Self-Study / Structured | 2 Major (1 Capstone) |

---

## Course Overview

This 16-week course takes you from the fundamentals of embedded systems development through a full application built with production-quality design patterns. Reading follows *Making Embedded Systems: Design Patterns for Great Software* by Elecia White (O'Reilly) as the primary text, chapter by chapter, one chapter per week. Each week pairs a focused reading assignment with a hands-on homework that produces working C code. Two projects — a mid-course sensor logger and a capstone application — accumulate real deliverables throughout.

| | |
|---|---|
| **Pace** | ~1 chapter per week; Weeks 8 and 16 reserved for projects |
| **Prerequisites** | Familiarity with C programming; basic electronics helpful |
| **Workload** | Estimated 6–10 hours/week (reading + homework + projects) |
| **Hardware** | Any ARM Cortex-M or AVR development board (e.g., STM32 Nucleo, Arduino, Nordic nRF) |

---

## Assessment Breakdown

| Component | Details | Weight |
|-----------|---------|--------|
| Weekly Homework (HW1–HW15) | 15 assignments × 2.8 pts each | 42% |
| Project 1 — Interrupt-Driven Sensor Logger | Mid-course project (Weeks 8–11) | 25% |
| Project 2 — Capstone Embedded Application | Final project (Weeks 12–16) | 33% |

---

## 16-Week Schedule

### Phase 1: Foundations (Weeks 1–4)

| Week | Topic | Reading | Assignment |
|------|-------|---------|------------|
| Wk 1 | Introduction: What is an Embedded System? | Ch. 1 (pp. 14–30) | **HW1:** Set up toolchain; blink an LED on your target board; write a brief reflection on embedded vs. desktop software differences |
| Wk 2 | Creating a System Architecture: Diagrams, Interfaces & Design for Change | Ch. 2 (pp. 31–77) | **HW2:** Draw a context diagram and block diagram for a simple embedded system of your choice; implement a basic driver interface (open/close/read/write) skeleton in C |
| Wk 3 | Getting Your Hands on the Hardware: Datasheets, Schematics & Debugging Toolbox | Ch. 3 (pp. 80–154) | **HW3:** Read a component datasheet and extract key parameters; practice reading the Arduino schematic; create a command-response test harness in C |
| Wk 4 | Inputs, Outputs & Timers: Registers, GPIO, Interrupts on Buttons & Timer Math | Ch. 4 (pp. 157–214) | **HW4:** Implement GPIO input/output; add a button interrupt with debounce; configure a hardware timer and verify frequency with an oscilloscope or logic analyzer |

### Phase 2: I/O, Communication & System Flow (Weeks 5–8)

| Week | Topic | Reading | Assignment |
|------|-------|---------|------------|
| Wk 5 | Interrupts: Interrupt Handlers, Shared Data & Design Patterns | Ch. 5 (pp. 215–270) | **HW5:** Implement a multi-level interrupt priority example; demonstrate a shared-resource protection scheme (volatile, critical section, or flag-based); document ISR latency |
| Wk 6 | Managing the Flow of Activity: Main Loop, Scheduling & State Machines | Ch. 6 (pp. 271–330) | **HW6:** Implement a finite state machine for a simple device (e.g., vending machine or traffic light); compare interrupt-driven vs. polling approaches |
| Wk 7 | Communicating with Peripherals: UART, SPI, I2C, USB | Ch. 7 (pp. 331–395) | **HW7:** Write a UART transmit/receive driver; implement SPI or I2C communication to a real peripheral; document bandwidth and timing tradeoffs |
| Wk 8 | Project 1 Kickoff & Mid-Course Review | Review Ch. 1–7 | **PROJECT 1 KICKOFF** — see project specification below |

### Phase 3: Systems, Debugging & Optimization (Weeks 9–12)

| Week | Topic | Reading | Assignment |
|------|-------|---------|------------|
| Wk 9 | Putting Together a System: Buffers, Pipelines & Common Peripherals (LCD, ADC, Flash) | Ch. 8 (pp. 396–450) | **HW8:** Implement a circular buffer for UART RX; add a producer-consumer pipeline between two subsystems; measure throughput |
| Wk 10 | Getting into Trouble: Debugging Skills, Hard Faults & Cleverness | Ch. 9 (pp. 451–510) | **HW9:** Intentionally trigger a hard fault and recover it; use a watchdog timer; instrument code with a cycle counter to profile an ISR |
| Wk 11 | Building Connected Devices: IoT, Firmware Updates & Security Basics | Ch. 10 (pp. 511–565) | **HW10:** Survey wireless protocol options for a battery-operated sensor node; write a design trade-off memo; sketch a simple OTA update state machine |
| Wk 12 | Doing More with Less: RAM, Code Space & Processor Cycle Optimization | Ch. 11 (pp. 566–620) | **HW11:** Audit RAM and flash usage of a prior project; apply const/static optimizations; reduce RAM footprint by at least 20% and document the changes |

### Phase 4: Math, Power, Motors & Capstone (Weeks 13–16)

| Week | Topic | Reading | Assignment |
|------|-------|---------|------------|
| Wk 13 | Math: Fixed-Point Arithmetic, Floating Point & DSP Basics | Ch. 12 (pp. 621–680) | **HW12:** Implement a fixed-point multiply and compare precision/performance against float; implement a simple moving-average filter in fixed-point |
| Wk 14 | Reducing Power Consumption: Processor Sleep, Architecture & Battery Life | Ch. 13 (pp. 681–730) | **HW13:** Add low-power sleep modes to a prior project; measure current draw with and without sleep; estimate battery life for a coin-cell target |
| Wk 15 | Motors and Movement: Motor Types, PWM Drive & Control | Ch. 14 (pp. 731–780+) | **HW14:** Implement PWM-based DC motor speed control or servo positioning; add overcurrent protection logic; log and plot speed vs. duty cycle |
| Wk 16 | Final Review & Project 2 Presentations | Review all chapters | **PROJECT 2 DUE** — demo + written report |

---

## Project Specifications

### Project 1 — Interrupt-Driven Sensor Logger *(Weeks 8–11)*

Design and implement a multi-peripheral embedded application that reads a sensor via SPI or I2C, processes data through a state machine built using the patterns from Chapter 6, and logs results over UART. The design must include interrupt-driven I/O, proper shared-variable protection, and a documented module structure following the Chapter 2 interface design guidelines.

**Deliverables:**
- Source code as a ZIP with a README describing each module and how to build/flash
- State machine diagram and brief module architecture description (1 page)
- Interrupt latency measurement (logic analyzer or cycle counter) for at least one ISR
- Short reflection (3–5 sentences): what you learned, what was challenging

---

### Project 2 — Capstone Embedded Application *(Weeks 12–16)*

Build a complete embedded application that integrates at least three subsystems from across the course: for example, a sensor acquisition pipeline, a state-machine controller, a communication output (UART/SPI/I2C), and a power or optimization strategy. Apply design patterns from throughout the book — driver interfaces, adapter patterns, dependency injection, and fixed-point math where appropriate.

**Deliverables:**
- Full source code with clear module boundaries and a README
- Architecture block diagram showing data flow between subsystems
- Test plan with at least 5 documented test cases and pass/fail results
- Profiling report: ISR timing, RAM/flash usage, and any power measurements
- Live demo or recorded video walkthrough (5–10 minutes)
- Written report (2–3 pages): design decisions, tradeoffs, and lessons learned

---

## Homework Guidelines

- Submit source code in a ZIP with a brief README explaining what was implemented and any known issues.
- Include a short reflection (3–5 sentences) on what you learned and what was challenging.
- Late submissions accepted with a 10% per-day penalty up to 3 days; beyond that, contact the instructor.
- Collaboration is encouraged for discussion, but submitted code must be your own work.
- Test your code on real hardware when possible; note any simulation-only testing in your README.
- Use a consistent coding style. Comment any non-obvious register writes or timing decisions.

---

## Study Tips & Workflow

Read the assigned chapter before starting the homework, not after. White's book is written to be read linearly — each chapter builds on the last — and the homework is designed to immediately apply what you just read. If you hit trouble in a lab, re-read the relevant section before debugging; the answer is usually there.

For each new peripheral or protocol, look up the actual datasheet for your hardware before writing a single line of code. Chapter 3 covers datasheet reading in depth for exactly this reason. The sections you need are the register map, the timing diagrams, and the electrical characteristics.

Write each module (GPIO driver, UART driver, state machine, buffer) as a standalone `.c`/`.h` pair. Test the logic on your PC first using a simple test harness with `printf`, then port the validated module to the microcontroller. This separation reduces hardware debug time dramatically and is the approach the book advocates.

Instrument everything. Use UART logging in early weeks. Add a software cycle counter or logic analyzer for ISR timing in middle weeks. The debugging chapter (Ch. 9) contains practical techniques for tracking down the bugs that will inevitably appear — read it carefully and apply the systematic methodology it describes.

Keep a lab notebook — physical or digital — with one entry per session. Record what you expected, what happened, what you changed, and why it worked or failed. This becomes the basis of your project reports with minimal additional effort.

---

*Making Embedded Systems — 16-Week Syllabus*
