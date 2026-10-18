# Feedback Systems

*An Introduction for Scientists and Engineers — 15-Week Syllabus*

---

## Course Overview

| | |
| --- | --- |
| **Textbook** | Karl Johan Åström and Richard M. Murray, *Feedback Systems: An Introduction for Scientists and Engineers* (Princeton University Press, 2008) — ISBN 978-0-691-13576-2. Free electronic edition v2.10b, 22 Feb 2009 |
| **Format** | 15-week self-study |
| **Pace** | About one chapter per week, following the authors' 14–15 week plan in the Preface: two weeks for Chapter 4 and two for Chapter 12 |
| **Platform** | GNU Octave for modeling, simulation, and design. NUCLEO-U575ZI-Q (Arm Cortex-M33) for the embedded controller in Weeks 11–15 |
| **Prerequisites** | Linear algebra (matrices, eigenvalues), ordinary differential equations, complex numbers. Laplace transforms help but are not required (the text covers them only in §8.5) |
| **Workload** | Estimated 6–10 hours/week (reading + exercises + simulation) |
| **Running example** | The cruise control model of §3.1 recurs throughout the text (Examples 5.11, 6.10, 8.7, 10.3, 10.5–10.7, 12.1, 12.5). This course builds that model once in Week 1 and reuses it every week |

---

## Textbook Chapter Map

> **PDF page = book page + 12** for `am08-complete_22Feb09.pdf` (book p. 1 is PDF p. 13).

| Chapter | Title | Book Pages | PDF Pages |
| --- | --- | --- | --- |
| Ch. 1 | Introduction | pp. 1–26 | PDF 13–38 |
| Ch. 2 | System Modeling | pp. 27–64 | PDF 39–76 |
| Ch. 3 | Examples | pp. 65–94 | PDF 77–106 |
| Ch. 4 | Dynamic Behavior | pp. 95–130 | PDF 107–142 |
| Ch. 5 | Linear Systems | pp. 131–166 | PDF 143–178 |
| Ch. 6 | State Feedback | pp. 167–200 | PDF 179–212 |
| Ch. 7 | Output Feedback | pp. 201–228 | PDF 213–240 |
| Ch. 8 | Transfer Functions | pp. 229–266 | PDF 241–278 |
| Ch. 9 | Frequency Domain Analysis | pp. 267–292 | PDF 279–304 |
| Ch. 10 | PID Control | pp. 293–314 | PDF 305–326 |
| Ch. 11 | Frequency Domain Design | pp. 315–346 | PDF 327–358 |
| Ch. 12 | Robust Performance | pp. 347–376 | PDF 359–388 |

---

## Assessment Breakdown

| Component | Details | Weight |
| --- | --- | --- |
| Weekly Exercises (HW1–HW14) | 14 assignments | 50% |
| Project 1 | Cruise control model and state-space design (Octave) | 20% |
| Project 2 | Embedded cruise controller on the NUCLEO-U575ZI-Q | 30% |

---

## 15-Week Schedule

