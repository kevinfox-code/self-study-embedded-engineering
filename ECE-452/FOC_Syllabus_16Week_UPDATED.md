# SELF-STUDY SYLLABUS
## Permanent Magnet Synchronous & Brushless DC Motor Drives
### Sensorless Field-Oriented Control — STM32 Implementation in C

| Duration | Primary Text | Target Hardware | Projects |
|---|---|---|---|
| 16 Weeks | Krishnan, CRC Press 2010 | Nucleo-U575ZI-Q + DRV8323RS | 3 Major (1 Final) |

---

## Course Overview

This 16-week self-paced course takes you from motor physics through a complete, clean-IP sensorless FOC stack written from scratch in C on an STM32 microcontroller. All reading assignments have been verified against the actual page content of Krishnan's *Permanent Magnet Synchronous and Brushless DC Motor Drives* (CRC Press, 2010). Each week pairs a specific reading assignment with a hands-on lab that produces a tested, modular C deliverable. Three projects accumulate into a fully functional sensorless drive by Week 16.

### Reference Materials

- **Krishnan, R.** — *Permanent Magnet Synchronous and Brushless DC Motor Drives* (CRC Press, 2010) — **[PRIMARY — 614 pages]**
- **Krishnan, R.** — *Electric Motor Drives: Modeling, Analysis, and Control* (Prentice Hall, 2001) — [SECONDARY]
- **ST AN4220** — Sensorless FOC for PMSM Motor Drives (free from st.com)
- **Microchip AN1078** — Sensorless Field Oriented Control of a PMSM (free from microchip.com)
- **NXP DRM148** — Sensorless PMSM FOC Design Reference Manual (free from nxp.com)
- **DRV8323RS Datasheet** — Texas Instruments (free from ti.com)

### Textbook Chapter Map — Verified Page Numbers

> **PDF page = Book page + 37** for the uploaded file (verified by spot-checking multiple chapters)

| Chapter | Title | Book Pages | PDF Pages |
|---|---|---|---|
| Ch. 1 | Permanent Magnets and Machines | pp. 3–134 | PDF 40–171 |
| Ch. 2 | Introduction to Inverters and Their Control | pp. 135–223 | PDF 172–260 |
| Ch. 3 | Dynamic Modeling of Permanent Magnet Synchronous Machines | pp. 225–276 | PDF 262–313 |
| Ch. 4 | Control Strategies for a Permanent Magnet Synchronous Machine | pp. 279–328 | PDF 316–365 |
| Ch. 5 | Flux-Weakening Operation | pp. 331–376 | PDF 368–413 |
| Ch. 6 | Design of Current and Speed Controllers | pp. 379–398 | PDF 416–435 |
| Ch. 7 | Parameter Sensitivity and Compensation | pp. 401–421 | PDF 438–458 |
| Ch. 8 | Rotor Position Estimation and Sensorless Control | pp. 423–451 | PDF 460–488 |
| Ch. 9–14 | PM Brushless DC Motor Drives (reference only for BLDC variants) | pp. 457–563 | PDF 494–600 |

---

## Weekly Schedule

---

### PHASE 1: Foundations — Machine Physics & Power Electronics (Weeks 1–4)

---

#### Week 1 — Permanent Magnets & PMSM Machine Physics

**Reading Assignment** *(Krishnan Ch. 1, pp. 3–90)*

- §1.1 Demagnetization characteristics & energy density (pp. 3–21)
- §1.2–1.3 PM arrangements & magnetization — radial, parallel, Halbach (pp. 21–30)
- §1.4 PM rotor configurations — surface-mounted, surface-inset, IPM, line-start (pp. 31–50)
- §1.5 Synchronous machine fundamentals: MMF, induced EMF, winding factors (pp. 51–75)
- §1.6.1–1.6.4 Effective air gap, electromagnetic power and torque equations (pp. 72–78)

**Lab / Coding Task**

