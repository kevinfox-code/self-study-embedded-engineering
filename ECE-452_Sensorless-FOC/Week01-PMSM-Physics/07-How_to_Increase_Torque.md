# How to Increase Torque?

Based on **τ = BAN·I·sin(δ)**, torque can be increased by:

| Method | Variable |
|---|---|
| Use a stronger magnet | **B** |
| Increase the current | **I** |
| Make loops of larger area | **A** |
| Increase the turns | **N** |

---

## Real-World Pros & Cons

### 1. Stronger Magnet (B)
**Pros:**
- No extra electrical losses (heat) — torque gain is "free" in terms of I²R losses
- Doesn't increase resistance or inductance of the winding

**Cons:**
- Stronger magnets (e.g., higher-grade rare-earth/NdFeB) are **expensive**
- Adds **weight**
- Rare-earth magnets can **demagnetize** at high temperatures
- Often a **fixed design choice** — hard to change after the motor is built
- Supply chain/ethical concerns around rare-earth mining

### 2. Increase Current (I)
**Pros:**
- Easiest to change dynamically — just drive more current (this is how motor controllers vary torque in real time)
- No physical redesign needed

**Cons:**
- Heat losses scale with **I²R** — higher current = much more heat, lower efficiency
- Requires thicker wires / better cooling to avoid overheating
- Limited by the power electronics (inverter/driver) current rating
- Can demagnetize permanent magnets if current is too high (armature reaction)

### 3. Larger Loop Area (A)
**Pros:**
- Increases torque without raising current or heat losses
- Conceptually simple — just a bigger loop/coil

**Cons:**
- Makes the motor **physically larger** — bad for compact applications (e.g., drones, EVs with tight packaging)
- Larger loop = **more wire length** = more resistance = more I²R loss (partially offsets the benefit)
- Increases rotor **inertia**, which can slow down acceleration/response

### 4. Increase Turns (N)
**Pros:**
- Increases torque per unit current (same effect as increasing flux linkage)
- Doesn't require a bigger magnet or more current

**Cons:**
- More turns = **more wire**, more resistance, more copper losses (I²R)
- Increases winding **inductance**, which slows down current response (worse high-speed/high-frequency performance)
- More turns of thinner wire (to fit in same space) can reduce current-carrying capacity
- Adds to manufacturing complexity and cost

---

## Quick Takeaway

| Variable | Best for... | Watch out for... |
|---|---|---|
| B | Efficiency gains with no extra heat | Cost, weight, demagnetization risk |
| I | Quick/dynamic torque control | Heat (I²R losses), component current limits |
| A | Torque without more heat | Motor size, added wire resistance, inertia |
| N | Torque per amp | Resistance, inductance, response speed |

In practice, real motor design is a **trade-off** between all four — there's no free lever; each one trades off against efficiency, size, cost, or dynamic performance.
