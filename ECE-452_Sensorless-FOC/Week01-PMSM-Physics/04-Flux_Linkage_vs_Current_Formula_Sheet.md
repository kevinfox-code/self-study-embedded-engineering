# Flux Linkage vs Current — Formula Sheet

## Key Concept
As current **i** increases, the flux linkage **ψ** curve **saturates** — it rises steeply at first, then flattens out. Because of this non-linearity, two different kinds of inductance can be defined at any operating point: **apparent inductance** and **incremental inductance**.

---

## 1. The Curve

The plot of **ψ (Flux Linkage)** vs **i (current)**:
- Starts at the origin
- Rises steeply for small i
- Curves over and flattens as i increases (saturation)

At an operating point **i₀**, the flux linkage is **ψ₀**.
If current increases by a small amount **Δi₀**, flux linkage increases by **Δψ₀**, reaching **ψ₀ + Δψ₀**.

---

## 2. Apparent Inductance

$$L_{apparent} = \frac{\psi_0}{i_0}$$

- This is the slope of a **straight line from the origin** to the point (i₀, ψ₀) on the curve.
- Represents the *average* inductance up to that point.

---

## 3. Incremental Inductance

$$L_{incremental} = \frac{\Delta \psi_0}{\Delta i_0}$$

- This is the slope of the **tangent line** to the curve at point (i₀, ψ₀).
- Represents the *instantaneous* rate of change of flux linkage with respect to current at that exact operating point.

---

## 4. Variable Definitions

| Symbol | Name | Unit |
|---|---|---|
| **ψ** | Flux Linkage | Weber-turns (Wb·t) |
| **i** | Current | Amperes (A) |
| **i₀** | Operating point current | A |
| **Δi₀** | Small increase in current | A |
| **ψ₀** | Flux linkage at i₀ | Wb·t |
| **Δψ₀** | Small increase in flux linkage | Wb·t |

---

## 5. Why This Matters — Saturation

> As current increases, the flux linkage **saturates** — because **incremental inductance falls**.

- Near the origin (low current): the curve is steep → apparent and incremental inductance are similar and high.
- At higher current (near saturation): the curve flattens → incremental inductance (slope of tangent) becomes much **smaller** than apparent inductance (slope from origin).
- This is the same non-linear saturation behavior seen in the **B-H curve** — flux linkage (like flux density) cannot increase indefinitely with current (like field strength).

---

## 6. Summary Table

| Equation | Description |
|---|---|
| L_apparent = ψ₀ / i₀ | Slope of line from origin to operating point |
| L_incremental = Δψ₀ / Δi₀ | Slope of tangent at operating point (local rate of change) |
| As i ↑, L_incremental ↓ | Inductance decreases approaching saturation |

---

## 7. Relationship to Earlier Concepts

| Flux Linkage vs Current | Equivalent in B-H Curve |
|---|---|
| ψ (Flux Linkage) | B (Flux Density) |
| i (Current) | H (Field Strength) |
| Apparent Inductance (ψ₀/i₀) | Apparent Permeability (B/H) |
| Incremental Inductance (Δψ₀/Δi₀) | Incremental Permeability (dB/dH) |

Both relationships show the same underlying non-linear saturation behavior of magnetic materials.
