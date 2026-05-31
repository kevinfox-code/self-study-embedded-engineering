# Full Closed-Loop FOC — Markdown Math Notes

## Variable Definitions

### Motor parameters

- $R_s$: stator resistance.
- $L_s$: stator inductance for an SPM motor, with $L_d = L_q = L_s$.
- $\psi$: permanent-magnet flux linkage magnitude.
- $p$: number of pole pairs.
- $J$: rotor and load inertia.
- $B$: viscous damping coefficient.
- $L_d$: d-axis inductance.
- $L_q$: q-axis inductance.

### Simulation parameters

- $T_s$: simulation sample time.
- $t_{end}$: simulation end time.
- $t$: simulation time vector.
- $N$: number of time samples.
- $V_{dc}$: DC bus voltage.
- $V_{lim}$: controller voltage limit, where $V_{lim} = V_{dc}/2$.

### Startup timing

- $t_{vf}$: V/f ramp interval.
- $t_{handoff}$: transition time from open-loop startup to closed-loop FOC.

### Speed reference

- $\omega_{ref,mech}$: mechanical speed reference.
- $\omega_{ref,elec}$: electrical speed reference, where $\omega_{ref,elec} = p\,\omega_{ref,mech}$.

### V/f startup parameters

- $f_{vf,end}$: final open-loop frequency in hertz.
- $\omega_{vf,end}$: final open-loop electrical angular speed, where $\omega_{vf,end} = 2\pi f_{vf,end}$.
- $V_{vf,max}$: maximum startup voltage magnitude.
- $V_{vf,min}$: minimum startup voltage magnitude.
- $\theta_{vf}$: open-loop electrical angle used during startup.
- $V_{\alpha}$, $V_{\beta}$: stationary-frame voltage commands.

### Current-loop PI gains

- $\omega_{c,i}$: current-loop bandwidth.
- $K_{p,id}$, $K_{i,id}$: d-axis PI proportional and integral gains.
- $K_{p,iq}$, $K_{i,iq}$: q-axis PI proportional and integral gains.
- $\int e_d\,dt$, $\int e_q\,dt$: accumulated d-axis and q-axis current-controller integrator states.

### Speed-loop PI gains

- $\omega_{c,spd}$: speed-loop bandwidth.
- $K_{p,spd}$, $K_{i,spd}$: speed-loop proportional and integral gains.
- $I_{q,lim}$: q-axis current limit.
- $\int e_{spd}\,dt$: speed-controller integrator state.

### Observer parameters

- $\psi^2$: squared permanent-magnet flux magnitude.
- $k_{drift}$: observer restoring gain used to regulate PM flux magnitude.
- $\lambda_{total,\alpha}$, $\lambda_{total,\beta}$: total stator flux estimate in the stationary frame.
- $\lambda_{PM,\alpha}$, $\lambda_{PM,\beta}$: estimated permanent-magnet flux components.
- $|\lambda_{PM}|$: PM flux magnitude.
- $e_{flux}$: PM flux magnitude error.
- $corr_{\alpha}$, $corr_{\beta}$: observer correction terms.
- $e_{bemf,\alpha}$, $e_{bemf,\beta}$: back-EMF observer inputs.

### PLL variables

- $K_{p,pll}$, $K_{i,pll}$: PLL proportional and integral gains.
- $\omega_{clamp}$: PLL speed clamp limit.
- $\theta_{obs}$: angle extracted from PM flux estimate.
- $e_{pll}$: PLL phase error.
- $\omega_{hat}$: estimated electrical angular speed.
- $\theta_{hat}$: estimated electrical rotor angle.
- $\int e_{pll}\,dt$: PLL integrator state.

### Plant and state variables

- $i_d$, $i_q$: d-axis and q-axis stator currents.
- $i_{\alpha}$, $i_{\beta}$: stationary-frame current components.
- $i_a$, $i_b$, $i_c$: phase currents.
- $V_d$, $V_q$: rotating-frame voltage commands after compensation.
- $V_{d,pi}$, $V_{q,pi}$: raw PI controller outputs.
- $V_m$: voltage magnitude, where $V_m = \sqrt{V_d^2 + V_q^2}$.
- $V_{a,true}$, $V_{b,true}$: applied stationary-frame voltages used by the plant and observer.
- $T_e$: electromagnetic torque.
- $\omega_m$: mechanical rotor speed.
- $\omega_r$: electrical rotor speed.
- $\theta_r$: true electrical rotor angle.

### Reference and error variables

- $i_{d,ref}$, $i_{q,ref}$: d-axis and q-axis current references.
- $e_d$, $e_q$: current control errors.
- $e_{spd}$: speed control error.
- $e_{\theta}$: angle estimation error, where $e_{\theta} = \operatorname{wrap}(\theta_{hat} - \theta_r)$.

