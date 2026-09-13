# Flow of Heat — Formula Sheet

## Core Heat Current Equation

$$q = \frac{\Delta T}{R}$$

This is the thermal analogy of Ohm's Law (Heat Current ↔ Current, ΔT ↔ Voltage, Thermal Resistance ↔ Electrical Resistance).

---

## Variable Definitions

| Symbol | Name | Unit |
|---|---|---|
| **q** | Heat Current | Watts (W) |
| **ΔT** | Temperature Difference | Kelvin (K) |
| **R** | Thermal Resistance | K/W |

---

## 1. Conduction

Heat flows **through** a material from a hotter side to a colder side.

**Setup:** A solid block of length **L**, cross-sectional area **A**, with T_L > T_R (left side hotter than right side). Heat current **q** flows left → right.

### Thermal Resistance (Conduction)

$$R = \frac{1}{\lambda} \times \frac{l}{A}$$

| Symbol | Name | Unit |
|---|---|---|
| **λ (lambda)** | Conductivity | W/(m·K) |
| **l** | Length (path of heat flow, e.g. L) | m |
| **A** | Cross-sectional Area | m² |

**Condition:** T_L > T_R (heat flows from hot to cold)

---

## 2. Convection

Heat flows **from a surface** into a moving fluid (e.g., air).

**Setup:** A surface at temperature **Ts**, exposed to a fluid at ambient temperature **T∞** (T-infinity), with Ts > T∞. Heat current **q** flows from the surface upward into the fluid.

### Thermal Resistance (Convection)

$$R = \frac{1}{h} \times \frac{1}{A}$$

| Symbol | Name | Unit |
|---|---|---|
| **h** | Heat Transfer Co-efficient | W/(m²·K) |
| **A** | Surface Area | m² |

**Condition:** Ts > T∞ (surface hotter than surrounding fluid)

---

## 3. Summary Table

| Mode | Resistance Formula | Key Coefficient | Driving Condition |
|---|---|---|---|
| **Conduction** | R = (1/λ)(l/A) | λ — Conductivity, W/(m·K) | T_L > T_R |
| **Convection** | R = (1/h)(1/A) | h — Heat Transfer Coefficient, W/(m²·K) | Ts > T∞ |

---

## 4. Comparison — Conduction vs Convection

| Feature | Conduction | Convection |
|---|---|---|
| Mechanism | Heat transfer **through** a solid | Heat transfer **from a surface** into a fluid |
| Material property | Conductivity (λ) | Heat transfer coefficient (h) |
| Geometry term | l/A (length over area) | 1/A (area only — no separate length term) |
| Temperature labels | T_L, T_R | Ts, T∞ |

---

## 5. General Pattern (Ohm's Law Analogy)

| Electrical | Magnetic | Thermal |
|---|---|---|
| Voltage (V) | MMF (F) | Temp. Difference (ΔT) |
| Current (I) | Flux (Φ) | Heat Current (q) |
| Resistance (R) | Reluctance (R) | Thermal Resistance (R) |
| V = IR | F = ΦR | ΔT = qR |