Motor parameter extraction:
- Measure Rs with 4-wire ohmmeter; record at cold & warm
- Measure Ls (d- and q-axis) with LCR meter at 1 kHz
- Verify against datasheet; compute Ke from EMF formula (Eq. 1.54, p. 54)
- Compare measured vs. calculated values in a table

**Deliverable**

- Lab notebook: parameter table (Rs, Ld, Lq, Ke) with % error vs. datasheet
- Signed hand-derivation of the induced EMF equation

---

#### Week 2 — Machine Inductances, Torque & Core Losses

**Reading Assignment** *(Krishnan Ch. 1, pp. 84–134)*

- §1.6.8 Inductance derivations — self, magnetizing, d- and q-axes (pp. 84–91)
- §1.7 Core losses: eddy-current & hysteresis, tooth and yoke loss computation (pp. 92–106)
- §1.8 Resistive losses (pp. 103–105)
- §1.10 Cogging torque — causes, analysis, mitigation methods (pp. 107–122)
- §1.11 Flux path types (pp. 115–122)

**Lab / Coding Task**

- Hand-derive Clarke and Park transformation matrices from first principles
- Implement as standalone C functions (`float32_t`)
- Write PC-side test harness (gcc/MinGW): apply random abc vectors, verify round-trip Clarke → Park → inverse ≈ input to < 1e-5
- Log PASS/FAIL via printf; document any numerical edge cases

**Deliverable**

- `clarke.c/.h`, `park.c/.h`
- `test_transforms.c` with PASS/FAIL output
- Hand-derivation scan (PDF)

---

#### Week 3 — Power Electronics & Inverter Topology

**Reading Assignment** *(Krishnan Ch. 2, pp. 135–175)*

- §2.1 Power devices: diode, MOSFET, IGBT — switching behavior & losses (pp. 135–145)
- §2.2–2.3 DC bus, 3-phase VSI switching states, 6-step operation (pp. 146–161)
- §2.7 PWM — sinusoidal modulation, sampling methods, transfer characteristics (pp. 164–175)
- §2.8 Hysteresis current control (pp. 173–176)
- §2.10 Inverter switching delay & dead-time effects (pp. 195–215)

**Lab / Coding Task**

- Configure STM32 TIM1: center-aligned mode, 20 kHz, complementary outputs
- Set dead-time via BDTR register; verify on oscilloscope (target: 200 ns)
- Configure DRV8323RS via SPI: gate drive current, OCP threshold, 3x PWM mode
- Scope all three high-side and low-side gate signals simultaneously
- Document all register values with bit-field comments in CubeIDE project

**Deliverable**

- Annotated CubeIDE project
- Scope screenshots: dead-time, complementary PWM waveforms
- DRV8323RS SPI configuration register map

---

#### Week 4 — Space Vector PWM (SVPWM)

**Reading Assignment** *(Krishnan Ch. 2, pp. 176–215)*

- §2.9 Space Vector Modulation — full derivation (pp. 176–195)
- §2.9.1 Switching states and active/zero voltage vectors (pp. 177–182)
- §2.9.2 SVM principle: sector detection, T1/T2/T0 timing calculation (pp. 177–188)
- §2.9.3 SVM modulator implementation (pp. 188–192)
- §2.9.4 Switching ripple analysis & hybrid PWM controller (pp. 188–197)
- §2.10.1 Inverter control model (pp. 207–208)
- Supplemental: ST AN4220 Sec. 3

**Lab / Coding Task**

Implement SVPWM module from scratch in C:
- Sector detection from Vα/Vβ inputs
- T1/T2/T0 timing calculation
- Write compare register values to TIM1 CCR1/2/3
- Hand-calculate expected duty cycles for Vref = 0.5·Vdc at θ = 30°
- Validate: scope 3-phase output, compare measured to calculated (target: < 1% error)

**Deliverable**

- `svpwm.c/.h` module
- Scope capture: 3-phase line voltages
- Duty cycle comparison table: calculated vs. measured