### PWM variables

- $T_a$, $T_b$, $T_c$: SVPWM duty cycles.
- $sector$: active SVPWM sector.
- $V_{a,out}$, $V_{b,out}$, $V_{c,out}$: inverter phase voltages referred to the DC midpoint.

---

## Core Relationships

The total stator flux is

| Symbol | Meaning | STM32 MCU Block | Electrical / Physical Component |
|---|---|---|---|
| $i_a, i_b, i_c$ | Phase currents | ADC + timer-triggered sampling | Current shunts or inline phase current amplifiers |
| $i_{\alpha}, i_{\beta}$ | Clarke-frame currents | CPU math in control ISR | Computed from measured phase currents |
| $i_d, i_q$ | Park-frame currents | CPU math in FOC routine | Rotating-frame stator current representation |
| $i_{d,ref}, i_{q,ref}$ | Current references | Control firmware state machine | Command targets for flux and torque production |
| $V_d, V_q$ | dq voltage commands | PI controller outputs | Requested stator voltage vector |
| $V_{\alpha}, V_{\beta}$ | Stationary-frame voltage commands | Inverse Park + SVPWM prep | Requested inverter voltage vector |
| $T_a, T_b, T_c$ | PWM duty cycles | Advanced timer (TIM1 / TIM8) outputs | Gate drive duty commands for three-phase bridge |
| $V_{dc}$ | DC bus voltage | ADC measurement channel | DC link capacitor and supply bus |
| $V_{a,out}, V_{b,out}, V_{c,out}$ | Inverter phase-leg voltages | Timer compare outputs via gate driver | MOSFET / IGBT three-phase inverter bridge |
| $V_{a,true}, V_{b,true}$ | Applied $\alpha\beta$ voltages | Estimated in firmware from PWM + bus voltage | Effective stator voltage seen by the motor |
| $\theta_{hat}$ | Estimated rotor electrical angle | Observer + PLL firmware | Virtual rotor position used for commutation |
| $\omega_{hat}$ | Estimated electrical speed | Observer + PLL firmware | Virtual rotor speed estimate |
| $\theta_r$ | True electrical rotor angle | Encoder interface or simulated plant | Actual rotor magnetic angle |
| $\omega_m$ | Mechanical rotor speed | Timer capture, observer, or estimator | Motor shaft speed |
| $\lambda_{PM,\alpha}$, $\lambda_{PM,\beta}$ | PM flux estimate | Observer state variables in firmware | Estimated rotor magnet flux linkage |
| $R_s$, $L_d$, $L_q$, $\psi$, $p$, $J$, $B$ | Motor and load parameters | Constants in flash / calibration table | Motor windings, rotor magnets, mechanical load |
| $K_{p,id}$, $K_{i,id}$, $K_{p,iq}$, $K_{i,iq}$, $K_{p,spd}$, $K_{i,spd}$ | Controller gains | Firmware constants or live tuning registers | Control-law tuning values |
$$
\theta_{hat} = \operatorname{atan2}(\lambda_{PM,\beta}, \lambda_{PM,\alpha})
$$

---

## Flux Observer

The observer integrates back-EMF to estimate total stator flux:

$$
\frac{d\lambda}{dt} = \left[V_{\alpha\beta} - R_s I_{\alpha\beta}\right] + k_{drift}\left(\psi^2 - |\lambda_{PM}|^2\right)\lambda_{PM}
$$

Expanded into components,

$$
e_{bemf,\alpha} = V_{a,true} - R_s i_{\alpha}
$$

$$
e_{bemf,\beta} = V_{b,true} - R_s i_{\beta}
$$

$$
|\lambda_{PM}|^2 = \lambda_{PM,\alpha}^2 + \lambda_{PM,\beta}^2
$$

$$
e_{flux} = \psi^2 - |\lambda_{PM}|^2
$$

$$
corr_{\alpha} = k_{drift}\, e_{flux}\, \lambda_{PM,\alpha}
$$

$$
corr_{\beta} = k_{drift}\, e_{flux}\, \lambda_{PM,\beta}
$$

$$
\lambda_{total,\alpha}[k+1] = \lambda_{total,\alpha}[k] + T_s\left(e_{bemf,\alpha} + corr_{\alpha}\right)
$$

$$
\lambda_{total,\beta}[k+1] = \lambda_{total,\beta}[k] + T_s\left(e_{bemf,\beta} + corr_{\beta}\right)
$$

---

## PLL and Angle Tracking

$$
\theta_{obs} = \operatorname{atan2}(\lambda_{PM,\beta}, \lambda_{PM,\alpha})
$$

