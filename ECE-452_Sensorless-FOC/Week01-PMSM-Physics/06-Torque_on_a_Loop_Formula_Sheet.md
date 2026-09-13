# Torque on a Loop — Formula Sheet

## Context
This is a **PMDC motor** (Permanent Magnet DC motor) involving split rings and sliding carbon brushes for commutation. A current-carrying loop sits in a magnetic field between N and S poles; the forces on each side of the loop create a **torque** that rotates the loop.

---

## Core Torque Equation

$$|\vec{\tau}| = F \times w \times \sin(\delta)$$

Where **δ**, the torque angle:
$$\delta = 90^\circ - \theta$$

---

## Variable Definitions

| Symbol | Name | Description |
|---|---|---|
| **τ** | Torque | Rotational force on the loop |
| **F** | Force | Force on each side of the current loop (from F = BIl) |
| **w** | Width | Width of the loop (moment arm) |
| **δ** | Torque Angle | δ = 90° − θ |
| **θ** | Phase Advance Angle | Angle between current direction and the q-axis |
| **B** | Magnetic Field | Field strength between N and S poles |
| **I** | Current | Current flowing through the loop |
| **l** | Length | Length of the loop side carrying current |
| **A** | Area | Area enclosed by the loop, A = l × w |
| **N** | Number of Turns | Number of turns of wire in the loop |
| **ψ** | Flux Linkage | ψ = B × A × N |

---

## Step-by-Step Derivation

**Step 1:** Start with the basic torque equation:
$$|\vec{\tau}| = F \times w \times \sin(\delta)$$

**Step 2:** Substitute F = BIl (force on a current-carrying wire):
$$|\vec{\tau}| = BIl \times w \times \sin(\delta)$$

**Step 3:** Recognize that l × w = A (Area of the loop):
$$|\vec{\tau}| = BIA \times \sin(\delta)$$

**Step 4:** Account for multiple turns (N):
$$|\vec{\tau}| = BAN \times I \times \sin(\delta)$$

**Step 5:** Substitute ψ = BAN (Flux Linkage):
$$|\vec{\tau}| = \psi \times I \times \sin(\delta)$$

**Step 6:** Using the angle relationship sin(δ) = cos(θ):
$$|\vec{\tau}| = \psi \times I \times \cos(\theta)$$

---

## Vector Form

$$\vec{\tau} = \vec{I} \times \vec{\psi}$$

$$|\vec{\tau}| = I \times \psi \times \sin(\delta) = I \times \psi \times \cos(\theta)$$

---

## Geometric Relationship

$$w \sin(\delta) = w \cos(\theta)$$

This comes from the geometry of the loop's orientation relative to the **d-q axis** frame:
- **q-axis:** vertical reference axis
- **d-axis:** horizontal reference axis
- **I** (current direction) makes angle **θ** with the q-axis
- **δ** is the angle between the loop's width vector **w** and the **B** field direction

---

## Summary Table

| Equation | Description |
|---|---|
| τ = F·w·sin(δ) | Basic torque from force and moment arm |
| τ = BIl·w·sin(δ) | Substituting F = BIl |
| τ = BIA·sin(δ), A = l·w | Substituting Area |
| τ = BAN·I·sin(δ) | Accounting for N turns |
| τ = ψ·I·sin(δ) = ψ·I·cos(θ) | Substituting Flux Linkage ψ = BAN |
| **τ** = **I** × **ψ** (vector) | Vector cross-product form |
| δ = 90° − θ | Relationship between torque angle and phase advance angle |
| ψ = BAN | Flux Linkage definition |

---

## Key Takeaway

Torque on the loop can be expressed in two equivalent ways depending on which angle is used:

$$|\vec{\tau}| = \psi I \sin(\delta) = \psi I \cos(\theta)$$

Both forms describe the same physical torque — just referenced to different angles (δ measured from B-field, θ measured from the q-axis).
