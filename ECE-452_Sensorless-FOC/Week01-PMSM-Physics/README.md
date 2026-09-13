# ECE-452 Week 01 — Permanent Magnets and PMSM Machine Physics

**Reading:** Krishnan Ch. 1, §1.1–§1.5 — magnet materials, the B-H curve, flux
linkage, and torque production.

## Formula Sheets

| Sheet | Topic |
|---|---|
| [01-Magnetism_Formula_Sheet.md](01-Magnetism_Formula_Sheet.md) | Field quantities, permeability, magnetic circuits |
| [02-BH_Curve_Formula_Sheet.md](02-BH_Curve_Formula_Sheet.md) | Hysteresis loop, remanence, coercivity, energy product |
| [03-Flux_Linkage_Inductance_Formula_Sheet.md](03-Flux_Linkage_Inductance_Formula_Sheet.md) | Flux linkage and self/mutual inductance |
| [04-Flux_Linkage_vs_Current_Formula_Sheet.md](04-Flux_Linkage_vs_Current_Formula_Sheet.md) | Saturation and the λ–i relationship |
| [05-Flow_of_Heat_Formula_Sheet.md](05-Flow_of_Heat_Formula_Sheet.md) | Thermal resistance and machine loss dissipation |
| [06-Torque_on_a_Loop_Formula_Sheet.md](06-Torque_on_a_Loop_Formula_Sheet.md) | Force and torque on a current-carrying loop |
| [07-How_to_Increase_Torque.md](07-How_to_Increase_Torque.md) | The design levers available for raising torque |

## Homework Notes

**Back-EMF Constant (Ke):**

- Compute $K_e$ from Eq. 1.54 (Krishnan p. 54): $K_e = \dfrac{V_{pk}}{p\,\omega_m}$, where $p$ is pole pairs; units $\mathrm{V/(rad/s)}$ electrical.

### Induced EMF Equation Derivation

The signed hand-derivation starts from synchronous machine fundamentals (§1.5, pp. 51–75). For a PMSM, the induced phase EMF is:

$$
e_a = N_{ph} k_w \frac{d}{dt}\!\left[\Phi_m \cos(\theta_e)\right]
	= -p\omega_m N_{ph} k_w \Phi_m \sin(p\theta_m)
$$

where $N_{ph}$ is turns per phase, $k_w$ winding factor, $\Phi_m$ flux per pole, $p$ pole pairs, $\theta_e = p\theta_m$ electrical angle, and $\omega_m$ mechanical speed. Peak value yields Eq. 1.54: $E_{pk} = p\omega_m\lambda_f$, with $K_e = \lambda_f$ (flux linkage).

## References

- Krishnan, *Permanent Magnet Synchronous and Brushless DC Motor Drives*, Ch. 1
- See [ECE-452_References.md](../ECE-452_References.md) for the full reading list

## AI Assistance

None.