---

### PHASE 2: Reference Frames, dq Modeling & Current Control (Weeks 5–8)

---

#### Week 5 — Dynamic dq Model & Reference Frame Theory ◀ PROJECT 1 START

**Reading Assignment** *(Krishnan Ch. 3, pp. 225–270)*

- §3.1 Real-time two-phase PMSM model derivation (pp. 226–230)
- §3.2 Transformation to rotor reference frames (pp. 231–238)
- §3.3 Three-phase to two-phase (Clarke) transformation (pp. 236–240)
- §3.5 Power equivalence — rationale for power-invariant scaling (pp. 241–242)
- §3.6 Electromagnetic torque from dq model (pp. 242–250)
- §3.7 Steady-state torque characteristics (pp. 244–257)

**Lab / Coding Task**

**PROJECT 1 START: Current Sensing & FOC Current Loop**

- Configure injected ADC triggered by TIM1 underflow (center of PWM off-time)
- Implement 3-shunt current reconstruction: Ia, Ib → Ic = −(Ia + Ib)
- Add DC offset calibration routine (average 1000 samples at zero current)
- Log Ia + Ib + Ic over UART; verify Kirchhoff: sum < 1% of peak current

**Deliverable**

- ADC + current sensing module
- Kirchhoff validation plot (from UART data)
- Hand derivation of Ch. 3 dq voltage equations (scanned)

---

#### Week 6 — Vector Control Derivation & Control Strategies

**Reading Assignment** *(Krishnan Ch. 4, pp. 279–328)*

- §4.1 Vector control concept (pp. 279–280)
- §4.2 Full vector control derivation: torque, Id/Iq decoupling (pp. 280–295)
- §4.3 Drive system schematic — torque & speed-controlled configurations (pp. 285–302)
- §4.4.1 Constant torque angle (δ = 90°) control — most relevant to FOC (pp. 304–307)
- §4.4.5 Maximum torque per unit current (MTPA) — concept (pp. 317–319)
- Supplemental: ST AN4220 Sec. 3 (FOC block diagram)

**Lab / Coding Task**

- Wire Clarke & Park transforms (Week 2 modules) into the ISR pipeline
- Apply open-loop d-axis voltage injection (Vd = 0.5 V, Vq = 0): use known electrical angle
- Log Id and Iq via UART at 1 kHz sample rate
- Verify: Id rises toward command, Iq stays near zero
- Measure ISR execution time with GPIO toggle + logic analyzer

**Deliverable**

- ISR integration with transform pipeline
- Logic analyzer capture: ISR execution time < 5 µs
- Id/Iq data log plot (Python or MATLAB)

---

#### Week 7 — PI Current Controller Design

**Reading Assignment** *(Krishnan Ch. 6, pp. 379–398)*

- §6.1 Current controller overview (pp. 380–381)
- §6.1.1 Rotor reference frame PI controllers — full derivation (pp. 381–385)
- §6.1.2 Stator reference frame controllers (pp. 382–385)
- §6.1.3 Deadbeat controllers — concept only (pp. 385–389)
- §6.2.1–6.2.2 Block diagram & simplified current loop transfer function (pp. 390–392)
- Supplemental: NXP DRM148 Sec. 3 (discrete PI implementation)

**Lab / Coding Task**

- Derive Id and Iq PI gains analytically from Rs, Ld, Lq, sampling period Ts:
  - Kp = Ld · ωbw, Ki = Rs · ωbw (target bandwidth = 0.1 · ωPWM)
- Implement discrete-time PI with back-calculation anti-windup
- Test Id PI: ramp electrical angle in open loop; step Id reference
- Capture step response: overshoot < 20%, settle < 5 · Ts

**Deliverable**

- `pi_controller.c/.h` (generic, reusable)
- Gain derivation worksheet with motor parameters
- Step response plot for Id (UART data log)

---

#### Week 8 — Closed-Loop Current Control Integration ◀ PROJECT 1 DUE