$$
e_{pll} = \sin(\theta_{obs} - \theta_{hat})
$$

$$
\left[\int e_{pll}\,dt\right](k+1) = \operatorname{clamp}\!\left(\left[\int e_{pll}\,dt\right](k) + K_{i,pll}\, e_{pll}\, T_s,\,-\omega_{clamp},\,\omega_{clamp}\right)
$$

$$
\omega_{hat} = K_{p,pll}\, e_{pll} + \int e_{pll}\,dt
$$

$$
\theta_{hat}[k+1] = \operatorname{wrap}\!\left(\theta_{hat}[k] + \omega_{hat}\, T_s\right)
$$

---

## Machine and Control Equations

$$
V_{d,app} = \cos(\theta_r)V_{a,true} + \sin(\theta_r)V_{b,true}
$$

$$
V_{q,app} = -\sin(\theta_r)V_{a,true} + \cos(\theta_r)V_{b,true}
$$

$$
i_d[k+1] = i_d[k] + T_s \frac{V_{d,app} - R_s i_d[k] + \omega_r L_q i_q[k]}{L_d}
$$

$$
i_q[k+1] = i_q[k] + T_s \frac{V_{q,app} - R_s i_q[k] - \omega_r L_d i_d[k] - \omega_r \psi}{L_q}
$$

$$
T_e = \frac{3}{2}p\left(\psi\, i_q + (L_d - L_q)\,i_d\, i_q\right)
$$

$$
\omega_m[k+1] = \omega_m[k] + T_s\frac{T_e - B\,\omega_m[k]}{J}
$$

$$
\omega_r = p\,\omega_m
$$

$$
\theta_r[k+1] = \operatorname{wrap}\!\left(\theta_r[k] + T_s\,\omega_r\right)
$$

---

## Transforms and Controllers

### Clarke Transform

$$
i_{\alpha} = \frac{2}{3}\left(i_a - \frac{1}{2}i_b - \frac{1}{2}i_c\right)
$$

$$
i_{\beta} = \frac{2}{3}\left(\frac{\sqrt{3}}{2}i_b - \frac{\sqrt{3}}{2}i_c\right)
$$

Phase current reconstruction:

$$
i_a = i_{\alpha}, \quad
i_b = -\frac{1}{2}i_{\alpha} + \frac{\sqrt{3}}{2}i_{\beta}, \quad
i_c = -\frac{1}{2}i_{\alpha} - \frac{\sqrt{3}}{2}i_{\beta}
$$

### Park Transform

$$
i_d = \cos(\theta_{hat})\,i_{\alpha} + \sin(\theta_{hat})\,i_{\beta}
$$

$$
i_q = -\sin(\theta_{hat})\,i_{\alpha} + \cos(\theta_{hat})\,i_{\beta}
$$

### Speed Controller

$$
e_{spd} = \omega_{ref,elec} - \omega_{hat}
$$

$$
\left[\int e_{spd}\,dt\right](k+1) = \left[\int e_{spd}\,dt\right](k) + K_{i,spd}\, e_{spd}\, T_s
$$

$$
i_{q,ref} = \operatorname{clamp}\!\left(K_{p,spd}\,e_{spd} + \int e_{spd}\,dt,\,-I_{q,lim},\,I_{q,lim}\right), \quad i_{d,ref} = 0
$$

### Current Controllers

$$
e_d = i_{d,ref} - i_d, \quad e_q = i_{q,ref} - i_q
$$

$$
\left[\int e_d\,dt\right](k+1) = \operatorname{clamp}\!\left(\left[\int e_d\,dt\right](k) + K_{i,id}\,e_d\,T_s,\,-V_{lim},\,V_{lim}\right)
$$

$$
\left[\int e_q\,dt\right](k+1) = \operatorname{clamp}\!\left(\left[\int e_q\,dt\right](k) + K_{i,iq}\,e_q\,T_s,\,-V_{lim},\,V_{lim}\right)
$$

$$
V_{d,pi} = K_{p,id}\,e_d + \int e_d\,dt, \quad V_{q,pi} = K_{p,iq}\,e_q + \int e_q\,dt
$$

### Decoupling and Voltage Limit

$$
V_d = V_{d,pi} - \omega_{hat}\,L_q\,i_q
$$

$$
V_q = V_{q,pi} + \omega_{hat}\,L_d\,i_d + \omega_{hat}\,\psi
$$

$$
V_m = \sqrt{V_d^2 + V_q^2}
$$

$$
(V_d, V_q) \leftarrow \begin{cases}
(V_d,\, V_q), & V_m \le V_{lim} \\
\left(V_d\dfrac{V_{lim}}{V_m},\; V_q\dfrac{V_{lim}}{V_m}\right), & V_m > V_{lim}
\end{cases}
$$

