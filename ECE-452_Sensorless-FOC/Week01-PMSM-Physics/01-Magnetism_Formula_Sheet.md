# Flow of Magnetism — Formula Sheet

## Core Magnetic Circuit Equation

$$\Phi = \dfrac{F}{R}$$

This is the magnetic-circuit analogy of Ohm's Law (Flux ↔ Current, MMF ↔ Voltage, Reluctance ↔ Resistance).

---

## Variable Definitions

| Symbol | Name | Formula | Unit |
|---|---|---|---|
| **Φ** | Magnetic Flux | Φ = B × A | Weber (Wb) |
| **B** | Flux Density | — | Tesla (T) |
| **A** | Cross-sectional Area | — | m² |
| **F** | Magnetomotive Force (MMF) | F = I × N | Ampere-turns (A·t) |
| **I** | Current | — | Amperes (A) |
| **N** | Number of Turns | — | (count) |
| **R** | Reluctance | R = l / (μA) | A·t/Wb |
| **l** | Length (of magnetic path) | — | m |
| **μ** | Permeability | μ = μ₀ × μᵣ | H/m |
| **μ₀** | Permeability of Free Space | constant ≈ 4π×10⁻⁷ H/m | H/m |
| **μᵣ** | Relative Permeability | — | (dimensionless) |
| **H** | Field (Magnetizing) Strength | H = I × N / l | Ampere-turns/metre |

---

## Derivation — Rearranging Φ = F/R

**Step 1:** Start with the core relationship:
$$\Phi \times R = F$$

**Step 2:** Substitute R = l/(μA) and F = I·N:
$$\Phi \times \left(\frac{l}{\mu A}\right) = I \times N$$

**Step 3:** Rearrange to isolate Φ/A:
$$\frac{\Phi}{A} = \mu \times \left(\frac{I \times N}{l}\right)$$

**Step 4:** Recognize the two key substitutions:
- Φ/A = **B** (Flux Density)
- I·N/l = **H** (Field Strength)

**Result — Final Relationship:**
$$\boxed{B = \mu \times H}$$

---

## Summary of Key Relationships

| Equation | Description |
|---|---|
| Φ = F / R | Magnetic flux = MMF ÷ Reluctance |
| Φ = B × A | Flux = Flux density × Area |
| F = I × N | MMF = Current × Turns |
| R = l / (μA) | Reluctance = Length ÷ (Permeability × Area) |
| H = I × N / l | Field strength = MMF ÷ Length |
| μ = μ₀ × μᵣ | Permeability = Free-space permeability × Relative permeability |
| **B = μ × H** | Flux density = Permeability × Field strength |

---

## Units Reference

| Quantity | Unit | Symbol |
|---|---|---|
| Magnetic Flux (Φ) | Weber | Wb |
| Flux Density (B) | Tesla | T |
| MMF (F) | Ampere-turns | A·t |
| Field Strength (H) | Ampere-turns/metre | A·t/m |
| Reluctance (R) | Ampere-turns/Weber | A·t/Wb |
