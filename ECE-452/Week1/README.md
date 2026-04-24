**Week 1 Homework: Permanent Magnets & PMSM Machine Physics**

**Back-EMF Constant (Ke):**

- Compute $K_e$ from Eq. 1.54 (Krishnan p. 54): $K_e = \dfrac{V_{pk}}{p\,\omega_m}$, where $p$ is pole pairs; units $\mathrm{V/(rad/s)}$ electrical.

### Induced EMF Equation Derivation

The signed hand-derivation starts from synchronous machine fundamentals (§1.5, pp. 51–75). For a PMSM, the induced phase EMF is:

$$
e_a = N_{ph} k_w \frac{d}{dt}\!\left[\Phi_m \cos(\theta_e)\right]
	= -p\omega_m N_{ph} k_w \Phi_m \sin(p\theta_m)
$$

where $N_{ph}$ is turns per phase, $k_w$ winding factor, $\Phi_m$ flux per pole, $p$ pole pairs, $\theta_e = p\theta_m$ electrical angle, and $\omega_m$ mechanical speed. Peak value yields Eq. 1.54: $E_{pk} = p\omega_m\lambda_f$, with $K_e = \lambda_f$ (flux linkage).
