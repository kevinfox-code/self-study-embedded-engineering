# ECE-452 Week 04 — Space Vector PWM

**Reading:** Krishnan — inverter modulation and space-vector theory.

**Lab deliverable:** symmetrical seven-segment SVPWM duty-cycle calculation in C,
with a hand-calculated known-answer case and a host test harness.

## What Was Built

- [`svpwm.h`](svpwm.h) / [`svpwm.c`](svpwm.c) — computes seven-segment SVPWM duty
  cycles from a stationary-frame reference vector (Vα, Vβ) and the DC bus
  voltage. Returns per-phase duties, the T1/T2/T0 dwell fractions, and the sector
  number in an `SVPWM_Output` struct, and can write the corresponding CCR values
  for STM32 TIM1.
- [`test_svpwm.c`](test_svpwm.c) — known-answer tests (including the
  hand-calculated θ = 30° deliverable), a six-sector sweep, and an invariant
  sweep over 360 angles × 3 amplitudes checking T1 + T2 + T0 = 1.
- [`Makefile`](Makefile) — `make run` builds and executes the harness on the host.

## Key Concepts

- **Linear modulation limit.** `|Vref| ≤ Vdc / √3 ≈ 0.577 · Vdc`. Above it the
  requested vector cannot be synthesised and the duties clip.
- **Seven-segment symmetry.** Splitting the zero vector evenly between V0 and V7
  centres the active pulses in the switching period, which is what keeps the
  current ripple symmetric and the harmonic content low.
- **Sector determination drives everything.** T1 and T2 are the dwell times of
  the two adjacent active vectors bounding the sector, so a sector-boundary error
  shows up as a phase-current glitch, not as a wrong average.
- **`float32_t` matches CMSIS-DSP.** The typedef is plain `float` on the host so
  the same source compiles for the target without edits.

## Build and Run

```bash
cd ECE-452/Week04-SVPWM
make run
```

`make clean` removes the host executable, which is git-ignored.

## References

- Krishnan, *Permanent Magnet Synchronous and Brushless DC Motor Drives* —
  inverter modulation
- AN4013 for STM32 center-aligned PWM with complementary outputs
- See [ECE-452_References.md](../ECE-452_References.md) for download links

## AI Assistance

None.
