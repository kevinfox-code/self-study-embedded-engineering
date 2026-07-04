# The B-H Curve — Formula Sheet

## Key Concept
Unlike electrical conductivity, **permeability is non-linear**. The relationship between Flux Density (B) and Magnetic Field Strength (H) changes shape depending on the material and the point on the curve.

---

## 1. B-H Curve of Soft Magnetics (Non-Linear)

The curve rises steeply, then bends over and flattens as the material approaches **saturation**.

### Apparent Permeability
$$\mu = \frac{B}{H}$$

- The ratio of total B to total H at any point on the curve.
- This is the *slope of a line from the origin* to a point on the curve.

### Incremental Permeability
$$\mu = \frac{dB}{dH}$$

- The *rate of change* of B with respect to H — i.e., the local slope (tangent) of the curve at a given point.
- This differs from apparent permeability because the curve is non-linear; near saturation, dB/dH becomes small even though B/H may still be relatively large.

### B-H Curve of Linear Material
- For comparison, a linear material has a constant-slope (straight-line) B-H relationship, meaning μ = B/H = dB/dH at every point (permeability doesn't change with H).

---

## 2. Variable Definitions

| Symbol | Name | Unit |
|---|---|---|
| **B** | Magnetic Flux Density | Tesla (T) |
| **H** | Magnetic Field Strength | Ampere/metre (A/m) |
| **μ** | Permeability | H/m |
| **dB** | Small change in Flux Density | T |
| **dH** | Small change in Field Strength | A/m |

---

## 3. The Hysteresis Loop (B-H Loop Under AC)

When an AC current is applied, the magnetizing force (H) cycles between +H and -H, tracing a closed loop instead of retracing the original curve — this is called **Hysteresis Loss**.

### Loop Points & Definitions

| Point | Label | Meaning |
|---|---|---|
| **a** | +Bmax (Saturation) | Maximum flux density in the positive direction; material is fully magnetized |
| **b** | +Br (Retentivity) | Residual flux density remaining when H returns to 0 (positive side) |
| **c** | -Hc (Coercivity) | Magnetizing force in the *opposite* direction needed to bring B back to 0 |
| **d** | +Hc (Coercivity) | Magnetizing force in the *opposite* direction needed to bring B back to 0 (returning leg) |
| **e** | -Br (Retentivity) | Residual flux density remaining when H returns to 0 (negative side) |
| **f** | -Bmax (Saturation) | Maximum flux density in the negative direction |

### Key Terms

- **Retentivity (Br):** The flux density that *remains* in the material after the magnetizing force is removed (returned to H = 0).
- **Coercivity (Hc):** The reverse magnetizing force required to reduce the residual flux density back to zero.
- **Saturation (Bmax):** The point beyond which increasing H produces no further significant increase in B.

### Loop Path
$$a \rightarrow b \rightarrow c \rightarrow d \rightarrow e \rightarrow f \rightarrow a$$

Tracing this full loop for one AC cycle represents one cycle of **hysteresis loss** — energy dissipated as heat due to the lag between B and H.

---

## 4. Summary Table

| Equation | Description |
|---|---|
| μ = B / H | Apparent permeability (slope from origin) |
| μ = dB / dH | Incremental permeability (local slope/tangent) |
| +Bmax, -Bmax | Saturation flux density (positive/negative) |
| +Br, -Br | Retentivity (residual flux density at H = 0) |
| +Hc, -Hc | Coercivity (reverse force needed to zero out B) |

---

## 5. Quick Notes
- **Permeability is non-linear** — unlike conductivity, which is constant for a given material.
- The **wider** the hysteresis loop, the **greater the hysteresis loss** (more energy dissipated per AC cycle).
- Soft magnetic materials have **narrow** loops (low loss); hard magnetic materials (permanent magnets) have **wide** loops (high retentivity/coercivity).
