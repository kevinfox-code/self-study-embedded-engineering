# Flux Linkage and Inductance — Formula Sheet

## Recall (from Flow of Magnetism)

$$\Phi \times R = F$$

$$\Phi \times \left(\frac{l}{\mu A}\right) = I \times N$$

The flux **Φ** "links" all **N** turns of wire wound around the core.

---

## 1. Flux Linkage

$$\psi = N \times \Phi$$

- **ψ (psi)** is the **Flux Linkage** — the total flux experienced across all turns of the coil.

---

## 2. Substituting Φ into Flux Linkage

Since Φ = N·I / (l/μA), substituting into ψ = N·Φ gives:

$$\psi = N^2 \times \left(\frac{\mu A}{l}\right) \times I$$

This expression contains two important derived quantities, bracketed together:

- **Permeance (P)** = μA / l
- **Inductance (L)** = N² × (μA/l) = N² × P

---

## 3. Variable Definitions

| Symbol | Name | Formula | Unit |
|---|---|---|---|
| **Φ** | Magnetic Flux | Φ = F/R | Weber (Wb) |
| **ψ** | Flux Linkage | ψ = N × Φ | Weber-turns (Wb·t) |
| **N** | Number of Turns | — | (count) |
| **I** | Current | — | Amperes (A) |
| **l** | Length of magnetic path | — | m |
| **μ** | Permeability | μ = μ₀ × μᵣ | H/m |
| **A** | Cross-sectional Area | — | m² |
| **P** | Permeance | P = μA / l | Henry (H) |
| **L** | Inductance | L = N² × (μA/l) = N² × P | Henry (H) |

---

## 4. Key Equations Summary

| Equation | Description |
|---|---|
| ψ = N × Φ | Flux Linkage = Turns × Flux |
| P = μA / l | Permeance — the "ease" with which flux is established (inverse of reluctance) |
| L = N² × P | Inductance = (Turns)² × Permeance |
| ψ = N² × (μA/l) × I | Flux Linkage expressed fully in circuit/geometry terms |
| **ψ = L × I** | Flux Linkage = Inductance × Current |

---

## 5. Units Note

Both **Inductance (L)** and **Permeance (P)** are measured in the same unit:

$$\text{Henry (H)}$$

---

## 6. Relationship Map

$$\Phi \xrightarrow{\times N} \psi \quad\quad \psi = N^2 \cdot \left(\frac{\mu A}{l}\right) \cdot I = L \cdot I$$

- Permeance (P) is the *geometric/material* term: μA/l
- Inductance (L) scales Permeance by N² — i.e., inductance depends on the **square** of the number of turns.