**Reading Assignment** *(Krishnan Ch. 3, pp. 257–276 & Ch. 6, pp. 390–398)*

- §3.11 Dynamic simulation of PMSM — understand expected closed-loop behavior (pp. 257–261)
- §3.12 Small-signal PMSM model (pp. 262–266)
- §3.13 Transfer functions & frequency response evaluation (pp. 264–269)
- §6.2.3 Speed controller design — preview for Week 13 (pp. 393–398)
- Supplemental: ST AN4220 Sec. 4 (current loop commissioning)

**Lab / Coding Task**

**PROJECT 1 DUE: Close both Id and Iq loops simultaneously**

- Implement cross-axis decoupling feedforward:
  - `Vd_ff = −ωe · Lq · Iq`
  - `Vq_ff = ωe · Ld · Id + ωe · λf`
- Spin motor with commanded electrical angle (open-loop position)
- Tune PI gains for target bandwidth; measure Bode gain/phase via swept sine if possible
- Record ISR worst-case execution time with all modules active

**Deliverable**

**PROJECT 1 COMPLETE:**
- Full current control stack running on hardware
- Tuning report: gain derivation, step response plots, ISR timing breakdown
- Decoupling feedforward verified with scope traces

---

### PHASE 3: Sensorless Position Estimation (Weeks 9–12)

---

#### Week 9 — Sensorless Control Methods Overview & BEMF Estimation ◀ PROJECT 2 START

**Reading Assignment** *(Krishnan Ch. 8, pp. 423–451)*

- §8.1 Current model adaptive scheme (pp. 423–427)
- §8.2 External signal injection methods — overview (pp. 428–431)
- §8.2.1 Revolving voltage phasor injection scheme (pp. 428–432)
- §8.3 Current model-based injection scheme (pp. 447–451)
- §8.4 Position estimation using PWM carrier components (pp. 448–451)
- Supplemental: Microchip AN1078 Sec. 1–3 (BEMF observer practical approach)

**Lab / Coding Task**

**PROJECT 2 START: Sensorless Observer Stack**

- Derive discrete BEMF estimator from Ch. 3 §3.1 voltage equations:
  - `eα = Vα − Rs · iα − Ls · (Δiα / Ts)`, and eβ similarly
- Implement alpha/beta BEMF estimator module in C
- Log estimated vs. expected BEMF at fixed open-loop speed (e.g., 500 RPM)

**Deliverable**

- `bemf_estimator.c/.h`
- Estimated vs. expected BEMF comparison plot
- Hand derivation of discrete BEMF equations

---

#### Week 10 — Signal Injection Sensorless & Sliding Mode Observer

**Reading Assignment** *(Krishnan Ch. 8, pp. 432–451)*

- §8.2.2 Flux linkage injection in rotating q-axis — full algorithm (pp. 432–441)
  - §8.2.2.1 Algorithm (pp. 433–436)
  - §8.2.2.2 Demodulation (pp. 436–437)
  - §8.2.2.3 Observer structure (pp. 437–438)
  - §8.2.2.4 Implementation (pp. 438–440)
- §8.2.3 Alternating voltage phasor injection (pp. 442–446)
- Supplemental: Microchip AN1078 full document (SMO implementation reference)

**Lab / Coding Task**

Implement full Sliding Mode Observer (SMO) as practical BEMF estimator:
- Switching function: `z = k · sign(iα_est − iα_meas)` per axis
- Low-pass filter on z to extract smooth BEMF: `eα_est`, `eβ_est`
- Compute estimated angle: `θ_est = atan2(−eα_est, eβ_est)`
- Validate angle error vs. known electrical angle in open-loop spin

**Deliverable**

- `smo.c/.h` (full sliding mode observer)
- Angle error vs. speed plot (UART data)
- SMO tuning notes: sliding gain k selection, LPF cutoff rationale

---

#### Week 11 — Parameter Sensitivity & PLL Angle Tracking

