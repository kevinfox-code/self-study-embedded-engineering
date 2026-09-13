# ECE-452 Week 02 — Clarke and Park Transforms

**Reading:** Krishnan Ch. 1, pp. 84–134 (§1.6.8, §1.7, §1.8, §1.10, §1.11) —
machine inductances, core and resistive losses, and cogging torque.

**Lab deliverable:** amplitude-invariant Clarke and Park transforms in C, with a
hand derivation and a test harness.

---

## Homework Notes

### §1.6.8 — Inductance Derivations

**Self-inductance** of a single winding arises from its own flux linking its own turns. For a sinusoidally distributed winding:

$$
L_{aa} = L_0 + L_2 \cos(2\theta_e)
$$

where $L_0$ is the average component and $L_2$ the position-dependent saliency term from rotor geometry. For a surface-mounted PMSM (SPM) with negligible saliency, $L_2 \approx 0$ and $L_{aa} \approx L_s$ (constant).

**d-axis inductance** ($L_d$) is the inductance seen when current flows along the axis aligned with the rotor magnet flux. It tends to be lower than $L_q$ in an SPM because the magnet's relative permeability ($\mu_r \approx 1.05$) acts like an extended air gap in the d-axis flux path.

**q-axis inductance** ($L_q$) is the inductance seen when current flows in quadrature to the magnet flux (the torque-producing axis). For an SPM, $L_d \approx L_q$. For an IPM (interior PM), $L_q > L_d$ because the q-axis flux path passes through iron (low reluctance) while the d-axis path crosses the buried magnet.

### §1.7 — Core Losses

Core losses split into two components:

$$
P_{core} = P_{eddy} + P_{hys}
$$

$$
P_{eddy} = k_e \, f^2 \, B_{pk}^2, \qquad P_{hys} = k_h \, f \, B_{pk}^n
$$

where $k_e$, $k_h$ are material constants, $f$ is electrical frequency, $B_{pk}$ peak flux density, and $n \approx 2$ (Steinmetz exponent). Tooth flux density is higher than yoke flux density (flux concentrates in teeth), so tooth losses dominate at high speed.

### §1.8 — Resistive Losses

$$
P_{cu} = 3 \, I_{ph}^2 \, R_s
$$

DC resistance $R_s$ is measured at standstill; effective AC resistance rises with frequency due to skin effect and proximity effect in the winding conductors (relevant above ~1 kHz electrical).

### §1.10 — Cogging Torque

Cogging torque is the ripple torque produced even with zero stator current, caused by the interaction between PM flux and the stator slot geometry. Magnitude depends on the ratio of slot-pitch to pole-pitch. Mitigation methods include:
- **Skewing** stator slots or rotor magnets by one slot pitch
- **Fractional-slot windings** — choosing slot/pole combinations with no common factor
- **Chamfered or segmented magnets** to smooth the permeance variation

---

## Lab Deliverable — Clarke & Park Transforms

### Hand Derivation: Clarke Transformation (abc → αβ)

**Objective:** Map a 3-phase abc vector to an equivalent 2-phase αβ representation that preserves amplitudes (amplitude-invariant form).

**Setup.** Consider three balanced windings displaced 120° apart. A current $i_a$ produces a stator MMF along the a-axis. We want to find two-phase equivalents $i_\alpha$ (along a-axis) and $i_\beta$ (perpendicular, 90° ahead).

**Step 1 — Project each phase onto α and β axes.**

The unit vectors along the three phase axes are:
$$
\hat{a} = (1, 0), \quad
\hat{b} = \!\left(-\tfrac{1}{2},\,\tfrac{\sqrt{3}}{2}\right), \quad
\hat{c} = \!\left(-\tfrac{1}{2},-\tfrac{\sqrt{3}}{2}\right)
$$