| Week | Folder | Status | Topic | Reading | Assignment |
| --- | --- | --- | --- | --- | --- |
| **Wk 1** | `Week01-Cruise-Control` | 🔄 In Progress | Introduction: What Is Feedback? | Ch. 1 (pp. 1–26) | HW1: Exercise 1.4. Rebuild the cruise control model behind Figure 1.13 (§3.1) in Octave, reproduce the figure, and retune the PI gains so a 1000 kg car overshoots by ≤ 1 m/s. Exercise 1.5: show that integral action gives zero steady-state error |
| **Wk 2** | *(planned)* | ⬜ Planned | System Modeling | Ch. 2 (pp. 27–64) | HW2: Write state-space and difference-equation models (§2.2) for a spring–mass system. Implement the difference equation as a C module, test it on the host, and compare it against an Octave ODE solution |
| **Wk 3** | *(planned)* | ⬜ Planned | Examples — Project 1 Kickoff | Ch. 3 (pp. 65–94) | **PROJECT 1 KICKOFF**. HW3: Exercise 3.1. Reproduce the hill response of Figure 3.3b, vary the mass by ±25%, and redesign the controller to return within 1% of the set speed within 3 s of the start of the hill |
| **Wk 4** | *(planned)* | ⬜ Planned | Dynamic Behavior: Stability | Ch. 4 §4.1–4.3 (pp. 95–110) | HW4: Exercise 4.3. Phase portrait of the PI cruise loop with throttle saturation (third gear, k_p = 0.5, k_i = 0.1, 20 m/s) |
| **Wk 5** | *(planned)* | ⬜ Planned | Lyapunov Stability and Nonlocal Behavior | Ch. 4 §4.4–4.6 (pp. 110–130) | HW5: Exercise 4.4 (candidate Lyapunov functions); find the region of attraction of the saturated cruise loop numerically |
| **Wk 6** | *(planned)* | ⬜ Planned | Linear Systems | Ch. 5 (pp. 131–166) | HW6: Matrix exponential and ZOH discretization (§5.2–5.3); compare the analytic linearization of Example 5.11 with the numerical one from Week 1 |
| **Wk 7** | *(planned)* | ⬜ Planned | State Feedback — Project 1 Due | Ch. 6 (pp. 167–200) | **PROJECT 1 DUE**. HW7: Reachability tests; pole placement with integral action for the cruise loop (Example 6.10) |
| **Wk 8** | *(planned)* | ⬜ Planned | Output Feedback | Ch. 7 (pp. 201–228) | HW8: Observability; design a state estimator and a discrete Kalman filter; implement the estimator update as a host-tested C module |
| **Wk 9** | *(planned)* | ⬜ Planned | Transfer Functions | Ch. 8 (pp. 229–266) | HW9: Transfer function and Bode plot of the PI cruise loop (Example 8.7); explain the Week 1 tuning result from the loop's frequency response |
| **Wk 10** | *(planned)* | ⬜ Planned | Frequency Domain Analysis | Ch. 9 (pp. 267–292) | HW10: Nyquist plot and gain/phase margins for the Week 1 baseline and tuned designs; relate stability margin to overshoot |
| **Wk 11** | *(planned)* | ⬜ Planned | PID Control — Project 2 Kickoff | Ch. 10 (pp. 293–314) | **PROJECT 2 KICKOFF**. HW11: Integrator windup (§10.4, Examples 10.5–10.6) and computer implementation (§10.5). Write a discrete PI controller in C with anti-windup and setpoint weighting; unit-test it on the host against Octave |
| **Wk 12** | *(planned)* | ⬜ Planned | Frequency Domain Design | Ch. 11 (pp. 315–346) | HW12: Sensitivity functions, feedforward design, and loop shaping for the cruise loop |
| **Wk 13** | *(planned)* | ⬜ Planned | Robust Performance: Uncertainty | Ch. 12 §12.1–12.3 (pp. 347–361) | HW13: Model the mass and gear variation as plant uncertainty; check robust stability of the cruise controller (Examples 12.1, 12.5) |
| **Wk 14** | *(planned)* | ⬜ Planned | Robust Pole Placement and Design | Ch. 12 §12.4–12.6 (pp. 361–376) | HW14: Design for robust performance across the 1000–3000 kg mass range; compare against the Week 1 trial-and-error design |
| **Wk 15** | *(planned)* | ⬜ Planned | Capstone | Review all | **PROJECT 2 DUE** — demo + written report |

---

## Project Specifications

### Project 1 — Cruise Control Model and State-Space Design

**Timeline: Weeks 3–7**

Build a reusable Octave model of the §3.1 cruise control system: the nonlinear vehicle dynamics (equation 3.3) with gear selection, road slope, and throttle saturation, plus numerical linearization about any operating point. Design a state-feedback controller with integral action (Chapter 6) and compare it against the PI controller from Weeks 1 and 3 on step changes in speed and on the hill of Figure 3.3. Deliverables: Octave sources, response plots for 1000–3000 kg, closed-loop pole locations, and a 1–2 page design rationale.

### Project 2 — Embedded Cruise Controller

**Timeline: Weeks 11–15**

Implement the cruise controller as firmware on the NUCLEO-U575ZI-Q. Run a discrete PI controller from a fixed-rate timer interrupt, with anti-windup and setpoint weighting. Close the loop against a discretized vehicle model running on the MCU or on the host over UART (hardware in the loop), stream speed and throttle over UART, and compare the logged response against the Octave simulation. Deliverables: source with CMakeLists.txt, a host unit-test suite for the controller module, measured ISR execution time, Octave-vs-hardware response plots, and a short report.

---

## Exercise Guidelines

- Commit Octave scripts and any C sources with a week README explaining what was implemented and any known issues.
- Run each script from its own directory with `octave --no-gui --quiet <script>.m`. Scripts save their plots as PNG files.
- Before tuning, check that each simulation reproduces the matching book figure. When it does not, find the reason and document it.
- Include a short reflection (3–5 sentences) on what you learned and what was challenging.
- Firmware assignments (Weeks 11–15) follow the toolchain and header conventions in the repository [README](../README.md).

---

## Reference Documents

| Document | Location | Purpose |
| --- | --- | --- |
| Åström & Murray, *Feedback Systems* (v2.10b) | [am08-complete_22Feb09.pdf](https://www.cds.caltech.edu/~murray/books/AM05/pdf/am08-complete_22Feb09.pdf) | Primary textbook |
| FBSwiki — first-edition companion site | [cds.caltech.edu/~murray/amwiki](http://www.cds.caltech.edu/~murray/amwiki/) | Per-chapter exercises, FAQs, errata, and MATLAB sources |
| Companion MATLAB sources | [cds.caltech.edu/~murray/books/AM05/matlab](http://www.cds.caltech.edu/~murray/books/AM05/matlab/) | Original scripts behind the book's figures |

See [ECE-340_References.md](ECE-340_References.md) for the complete list.