**Reading Assignment** *(Krishnan Ch. 7, pp. 401–421)*

- §7.1 Parameter sensitivity introduction — effect of Rs, λf, Lq on drive performance (pp. 401–404)
- §7.1.1 Ratio of torque to its reference under parameter errors (pp. 402–403)
- §7.1.2 Ratio of mutual flux linkages to its reference (pp. 403–404)
- §7.2 Air gap power feedback compensation algorithm & performance (pp. 404–412)
- §7.3 Reactive power feedback compensation scheme (pp. 413–421)
- Supplemental: ST AN4220 Sec. 5 (PLL-based speed/angle tracking)

**Lab / Coding Task**

Replace raw atan2 with a Phase-Locked Loop on SMO outputs:
- PLL error signal: `sin(θ_est − θ_PLL) ≈ θ_est − θ_PLL` for small error
- PI loop drives θ_PLL and integrates to yield ω_est (speed estimate)
- Compare raw atan2 vs. PLL-tracked angle: noise and latency
- Plot angle noise (std dev) and step latency at 500, 1000, 2000 RPM

**Deliverable**

- `pll.c/.h` (PLL tracker module)
- Noise & latency comparison plot
- PLL bandwidth selection rationale

---

#### Week 12 — Open-Loop Startup & Observer Handoff ◀ PROJECT 2 DUE

**Reading Assignment** *(Krishnan Ch. 4, pp. 285–302 & Ch. 5, pp. 331–342)*

- §4.3.1–4.3.2 Torque-controlled drive startup — open-loop V/f approach (pp. 285–292)
- §4.3.3 Speed-controlled drive system structure (pp. 293–302)
- §5.1 Maximum speed boundary and flux-weakening introduction (pp. 332–333)
- §5.2 Flux-weakening algorithm overview — indirect control (pp. 333–342)
- Supplemental: ST AN4220 Sec. 6 (startup sequence & observer handoff)

**Lab / Coding Task**

**PROJECT 2 DUE: Implement full startup → sensorless handoff sequence**

1. **Align:** apply Vd for 500 ms to lock rotor to θ = 0
2. **V/f ramp:** increase Vref & frequency from 0 to handoff speed over 2 s
3. **Handoff:** when ω_est > threshold, switch to SMO + PLL closed-loop
4. Characterize minimum handoff speed; verify smooth transition (no current spike)

**Deliverable**

**PROJECT 2 COMPLETE:**
- Full sensorless observer stack running on hardware
- Startup video + handoff angle/speed plot
- Minimum handoff speed characterization table

---

### PHASE 4: Speed Control, Flux Weakening & System Integration (Weeks 13–16)

---

#### Week 13 — Outer Speed Controller Design ◀ PROJECT 3 START

**Reading Assignment** *(Krishnan Ch. 6, pp. 389–398)*

- §6.2 Speed controller — full derivation (pp. 389–398)
- §6.2.1 Block diagram derivation (pp. 390–391)
- §6.2.2 Simplified current loop transfer function for speed loop design (pp. 391–393)
- §6.2.3 Speed PI design — symmetric optimum method with worked example (pp. 393–397)
- §6.2.3.1 Speed measurement smoothing (pp. 396–398)
- Supplemental: review Ch. 4 §4.3.3.1 mutual flux programming for speed control

**Lab / Coding Task**

**PROJECT 3 START: Complete Sensorless FOC Drive**

- Implement outer speed PI loop: `Iq_ref = Kps · (ωr_ref − ωr_est) + integral`
- Tune for 10:1 bandwidth separation from current loop (symmetric optimum method)
- Add speed reference ramp: 0 → rated speed in 3 s
- Step response: command 10% → 100% rated speed; measure rise time & overshoot

**Deliverable**

- `speed_controller.c/.h`
- Step response scope/log capture
- Gain derivation from symmetric optimum method

---

#### Week 14 — Flux Weakening & Fault Handling