The total MMF vector in αβ:
$$
\vec{F} = i_a\,\hat{a} + i_b\,\hat{b} + i_c\,\hat{c}
$$
$$
F_\alpha = i_a(1) + i_b\!\left(-\tfrac{1}{2}\right) + i_c\!\left(-\tfrac{1}{2}\right)
= i_a - \tfrac{i_b}{2} - \tfrac{i_c}{2}
$$
$$
F_\beta = i_a(0) + i_b\!\left(\tfrac{\sqrt{3}}{2}\right) + i_c\!\left(-\tfrac{\sqrt{3}}{2}\right)
= \tfrac{\sqrt{3}}{2}\,i_b - \tfrac{\sqrt{3}}{2}\,i_c
$$

**Step 2 — Apply the amplitude-invariant scaling factor.**

A 3-phase system produces a peak MMF of $\tfrac{3}{2}N i_{pk}$ in the rotating direction, while a 2-phase system produces $N i_{pk}$. To equate peak amplitudes of the αβ currents to the 3-phase peak, multiply by $\tfrac{2}{3}$:

$$
\boxed{
i_\alpha = \frac{2}{3}\!\left(i_a - \frac{i_b}{2} - \frac{i_c}{2}\right)
}
$$
$$
\boxed{
i_\beta = \frac{2}{3}\!\left(\frac{\sqrt{3}}{2}\,i_b - \frac{\sqrt{3}}{2}\,i_c\right)
}
$$

**Step 3 — Zero-sequence component** (captures common-mode, does not affect torque):
$$
\boxed{i_0 = \frac{1}{3}(i_a + i_b + i_c)}
$$

**Inverse Clarke.** Solve for abc given αβ0. The transformation matrix is 3×3 and invertible:

$$
T^{-1} =
\begin{bmatrix}
 1         &  0                & 1 \\
-\tfrac{1}{2} &  \tfrac{\sqrt{3}}{2} & 1 \\
-\tfrac{1}{2} & -\tfrac{\sqrt{3}}{2} & 1
\end{bmatrix}
$$

Applying:
$$
i_a =  i_\alpha + i_0
$$
$$
i_b = -\tfrac{1}{2}\,i_\alpha + \tfrac{\sqrt{3}}{2}\,i_\beta + i_0
$$
$$
i_c = -\tfrac{1}{2}\,i_\alpha - \tfrac{\sqrt{3}}{2}\,i_\beta + i_0
$$

**Verification (balanced, θ=0):** $i_a=1,\; i_b=-\tfrac{1}{2},\; i_c=-\tfrac{1}{2}$

$$
i_\alpha = \tfrac{2}{3}\!\left(1 + \tfrac{1}{4} + \tfrac{1}{4}\right) = \tfrac{2}{3}\cdot\tfrac{3}{2} = 1 \;\checkmark
$$
$$
i_\beta = \tfrac{2}{3}\!\left(\tfrac{\sqrt{3}}{2}\cdot(-\tfrac{1}{2}) - \tfrac{\sqrt{3}}{2}\cdot(-\tfrac{1}{2})\right) = 0 \;\checkmark
$$

---

### Hand Derivation: Park Transformation (αβ → dq)

**Objective:** Rotate the stationary αβ frame into the synchronously rotating dq frame aligned with the rotor flux at electrical angle $\theta_e$.

**Step 1 — Geometry of the rotation.**

The d-axis is defined as the axis of the rotor magnet flux, at angle $\theta_e$ from the stator α-axis. A standard 2D rotation by $-\theta_e$ projects the stationary αβ vector onto the rotating dq axes:

$$
\begin{bmatrix} i_d \\ i_q \end{bmatrix}
=
\begin{bmatrix}  \cos\theta_e & \sin\theta_e \\
                -\sin\theta_e & \cos\theta_e \end{bmatrix}
\begin{bmatrix} i_\alpha \\ i_\beta \end{bmatrix}
$$

$$
\boxed{i_d =  i_\alpha\cos\theta_e + i_\beta\sin\theta_e}
$$
$$
\boxed{i_q = -i_\alpha\sin\theta_e + i_\beta\cos\theta_e}
$$

The rotation matrix $P(\theta_e)$ is orthogonal: $P^T = P^{-1}$. The inverse Park transform is simply the transpose:

$$
\begin{bmatrix} i_\alpha \\ i_\beta \end{bmatrix}
=
\begin{bmatrix} \cos\theta_e & -\sin\theta_e \\
                \sin\theta_e &  \cos\theta_e \end{bmatrix}
\begin{bmatrix} i_d \\ i_q \end{bmatrix}
$$

$$
\boxed{i_\alpha = i_d\cos\theta_e - i_q\sin\theta_e}
$$
$$
\boxed{i_\beta  = i_d\sin\theta_e + i_q\cos\theta_e}
$$

**Step 2 — Physical interpretation.**

In steady state, balanced 3-phase sinusoidal currents appear as a constant DC vector in the dq frame:
- $i_d$ controls flux (aligned with magnet flux)
- $i_q$ controls torque (in quadrature with magnet flux)

This is why dq control (FOC) replaces AC current regulation with simpler DC PI loops.

**Verification:** $i_\alpha=1,\; i_\beta=0,\; \theta_e=90°$

$$
i_d = 1\cdot\cos 90° + 0\cdot\sin 90° = 0 \;\checkmark
$$
$$
i_q = -1\cdot\sin 90° + 0\cdot\cos 90° = -1 \;\checkmark
$$

(At θ=90°, the d-axis has rotated 90° away from α, so the α-component projects entirely onto the negative q-axis.)

---

## Test Results

```
=== Clarke & Park Transform Test Suite ===

--- Known-Answer: Clarke ---
PASS  KA-Clarke-1: balanced abc at theta=0  -> alpha=1.0000 beta=0.0000 zero=0.0000
PASS  KA-Clarke-2: balanced abc at theta=90 -> alpha=0.0000 beta=1.0000 zero=0.0000
PASS  KA-Clarke-3: pure zero-sequence       -> alpha=0.0000 beta=0.0000 zero=1.0000

--- Known-Answer: Park ---
PASS  KA-Park-1: alpha=1,beta=0,theta=0   -> d=1.0000 q=0.0000
PASS  KA-Park-2: alpha=0,beta=1,theta=0   -> d=0.0000 q=1.0000
PASS  KA-Park-3: alpha=1,beta=0,theta=90  -> d=-0.0000 q=-1.0000

--- Edge Cases ---
PASS  EC-Zero: zero input round-trips correctly
PASS  EC-2pi:  theta=2π equivalent to theta=0
PASS  EC-Large: amplitude 1000 round-trips (alpha=1000.0000)

--- Round-Trip Random ---
Round-trip random test (1000 vectors, tol=1e-05):
  PASS: 1000   FAIL: 0   max_err: 3.576e-07

==========================================
Overall result: PASS
==========================================
```

**Worst-case round-trip error: 3.576×10⁻⁷** (tolerance 1×10⁻⁵) — 28× margin.

### Numerical Edge Cases Documented

| Case | Observation |
|---|---|
| Zero input | Round-trips to zero without NaN or denormal issues |
| θ = 2π | `cosf(2π)` returns 1.0 exactly in glibc; behavior may vary on embedded targets — verify with hardware math library |
| Large amplitude (×1000) | Relative error unchanged; absolute error scales with amplitude as expected for float32 |
| Unbalanced (random i0≠0) | Zero-sequence preserved through Clarke; Park does not touch i0 — passing through correctly |
| θ near ±π | No discontinuities observed; `sinf`/`cosf` well-behaved in this region |

---

## Deliverables

| File | Description |
|---|---|
| [clarke.h](clarke.h) | Clarke transform declarations and type definitions |
| [clarke.c](clarke.c) | Amplitude-invariant Clarke forward and inverse |
| [park.h](park.h) | Park transform declarations |
| [park.c](park.c) | Park forward and inverse (uses `cosf`/`sinf`) |
| [test_transforms.c](test_transforms.c) | Known-answer, edge-case, and 1000-sample random round-trip test |
| [Makefile](Makefile) | `make run` to build and execute test harness on PC |
| Hand derivation | Inline above (§ Clarke Derivation, § Park Derivation) |