### Inverse Park

$$
V_{\alpha} = \cos(\theta_{hat})\,V_d - \sin(\theta_{hat})\,V_q
$$

$$
V_{\beta} = \sin(\theta_{hat})\,V_d + \cos(\theta_{hat})\,V_q
$$

---

## Startup and Reference Equations

$$
\omega_{ref,mech}(t) = \min\!\left(\max\!\left(\frac{t - t_{handoff}}{0.3}, 0\right), 1\right) \cdot 1500\left(\frac{2\pi}{60}\right)
$$

$$
\omega_{ref,elec}(t) = p\,\omega_{ref,mech}(t)
$$

$$
\alpha_{vf}(t) = \min\!\left(\frac{t}{t_{vf}}, 1\right)
$$

$$
\omega_{vf}(t) = \alpha_{vf}(t)\,\omega_{vf,end}, \quad V_{vf}(t) = V_{vf,min} + \alpha_{vf}(t)\left(V_{vf,max} - V_{vf,min}\right)
$$

$$
V_{\alpha} = V_{vf}\cos(\theta_{vf}), \quad V_{\beta} = V_{vf}\sin(\theta_{vf})
$$

$$
\theta_{vf}[k+1] = \operatorname{wrap}\!\left(\theta_{vf}[k] + \omega_{vf}\,T_s\right)
$$

---

## SVPWM and Inverter Mapping

$$
V_{a,out} = (T_a - 0.5)\,V_{dc}, \quad
V_{b,out} = (T_b - 0.5)\,V_{dc}, \quad
V_{c,out} = (T_c - 0.5)\,V_{dc}
$$

$$
V_{a,true} = \frac{2}{3}\left(V_{a,out} - \frac{1}{2}V_{b,out} - \frac{1}{2}V_{c,out}\right)
$$

$$
V_{b,true} = \frac{2}{3}\left(\frac{\sqrt{3}}{2}V_{b,out} - \frac{\sqrt{3}}{2}V_{c,out}\right)
$$

---

## STM32-to-Electrical Mapping

| Symbol | Meaning | STM32 MCU Block | Electrical / Physical Component |
|---|---|---|---|
| $i_a, i_b, i_c$ | Phase currents | ADC + timer-triggered sampling | Current shunts or inline phase current amplifiers |
| $i_{\alpha}, i_{\beta}$ | Clarke-frame currents | CPU math in control ISR | Computed from measured phase currents |
| $i_d, i_q$ | Park-frame currents | CPU math in FOC routine | Rotating-frame stator current representation |
| $i_{d,ref}, i_{q,ref}$ | Current references | Control firmware state machine | Command targets for flux and torque production |
| $V_d, V_q$ | dq voltage commands | PI controller outputs | Requested stator voltage vector |
| $V_{\alpha}, V_{\beta}$ | Stationary-frame voltage commands | Inverse Park + SVPWM prep | Requested inverter voltage vector |
| $T_a, T_b, T_c$ | PWM duty cycles | Advanced timer (TIM1 / TIM8) outputs | Gate drive duty commands for three-phase bridge |
| $V_{dc}$ | DC bus voltage | ADC measurement channel | DC link capacitor and supply bus |
| $V_{a,out}, V_{b,out}, V_{c,out}$ | Inverter phase-leg voltages | Timer compare outputs via gate driver | MOSFET / IGBT three-phase inverter bridge |
| $V_{a,true}, V_{b,true}$ | Applied $\alpha\beta$ voltages | Estimated in firmware from PWM + bus voltage | Effective stator voltage seen by the motor |
| $\theta_{hat}$ | Estimated rotor electrical angle | Observer + PLL firmware | Virtual rotor position used for commutation |
| $\omega_{hat}$ | Estimated electrical speed | Observer + PLL firmware | Virtual rotor speed estimate |
| $\theta_r$ | True electrical rotor angle | Encoder interface or simulated plant | Actual rotor magnetic angle |
| $\omega_m$ | Mechanical rotor speed | Timer capture, observer, or estimator | Motor shaft speed |
| $\lambda_{PM,\alpha}, \lambda_{PM,\beta}$ | PM flux estimate | Observer state variables in firmware | Estimated rotor magnet flux linkage |
| $R_s, L_d, L_q, \psi, p, J, B$ | Motor and load parameters | Constants in flash / calibration table | Motor windings, rotor magnets, mechanical load |
| $K_{p,id}, K_{i,id}, K_{p,iq}, K_{i,iq}, K_{p,spd}, K_{i,spd}$ | Controller gains | Firmware constants or live tuning registers | Control-law tuning values |