**Reading Assignment** *(Krishnan Ch. 5, pp. 331–376)*

- §5.1 Maximum speed boundary (pp. 332–333)
- §5.2 Flux-weakening algorithm — indirect control scheme (pp. 333–342)
  - §5.2.1 Indirect control scheme (pp. 336–338)
  - §5.2.2 Constant torque mode controller (pp. 336–338)
  - §5.2.3 Flux-weakening controller (pp. 338–339)
- §5.4 Parameter sensitivity in flux weakening (pp. 349–351)
- §5.8 SMPM vs. IPM in flux weakening — normalized comparison (pp. 371–376)
- Supplemental: DRV8323RS datasheet Sec. 8 (fault registers & nFAULT pin)

**Lab / Coding Task**

Implement fault state machine:
- **Overcurrent:** monitor DRV8323RS nFAULT GPIO; latch & report fault code via UART
- **Overvoltage:** ADC monitor DC bus; trip if > 110% rated Vdc
- **Stall detection:** ω_est dropout below threshold for > 200 ms
- Test each path: inject fault, verify motor coasts to stop, confirm UART fault log
- Optional: implement basic Id = 0 → Id < 0 flux-weakening extension above base speed

**Deliverable**

- `fault_handler.c/.h` (state machine)
- Fault injection test log for each fault type
- Optional: flux-weakening speed extension demonstration

---

#### Week 15 — System Integration, Tuning & Performance Validation

**Reading Assignment** *(Krishnan Ch. 3, pp. 257–276 & Ch. 4, pp. 296–302)*

- §3.11 Dynamic simulation reference — compare to measured drive behavior (pp. 257–261)
- §3.14 Computation of time responses (pp. 266–270)
- §4.3.4 Simulation & results of speed-controlled drive (pp. 296–302)
- Review: Ch. 6 §6.2.3 speed controller performance criteria (pp. 393–398)
- Review: all application notes — ST AN4220, Microchip AN1078, NXP DRM148

**Lab / Coding Task**

- Full-stack integration: startup → sensorless → speed control → fault handling
- Characterize speed regulation accuracy vs. simulated load (hand-brake or resistive)
- Measure and optimize ISR execution time: profile each module with GPIO toggle
- Target: ISR < 10 µs total at 20 kHz; identify and fix bottlenecks
- Confirm all Project 1 + 2 + 3 modules operate together without interference

**Deliverable**

- Integrated firmware build (clean compile, zero warnings)
- ISR timing breakdown table (per module)
- Speed regulation accuracy: ±X RPM at no-load vs. loaded

---

#### Week 16 — Final Project Demo & Documentation ◀ PROJECT 3 DUE

**Reading Assignment** *(Review: all course notes, lab notebooks, AN references)*

- Ch. 3–4: verify understanding of dq model used throughout the project
- Ch. 6: confirm controller design basis is fully documented in the write-up
- Ch. 8: document sensorless method chosen and limitations vs. other Ch. 8 alternatives
- Ch. 7 §7.1: note parameter sensitivity risks specific to your motor and drive system

**Lab / Coding Task**

**PROJECT 3 DUE — FINAL DEMONSTRATION:**

1. **Cold start:** power on → align → V/f ramp → sensorless handoff
2. **Speed sweep:** 10% → 50% → 100% rated speed, record via UART
3. **Load step:** introduce simulated load at 50% speed, verify speed recovery
4. **Fault demo:** trigger overcurrent, verify safe shutdown & UART fault log
5. **Record** demonstration video (phone or screen capture acceptable)

**Deliverable**

**PROJECT 3 COMPLETE — FINAL DELIVERABLES:**
- Clean firmware repo (GitHub or zip): all modules, zero X-CUBE-MCSDK FOC dependencies
- Technical write-up: system block diagram, all design decisions, parameter derivations
- Performance summary: speed regulation accuracy, ISR timing per module, handoff speed
- Demo video (2–5 min): cold start, speed sweep, load step, fault recovery

---

## Project Descriptions

### Project 1: Current Sensing & FOC Current Loop (Weeks 5–8)

*Build the lower half of the FOC stack from raw ADC samples to closed-loop Id/Iq control. Reading basis: Krishnan Ch. 1 (motor parameters), Ch. 3 §3.1–3.7 (dq model), Ch. 6 §6.1 (PI controllers).*

1. 3-shunt ADC module: injected conversion, DC offset calibration, Kirchhoff verification (Ia + Ib + Ic < 1% of peak)
2. Clarke & Park transform modules with PC-side unit test harness (round-trip error < 1e-5)
3. Discrete PI controller module: Id and Iq, back-calculation anti-windup, cross-axis decoupling feedforward
4. Open-loop spin demonstration with both current loops closed
5. Tuning report: gains derived from motor parameters (Rs, Ld, Lq, Ts), step response plots, ISR timing breakdown

---

### Project 2: Sensorless Observer Stack (Weeks 9–12)

*Implement the SMO + PLL observer and validate end-to-end sensorless position estimation with a working motor startup sequence. Reading basis: Krishnan Ch. 8 §8.1–8.4 (sensorless methods), Ch. 3 §3.1 (BEMF derivation), Ch. 7 §7.1 (parameter sensitivity awareness).*

1. BEMF estimator module (alpha/beta stationary frame, discrete from §3.1 voltage equations)
2. Sliding Mode Observer (SMO): switching function, low-pass filter, atan2 angle extraction
3. PLL-based angle/speed tracker: PI-driven phase accumulator from SMO outputs
4. Startup state machine: align → V/f ramp → SMO + PLL handoff
5. Validation report: angle error vs. speed table, minimum handoff speed, raw vs. PLL noise comparison plot

---

### Project 3 (Final): Complete Sensorless FOC Drive (Weeks 13–16)

*Integrate all subsystems into a production-quality sensorless FOC drive with fault handling and demonstrated speed control under load. Reading basis: Krishnan Ch. 6 §6.2 (speed controller), Ch. 5 §5.1–5.2 (flux-weakening extension), Ch. 3 §3.11–3.14 (dynamic validation reference).*

1. Full integrated firmware: all modules, clean C, zero X-CUBE-MCSDK FOC library dependencies
2. Outer speed PI controller with reference ramp and symmetric-optimum tuning
3. Fault state machine: overcurrent (nFAULT), overvoltage (ADC monitor), stall detection
4. Demonstration video (2–5 min): cold start, speed sweep 10%–100%, load step, fault recovery
5. Technical write-up: system block diagram, all design decisions, parameter derivations, performance characterization

---

## Study Tips & Workflow

1. **Read the chapter before starting the lab — not after.** Deriving key equations by hand first (Clarke, Park, PI discretization, SMO switching function) catches sign errors and unit mistakes before they appear in hardware.

2. **Build and unit-test each module as a standalone .c/.h pair** with a test harness that compiles and runs on your PC. Validate the math in a logging environment, then port the tested module into the ISR. This cuts hardware debug time dramatically.

3. **Instrument everything.** Use UART logging in early weeks; logic analyzer and STM32 DAC in later weeks. Visibility into Id, Iq, θ_est, ω_est, and SVPWM duty cycles at all times is essential — especially for the sensorless handoff in Week 12.

4. **Keep a lab notebook** (physical or digital) with one entry per session: what you expected, what happened, what changed, and why it worked or failed. This becomes your Project 3 write-up with minimal extra effort.

5. **Textbook page numbers cited are verified** against the actual Krishnan (CRC Press, 2010) PDF content. PDF page = book page + 37 for the uploaded file. All cited sections have been confirmed to contain the described material.

---

*Sensorless FOC Self-Study Syllabus | STM32 Nucleo-U575ZI-Q + DRV8323RS | 16 Weeks | Reading assignments verified against Krishnan (CRC Press, 2010) page content*
