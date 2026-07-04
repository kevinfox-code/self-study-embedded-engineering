# Sensorless FOC Library for SM-PMSM — STM32U5 + DRV8323 — Implementation Plan

**Document type:** Handoff implementation plan for a lower-cost coding model.
**Scope:** Plan only. No library source code is contained here. Every module below is specified by contract (purpose, API, types, dependencies, tests) precisely enough to be implemented file-by-file with minimal supervision.

---

## 1. Goal

Build a reusable, production-oriented, **sensorless field-oriented control (FOC) library** in embedded C for a surface-mounted PMSM (SM-PMSM), targeting STM32U5 with a TI DRV8323 smart gate driver, running under CMSIS-RTOS v2 on FreeRTOS.

The library must:

1. Run all control math in **fixed-point** (no runtime `float`/`double` in any control path, ISR, or task-rate code).
2. Provide a complete motor state machine (IDLE → INIT → CALIBRATE → IDENTIFY → ALIGN → OPEN_LOOP_START → OPEN_LOOP_RAMP → TRANSITION → CLOSED_LOOP_RUN → STOPPING, plus FAULT), with defined guards, timeouts, and restart policy.
3. Estimate rotor angle and speed sensorlessly via a back-EMF / flux-model observer with a PLL tracking loop, with an explicit open-loop startup path (back-EMF observability is poor near zero speed).
4. Identify motor parameters (Rs, Ls, flux linkage λm / back-EMF constant Ke) and/or match against a table of candidate motor profiles (`motor_modeler`), with quality/acceptance checks.
5. Implement cascaded control: fast current loop in the PWM/ADC ISR, torque/current supervision task, speed control task.
6. Isolate **all** HAL/CubeMX/board specifics behind `constants.h` + `board_support.*` + `motor_hw_if.*` so the control core is host-buildable and host-testable without HAL or FreeRTOS.
7. Integrate DRV8323 fully: SPI register configuration, gate-drive setup, current-shunt amplifier (CSA) configuration, fault readback, enable/fault GPIO sequencing.
8. Provide a user-initiated **TUNE state** (entered from IDLE) with three sub-modes for manual controller calibration: torque-PI step excitation, speed-PI square-wave step excitation, and observer inspection (synchronized phase-current vs back-EMF capture), all with high-rate data capture and live gain adjustment.
9. Ship with first-class tests: unit tests (host), module tests with synthetic traces (host), integration tests (target), hardware bring-up tests (target), and fixed-point-vs-float numeric verification with defined tolerance bands.

"Done" is defined by Section 13.

---

## 2. System context

| Item | Assumption | Notes / where configured |
|---|---|---|
| MCU | STM32U5 family (Cortex-M33, ≥160 MHz core assumed) | Exact part, clocks in `constants.h` |
| PWM timer | Advanced timer (TIM1) center-aligned, complementary outputs + dead time | Timer handle/name only in `constants.h` |
| PWM frequency | 20 kHz default (`FOC_PWM_FREQ_HZ`), fast-loop = every PWM period (decimation factor configurable, default 1) | `constants.h` / `motor_params.h` |
| ADC | ADC injected (or DMA regular) conversions hardware-triggered by TIM1 event; 3 phase-current channels + Vbus + optional temperature | Channel macros only in `constants.h` |
| Current sensing | 3 low-side shunts amplified by DRV8323 integrated CSAs (gain SPI-selectable: 5/10/20/40 V/V; default 20 V/V), biased at VREF/2 | Shunt value, gain, VREF in `constants.h` |
| Sampling instant | Center-aligned PWM; sample while low-side FETs conduct (counter update event corresponding to full low-side conduction). 2-of-3 phase selection at high modulation index | Board layer responsibility |
| Gate driver | DRV8323 (SPI variant), 6x/3x/1x PWM modes — use **6x PWM mode**; smart gate drive (IDRIVE/TDRIVE) configured via SPI; nFAULT GPIO input; ENABLE GPIO output | `drv8323.*` + `board_support.*` |
| RTOS | FreeRTOS via CMSIS-RTOS v2 API only (`osThreadNew`, `osMessageQueueNew`, `osEventFlags…`, `osMutex…`, `osThreadFlags…`) | Control core never includes RTOS headers |
| Motor | SM-PMSM, Ld ≈ Lq = Ls allowed; pole pairs known per profile or user-supplied | `motor_params.h`, profiles |
| Numeric | Fixed-point only at runtime. FPU may exist but must not be used in control paths. Floats allowed **only** in host-side test/reference code | Section 7 |
| DMA | Allowed for ADC result transfer and SPI, but deterministic fast-loop timing has priority; injected-ADC-in-ISR is the default design | `board_support.c` |
| Fast-loop budget | ≤ 50% of PWM period at 20 kHz → ≤ 25 µs worst case, target ≤ 15 µs | Verified in bring-up tests |
| Bus voltage | 12–48 V class assumed; all limits parameterized | `motor_limits.h` |

**Explicit constraint restated:** `constants.h` is the *only* file that may reference CubeMX/HAL-generated symbols (handles like `htim1`, `hadc1`, `hspi1`, channel macros, pin/port names, IRQ names, DMA linkage symbols). No control-law file may include a Cube-generated header or call a raw HAL API. All HAL calls live in `board_support.c` (and DRV8323 SPI transport), behind the portable interface `motor_hw_if.h`.

---

## 3. Architecture

### 3.1 Layers and dependency direction

Dependencies point strictly downward. Nothing in Layers A–B may include HAL, CMSIS device headers, Cube-generated headers, or FreeRTOS/CMSIS-RTOS headers.

```
┌────────────────────────────────────────────────────────────┐
│ Layer D: Application / integration (target only)           │
│   freertos_tasks.c   isr_motor.c   (user app main)         │
├────────────────────────────────────────────────────────────┤
│ Layer C: Platform (target only, ALL HAL confined here)     │
│   constants.h  board_support.c/.h  drv8323 transport glue  │
│   implements motor_hw_if.h                                 │
├────────────────────────────────────────────────────────────┤
│ Layer B: Control services (portable, host-testable)        │
│   motor_ctrl  motor_sm  motor_ident  motor_modeler         │
│   motor_observer  motor_foc  motor_faults                  │
│   drv8323 register logic (pack/unpack, no transport)       │
├────────────────────────────────────────────────────────────┤
│ Layer A: Foundations (portable, host-testable)             │
│   motor_types.h  motor_math  motor_filter  motor_pi        │
│   motor_params.h  motor_limits.h  motor_hw_if.h (iface)    │
└────────────────────────────────────────────────────────────┘
```

Rules the implementation model must enforce mechanically:

- Layer A/B files: `#include` only C standard headers (`stdint.h`, `stdbool.h`, `stddef.h`, `string.h`) and other Layer A/B headers.
- `motor_hw_if.h` declares the hardware interface (pure C functions + callback registration). Layer C implements it on target; the test harness implements it on host (mock).
- `drv8323.h/.c` is split logically: register map, pack/unpack, config computation, fault decode = portable (Layer B); byte transport = a 2-function callback interface (`spi_transfer`, `delay_us` etc.) supplied by Layer C.
- `constants.h` is included **only** by Layer C/D files. A CI grep check enforces this (Section 11.7).

### 3.2 Runtime partitioning (ISR/task split)

| Context | Rate | Owner file | Work |
|---|---|---|---|
| **Fast loop ISR** (ADC end-of-injected-conversion / TIM update) | 20 kHz (= PWM) | `isr_motor.c` → `motor_ctrl_fast_loop()` | Read raw ADC, offset-correct & scale currents + Vbus, Clarke, observer update, Park, Id/Iq PI, decoupling (optional), voltage limit, inverse Park, SVPWM, write compare registers, fast overcurrent/overvoltage checks, state-machine fast actions (align/open-loop angle generation, ident excitation), telemetry snapshot |
| **Torque/current task** | 1 kHz | `freertos_tasks.c` → `motor_ctrl_current_task_step()` | Iq/Id reference management (rate limits, field-weakening hook — stub for SM-PMSM), slow protection (I²t, temperature), ident/state-machine slow steps, DRV8323 periodic fault-register poll |
| **Speed task** | 1 kHz (decimatable to 500 Hz) | `freertos_tasks.c` → `motor_ctrl_speed_task_step()` | Speed PI → Iq reference, speed ramp generation, transition supervision |
| **Supervisor task** | 100 Hz | `freertos_tasks.c` → `motor_ctrl_supervisor_step()` | State-machine slow transitions, timeouts, command queue processing, fault-manager escalation, parameter commit, logging |
| **DRV/fault ISR** | async | `isr_motor.c` (EXTI on nFAULT) | Immediate PWM kill via `hw_if` emergency stop, latch fault, notify supervisor |

Ownership rule: **only the fast-loop ISR writes PWM compare values and reads phase currents.** Tasks communicate with the ISR exclusively through the shared control block with defined single-writer fields (Section 9.4).

### 3.3 Data flow (one fast-loop cycle)

```
TIM1 update ──trigger──▶ ADC injected (Ia,Ib,Ic,Vbus)
                              │ EOC IRQ
                              ▼
        isr_motor.c: read raw → hw_if abstracts registers
                              ▼
        offsets/scaling (board calib) → i_abc [Q16.16 A], vbus [Q16.16 V]
                              ▼
        Clarke → iαβ ─▶ Observer(vαβ_prev, iαβ) → θ̂, ω̂, confidence
                              ▼
        angle select: θ_forced (align/open-loop/ident) or θ̂ (closed loop)
                              ▼
        Park → id,iq → Id/Iq PI (anti-windup, Vdq limit) → vd,vq
                              ▼
        inverse Park → vαβ (stored for next observer step) → SVPWM
                              ▼
        duty a,b,c → hw_if_pwm_set_compare() → TIM1 CCR1..3
```

---
## 4. Repository layout

```
foc-lib/
├── README.md
├── LICENSE
├── CMakeLists.txt                  # top level: selects target vs host build
├── cmake/
│   ├── toolchain-arm-gcc.cmake
│   └── host.cmake
├── include/foc/                    # public headers, portable (Layers A+B)
│   ├── motor_types.h
│   ├── motor_params.h
│   ├── motor_limits.h
│   ├── motor_math.h
│   ├── motor_filter.h
│   ├── motor_pi.h
│   ├── motor_faults.h
│   ├── motor_observer.h
│   ├── motor_foc.h
│   ├── motor_ident.h
│   ├── motor_modeler.h
│   ├── motor_tune.h
│   ├── motor_sm.h
│   ├── motor_ctrl.h
│   ├── motor_hw_if.h               # portable HW interface (declarations only)
│   └── drv8323.h                   # register logic + transport callback iface
├── src/core/                       # portable implementations (host-buildable)
│   ├── motor_math.c
│   ├── motor_filter.c
│   ├── motor_pi.c
│   ├── motor_faults.c
│   ├── motor_observer.c
│   ├── motor_foc.c
│   ├── motor_ident.c
│   ├── motor_modeler.c
│   ├── motor_tune.c
│   ├── motor_sm.c
│   ├── motor_ctrl.c
│   └── drv8323.c                   # pack/unpack/config/fault-decode + calls
│                                   #   injected transport callbacks only
├── src/port/stm32u5/               # Layer C+D, target only
│   ├── constants.h                 # THE ONLY bridge to Cube/HAL symbols
│   ├── board_support.h
│   ├── board_support.c             # all HAL calls live here
│   ├── motor_hw_if.c               # implements motor_hw_if.h via board_support
│   ├── drv8323_transport.c         # SPI/GPIO/delay callbacks for drv8323
│   ├── isr_motor.c                 # ADC EOC / TIM / EXTI nFAULT handlers
│   └── freertos_tasks.c            # CMSIS-RTOS v2 task + object creation
├── examples/stm32u5_nucleo/        # reference integration (CubeMX project not
│   └── ...                         #   part of library; shows wiring pattern)
├── test/
│   ├── unit/                       # host unit tests (Unity or CTest asserts)
│   │   ├── test_motor_math.c
│   │   ├── test_motor_filter.c
│   │   ├── test_motor_pi.c
│   │   ├── test_transforms.c
│   │   ├── test_trig.c
│   │   ├── test_limits_sat.c
│   │   ├── test_modeler_match.c
│   │   └── test_drv8323_regs.c
│   ├── module/                     # host module tests with synthetic traces
│   │   ├── test_observer_convergence.c
│   │   ├── test_ident_synthetic.c
│   │   ├── test_sm_transitions.c
│   │   ├── test_faults.c
│   │   ├── test_startup_transition.c
│   │   └── test_tune_sequences.c
│   ├── sim/                        # SIL harness
│   │   ├── sim_motor_model.c/.h    # float PMSM model (HOST ONLY)
│   │   ├── replay.c/.h             # deterministic vector replay driver
│   │   └── hw_if_mock.c            # host implementation of motor_hw_if.h
│   ├── vectors/                    # shared CSV/binary test vectors
│   │   ├── observer_ramp_1krpm.csv
│   │   ├── ident_rs_step.csv
│   │   └── ...
│   ├── reference/                  # float reference implementations (HOST ONLY)
│   │   ├── ref_math.c              # float trig/transforms/PI for tolerance cmp
│   │   └── ref_observer.c
│   └── hil/                        # on-target bring-up test procedures + code
│       ├── hil_checklist.md
│       └── hil_tests.c             # ADC offset, DRV comms, PWM seq, no-load run
└── tools/
    ├── gen_trig_lut.py             # generates sine LUT table source (build-time)
    └── vector_gen.py               # generates test/vectors from float model
```

Build rules:

- **Host build** (`cmake -DFOC_HOST=1`): compiles `include/foc` + `src/core` + `test/**`. Must compile with `-Wall -Wextra -Werror` under gcc/clang on x86-64. No HAL, no FreeRTOS, no `constants.h`.
- **Target build**: compiles `include/foc` + `src/core` + `src/port/stm32u5` as a static library `libfoc.a`; the application's Cube project links it and provides HAL init.
- CI job order: host unit → host module → SIL replay → target compile check.

---

## 5. Module specifications

Conventions used below: all APIs return `foc_status_t` (`FOC_OK`, `FOC_EINVAL`, `FOC_EBUSY`, `FOC_EFAULT`, `FOC_ETIMEOUT`, `FOC_ENOTREADY`) unless stated. All context structs are caller-allocated; no dynamic allocation anywhere in Layers A–C. All init functions take a `const *config` and fully initialize the context (no reliance on zeroed BSS). Functions marked **[ISR]** must be ISR-safe: no blocking, no RTOS calls, bounded WCET.

### 5.1 `motor_types.h` (Layer A, header only)

- **Purpose:** Single home for fixed-point typedefs, common value structs, status codes, and compile-time helpers. No functions with side effects.
- **Major types:**
  - `typedef int32_t q16_t;` — Q16.16 general-purpose signed.
  - `typedef int16_t q15_t;` — Q1.15 for trig outputs / per-unit interop.
  - `typedef int32_t q31_t;` — Q1.31 intermediate (PI integrators, filter states).
  - `typedef uint32_t angle_t;` — electrical angle, full turn = 2^32 (wraps naturally).
  - `typedef q16_t amps_q16_t, volts_q16_t, radps_q16_t, ohm_q16_t, mh_q16_t, mwb_q16_t;` — documentation typedefs (see scaling table §7.2). `radps_q16_t` carries **mechanical** rad/s; electrical rad/s is derived transiently (§7.2 speed row) so motors >50 krpm at high pole counts do not overflow Q16.16.
  - `foc_abc_t {q16_t a,b,c;}`, `foc_ab_t {q16_t alpha,beta;}`, `foc_dq_t {q16_t d,q;}`.
  - `foc_status_t` enum; `foc_bool_t`.
  - Macros: `Q16(x)` compile-time constant from literal, `Q16_ONE`, `Q16_MAX/MIN`, `ANGLE_FROM_DEG(d)`, `static_assert` guards on type sizes.
- **Dependencies:** `stdint.h`, `stdbool.h` only.
- **Tests:** covered indirectly by `test_motor_math.c` (macro correctness, rounding of `Q16()` constants).

### 5.2 `motor_math.h/.c` (Layer A)

- **Purpose:** Fixed-point arithmetic kernel and trig; the only place saturating mul/div and trig live.
- **Public API (all [ISR], pure, no state):**
  - `q16_t q16_mul(q16_t a, q16_t b);` — 64-bit intermediate, round-to-nearest, saturate.
  - `q16_t q16_div(q16_t a, q16_t b);` — 64-bit numerator shift, saturate; `b==0` → saturated result of sign(a), sets no errno (document).
  - `q16_t q16_mul_ns(q16_t,q16_t);` — non-saturating fast variant for pre-proven ranges (used inside FOC hot path; every call site must carry a range-proof comment).
  - `q16_t q16_abs/q16_min/q16_max/q16_clamp(q16_t v, q16_t lo, q16_t hi);`
  - `q15_t foc_sin(angle_t theta); q15_t foc_cos(angle_t theta);` — quarter-wave LUT (256 entries q15) + linear interpolation; max abs error ≤ 4 LSB q15.
  - `void foc_sincos(angle_t, q15_t *s, q15_t *c);`
  - `angle_t foc_atan2(q16_t y, q16_t x);` — CORDIC-style or LUT+octant; max error ≤ 0.05° electrical (needed by observer PLL error and ident).
  - `q16_t q16_sqrt(q16_t x);` — integer Newton, for magnitude/limit calcs (task rate only; document WCET).
  - `q16_t q16_from_raw_scaled(int32_t raw, q16_t gain, q16_t offset);` — helper for ADC scaling.
- **Internal:** `foc_sin_lut[257]` generated by `tools/gen_trig_lut.py` into a checked-in `.c` table (regeneration documented in README).
- **Dependencies:** `motor_types.h`.
- **Tests before integration:** `test_motor_math.c`, `test_trig.c` (Section 11.1): exhaustive edge saturation cases, randomized 1e6-sample compare vs double reference with per-op tolerance; overflow behavior asserted.

### 5.3 `motor_filter.h/.c` (Layer A)

- **Purpose:** First-order IIR low-pass and high-pass filters in fixed point, coefficient computed offline/at init from cutoff and sample rate.
- **Types:** `foc_lpf_t {q31_t state; q16_t alpha;}`, `foc_hpf_t {q31_t state_in, state_out; q16_t alpha;}`.
- **API:**
  - `foc_status_t foc_lpf_init(foc_lpf_t*, q16_t cutoff_hz, q16_t sample_hz);` — computes `alpha = 2π·fc·Ts / (1 + 2π·fc·Ts)` in fixed point at init (integer math only; document the formula and rounding).
  - `q16_t foc_lpf_step(foc_lpf_t*, q16_t x);` **[ISR]** — q31 internal accumulator to avoid limit cycles.
  - `void foc_lpf_reset(foc_lpf_t*, q16_t value);`
  - Same trio for `foc_hpf_*`.
- **Dependencies:** `motor_types.h`, `motor_math.h`.
- **Tests:** `test_motor_filter.c`: step response time-constant within ±5% of analytic; DC gain 1±0.5% (LPF) / 0±1 LSB drift over 1e6 samples (HPF); no limit cycle at zero input.

### 5.4 `motor_pi.h/.c` (Layer A)

- **Purpose:** PI controller with back-calculation anti-windup, output saturation, bumpless re-init.
- **Type:** `foc_pi_t { q16_t kp, ki_ts, kaw; q31_t integ; q16_t out_min, out_max; q16_t out_last; }` (`ki_ts` = ki pre-multiplied by Ts at init).
- **API:**
  - `foc_status_t foc_pi_init(foc_pi_t*, const foc_pi_cfg_t*);` (cfg: kp, ki, ts_hz, limits, kaw).
  - `q16_t foc_pi_step(foc_pi_t*, q16_t err);` **[ISR]** — order: P + I(q31) → sat → back-calc `integ += kaw·(sat_out − raw_out)`.
  - `void foc_pi_reset(foc_pi_t*, q16_t preload_out);` — bumpless: preloads integrator so first output equals `preload_out` at zero error (required at open-loop→closed-loop transition).
  - `void foc_pi_set_limits(foc_pi_t*, q16_t lo, q16_t hi);` **[ISR]** (dynamic Vq limit from Vd).
- **Tests:** `test_motor_pi.c`: step tracking vs float reference (tolerance §11.6), windup demonstration (limit hit, error reverses, recovery time bounded), bumpless reset property asserted exactly.

### 5.5 `motor_params.h` (Layer A, header only)

- **Purpose:** Runtime parameter set structs and defaults. Pure data definitions; no HAL, no Cube symbols.
- **Major types:**
  - `foc_motor_params_t { q16_t rs_ohm; q16_t ls_mh; q16_t lambda_m_mwb; uint8_t pole_pairs; q16_t rated_current_a; q16_t max_speed_radps_m; }` — speed is **mechanical** rad/s (Q16.16); electrical values are computed transiently as 64-bit intermediates (`ω_e = ω_m × pole_pairs`) and never stored in q16_t.
  - `foc_tuning_t { foc_pi_cfg_t pi_id, pi_iq, pi_speed; q16_t obs_gain_g1, obs_gain_g2; q16_t pll_kp, pll_ki; q16_t lpf_speed_hz, lpf_vbus_hz, hpf_bemf_hz; }`
  - `foc_startup_t { q16_t align_current_a; uint16_t align_ms; q16_t ol_current_a; q16_t ol_accel_radps2_m; q16_t ol_target_radps_m; q16_t trans_min_conf; uint16_t trans_hold_ms; uint16_t trans_blend_ms; }` — accel/target in **mechanical** rad/s (rad/s²); the forced-angle integrator converts to electrical angle increment per fast loop via `Δθ_e = ω_m × pole_pairs × Ts` (64-bit intermediate at the conversion point only).
  - `foc_params_t` = aggregation of the three above + `motor_limits_t`.
- **Rule:** every field documents its Q format and unit in a comment. Defaults provided as `FOC_PARAMS_DEFAULT` macro (safe conservative values for a generic 24 V gimbal-class motor).
- **Tests:** compile-time `static_assert` on struct sizes; consumed by all module tests.

### 5.6 `motor_limits.h` (Layer A, header only)

- **Purpose:** Protection thresholds + saturation policy constants.
- **Type:** `motor_limits_t { q16_t i_phase_max_a; q16_t i_phase_trip_a; q16_t vbus_min_v, vbus_max_v; q16_t temp_max; q16_t modulation_max; /*Q16, ≤ 0.95*/ q16_t i2t_limit, i2t_leak; uint16_t oc_trip_count; }`.
- Helper inline: `q16_t foc_vdq_limit(q16_t vbus)` returns `modulation_max · vbus / √3` (circle limit), fixed point.
- **Tests:** `test_limits_sat.c`: circle limiting preserves angle (vd priority policy: clamp vq to remaining circle), trip thresholds exact.

### 5.7 `motor_faults.h/.c` (Layer B)

- **Purpose:** Central fault manager: bitmask latch, debounce counters, severity classes, restart policy input.
- **Types:** `foc_fault_t` bit enum: `OC_HW (DRV nFAULT), OC_SW, OV_BUS, UV_BUS, OT, OBS_LOST, IDENT_FAIL, ADC_CAL_FAIL, DRV_SPI_FAIL, SM_TIMEOUT, WDG_MISSED_LOOP`. Severity map: `FATAL_LATCH` (needs explicit user clear), `AUTO_RETRY` (bounded auto-restart), `WARNING`.
- **API:** `foc_faults_init(ctx, const map)`, `foc_faults_raise(ctx, fault)` **[ISR]**, `foc_faults_raise_debounced(ctx, fault, count_limit)` **[ISR]**, `foc_faults_active(ctx)` **[ISR]**, `foc_faults_clear(ctx, mask)`, `foc_faults_retry_allowed(ctx)` (consults per-fault retry counters + cooldown ticks).
- **Ownership:** raise from ISR allowed (single 32-bit atomic OR via `__atomic` builtins on host / LDREX-free plain 32-bit store-or with IRQ-safe wrapper provided by `motor_hw_if.h` critical-section pair).
- **Tests:** `test_faults.c`: debounce exactness, severity routing, retry budget exhaustion → FATAL escalation, clear semantics.

### 5.8 `motor_hw_if.h` (Layer A header) / `motor_hw_if.c` (Layer C impl)

- **Purpose:** THE portable hardware boundary. Header lives with the core and contains **no HAL types** — only stdint. Target implements it via `board_support`; host tests implement it in `test/sim/hw_if_mock.c`.
- **Public API (all [ISR]-safe unless noted):**
  - `void hw_pwm_set_compare(uint16_t ca, uint16_t cb, uint16_t cc);`
  - `uint16_t hw_pwm_period_ticks(void);`
  - `void hw_pwm_outputs_enable(void); void hw_pwm_outputs_disable(void);` — disable = force all outputs low/hi-Z via break/MOE mechanism; must be callable from ISR and be the emergency-stop primitive.
  - `void hw_adc_read_raw(hw_adc_raw_t *out);` — fills `{uint16_t ia,ib,ic,vbus,temp}` from latest injected results.
  - `q16_t hw_vbus_scale(uint16_t raw); void hw_current_scale3(const hw_adc_raw_t*, const hw_cal_t*, foc_abc_t *out_amps);` — implemented portably? **No** — scaling constants come from Layer C; but to keep math testable, these two are thin: they call into `motor_math` with gains provided by `hw_cal_t` (struct defined here, filled by board layer / calibration).
  - **`hw_cal_t` (normative, user-tunable hardware calibration):** defined in `motor_hw_if.h` (Layer A, stdint-only) so the scaling math is host-testable. Every field carries unit + Q format:
    ```c
    typedef struct {
        /* Per-phase current channels (a, b, c) */
        q16_t  i_gain[3];       /* A per ADC LSB, Q16.16 — from shunt, CSA gain, ADC FS; per-channel to absorb component tolerance */
        int16_t i_offset_raw[3];/* raw ADC counts at zero current — measured by CALIBRATE (§6.7), user-overridable seed */
        int8_t  i_polarity[3];  /* +1/−1 — board layout may invert a shunt sense pair */
        /* Bus voltage */
        q16_t  vbus_gain;       /* V per ADC LSB, Q16.16 — from divider ratio + ADC reference */
        q16_t  vbus_offset_v;   /* V, Q16.16 — divider/opamp offset trim, default 0 */
        /* Optional temperature channel */
        q16_t  temp_gain;       /* °C per LSB, Q16.16 (0 = channel unused) */
        q16_t  temp_offset_c;   /* °C, Q16.16 */
    } hw_cal_t;
    ```
    **Fill order (normative):** (1) compile-time defaults derived in Layer C from `constants.h` board values (`BSP_SHUNT_MOHM`, `BSP_CSA_GAIN_DEFAULT`, `BSP_VBUS_DIVIDER_*`, `BSP_ADC_FULLSCALE`, ADC VREF) via `drv8323_amps_per_lsb()`; (2) **user hardware-specific overrides** — the application may supply a full or partial `hw_cal_t` through `foc_status_t hw_cal_set(const hw_cal_t*)` / `hw_cal_get(hw_cal_t*)` (task context, applied only in IDLE/FAULT, same commit discipline as `foc_ctrl_apply_params`) to account for measured board realities (actual shunt value, divider tolerance, op-amp gain error, sense-line inversion); (3) runtime offset calibration (§6.7) refines `i_offset_raw` and validates against `cal_max_residual_a`. Plausibility guards at `hw_cal_set`: gains > 0, |offsets| within configured bounds, polarity ∈ {+1,−1}; violation → `FOC_EINVAL`. Persisted storage of user cal values is the application's responsibility (same rule as ident results, §6.4).
  - `uint32_t hw_cycles_now(void);` (DWT cycle counter) — WCET measurement.
  - `void hw_crit_enter(void); void hw_crit_exit(void);` — IRQ-mask critical section used by fault manager and control-block snapshots.
  - Non-ISR: `foc_status_t hw_gpio_drv_enable(bool on);`, `bool hw_gpio_drv_nfault(void);`, `foc_status_t hw_delay_us(uint32_t);`
- **Tests:** mock implementation in `test/sim/hw_if_mock.c` records calls into ring buffers; asserted by integration-style host tests (e.g., SM emergency stop must call `hw_pwm_outputs_disable` before anything else). `hw_cal_t` scaling math (per-channel gain/offset/polarity, vbus divider) host-tested with known raw→engineering-unit vectors; `hw_cal_set` plausibility-guard rejection cases (zero/negative gain, bad polarity) covered.

### 5.9 `drv8323.h/.c` (Layer B logic + injected transport)

- **Purpose:** Complete DRV8323 driver: register map, config computation, bring-up sequence, fault decode, CSA configuration. Transport-agnostic.
- **Types:**
  - `drv8323_regs_t` — bitfield-free explicit shift/mask encode/decode for registers 0x00–0x06 (FAULT1, FAULT2/VGS, DRIVER_CTRL, GATE_HS, GATE_LS, OCP_CTRL, CSA_CTRL).
  - `drv8323_cfg_t { pwm_mode (6x), idrive_p/n_hs, idrive_p/n_ls, tdrive, dead_time_src, ocp_mode, ocp_deg, vds_lvl, csa_gain (5/10/20/40), csa_vref_div2, sense_ocp_lvl, ... }` — every field an enum matching datasheet codes.
  - `drv8323_transport_t { int (*xfer16)(uint16_t tx, uint16_t *rx); void (*delay_us)(uint32_t); void (*enable_pin)(bool); bool (*nfault_pin)(void); }` — supplied by Layer C (`drv8323_transport.c`).
  - `drv8323_faults_t` decoded struct (per-FET VDS, OCP, GDF, UVLO, OTSD, CPUV, VGS bits).
- **API:**
  - `uint16_t drv8323_pack(reg_id, const drv8323_regs_t*);` / `drv8323_unpack(...)` — pure, host-testable.
  - `foc_status_t drv8323_init(drv8323_t*, const drv8323_transport_t*, const drv8323_cfg_t*);` — sequence: enable pin high → wait ≥1 ms → write all ctrl regs → read-back verify each (retry ×3) → clear faults → verify nFAULT deasserted. Any mismatch → `FOC_EFAULT` + detail code.
  - `foc_status_t drv8323_read_faults(drv8323_t*, drv8323_faults_t*);` (task context only).
  - `foc_status_t drv8323_set_csa_gain(...)`, `drv8323_cal_csa(...)` (drives CSA_CAL bits for amplifier offset calibration — coordinate with ADC offset routine §6.7).
  - `q16_t drv8323_amps_per_lsb(const drv8323_cfg_t*, q16_t shunt_mohm, q16_t vref_v, uint16_t adc_fullscale);` — computes current scaling used to fill `hw_cal_t`. Pure, host-testable.
- **Tests:** `test_drv8323_regs.c`: pack/unpack round-trip all fields; init sequence against scripted mock transport (verify order, verify read-back mismatch → fail); fault word decode vectors from datasheet examples; amps-per-lsb math vs float reference.

### 5.10 `motor_foc.h/.c` (Layer B)

- **Purpose:** Stateless-ish FOC core: transforms, current regulators, voltage limiting, SVPWM.
- **Type:** `foc_core_t { foc_pi_t pi_id, pi_iq; q16_t vd_out, vq_out; foc_ab_t v_ab_out; uint16_t duty[3]; q16_t mod_index; }`
- **API (all [ISR]):**
  - `void foc_clarke(const foc_abc_t*, foc_ab_t*);` (2-current or 3-current balanced variant, selected by flag).
  - `void foc_park(const foc_ab_t*, q15_t sin, q15_t cos, foc_dq_t*);` / `foc_ipark(...)`.
  - `void foc_current_step(foc_core_t*, const foc_dq_t *i_meas, const foc_dq_t *i_ref, q16_t vdq_limit);` — runs both PIs, applies circle limit (vd priority), stores vd/vq.
  - `void foc_svpwm(const foc_ab_t *v_ab, q16_t vbus, uint16_t period_ticks, uint16_t duty_out[3]);` — min-max injection SVPWM; document sector math; output clamped to [min_pulse, period−min_pulse] to honor low-side sampling window (min pulse from `motor_params`).
  - `void foc_core_init/reset(...)` (reset used at every state entry that re-engages current control; supports PI preload for bumpless transition).
- **Tests:** `test_transforms.c` (Clarke/Park round-trip ≤ 2 LSB error vs float over randomized vectors; power invariance choice documented — use amplitude-invariant), SVPWM unit vectors (six sector boundary cases + zero vector + overmodulation clamp), current-step behavior with scripted PI.

### 5.11 `motor_observer.h/.c` (Layer B)

- **Purpose:** Sensorless rotor angle/speed estimation. Two cooperating parts: (1) back-EMF estimator in αβ (flux-model form), (2) PLL angle/speed tracker. Plus confidence metric.
- **Algorithm contract (must be implemented exactly as specified):**
  1. EMF estimate: `e_ab = v_ab_applied(prev cycle) − rs·i_ab − ls·di_ab/dt`, with `di/dt` as backward difference filtered by a fixed LPF (fc ≈ 1–2 kHz, param); alternatively integrate flux: `ψ_ab = ∫(v−rs·i)dt − ls·i_ab` with HPF (fc from tuning, default 2 Hz) to kill integrator drift. **Implement the flux form** (better low-frequency noise behavior); the HPF is the drift-mitigation mechanism and its phase error vs speed must be documented and compensated by a speed-dependent phase advance term (LUT of 8 breakpoints, params).
  2. Angle from flux: `θ_flux = atan2(ψ_beta, ψ_alpha)`; electrical angle `θ̂` tracked by PLL: `err = wrap(θ_flux − θ̂)` (use q31 angle subtraction, naturally wrapping), `ω̂ += pll_ki·err·Ts`, `θ̂ += (ω̂ + pll_kp·err)·Ts`.
  3. Speed output: the PLL tracks **electrical** angle/speed internally, but its electrical ω accumulator is held in a wide (q31/64-bit-updated) internal state, never in a `radps_q16_t`. The exported speed getter divides by `pole_pairs` and returns **mechanical** rad/s Q16.16 (`ω̂_m = ω̂_e / pp`), LPF-filtered (tuning.lpf_speed_hz) for the speed loop; raw internal `ω̂_e` used for the PLL update only. High-pole-count/high-rpm machines (ω_e beyond ±32767 rad/s) therefore work without saturation.
  4. Confidence: `conf = f(|ψ| vs expected λm, |pll err| LPF)`; expected flux = `lambda_m_mwb`; confidence in Q16 [0,1]; below `trans_min_conf` in CLOSED_LOOP for `obs_lost_ms` → raise `OBS_LOST`.
- **Type:** `foc_obs_t { q31_t psi_a, psi_b; foc_hpf_t hpf_a, hpf_b; angle_t theta; q31_t omega_e_int; /* wide internal electrical ω accumulator */ q16_t omega_m; /* exported mechanical rad/s Q16.16 */ foc_lpf_t lpf_omega, lpf_err; q16_t conf; foc_ab_t v_prev; }`
- **API:** `foc_obs_init(obs, const foc_motor_params_t*, const foc_tuning_t*, q16_t ts_s)`, `void foc_obs_step(foc_obs_t*, const foc_ab_t *i_ab, const foc_ab_t *v_ab_applied_prev)` **[ISR]**, `foc_obs_reset(obs, angle_t seed_theta, q16_t seed_omega)` (seeded from open-loop generator at TRANSITION), getters for θ̂/ω̂/conf (inline).
- **Explicit limitation, restated for the implementer:** below ~5–10% rated speed the EMF/flux signal is too small; the observer output is **not** used for commutation there. Startup relies on forced align + open-loop ramp; the observer runs in shadow mode during OPEN_LOOP_RAMP so its state converges before handoff.
- **Tests:** `test_observer_convergence.c` (Section 11.2): synthetic PMSM traces (from `sim_motor_model`) at 10/25/50/100% speed — angle error after convergence ≤ 3° electrical RMS at ≥25% speed, ≤ 8° at 10%; convergence time from seeded start ≤ 100 ms; drift test: 60 s constant-speed replay, no unbounded flux growth; confidence drops below threshold within 50 ms when EMF is zeroed in the trace.

### 5.12 `motor_ident.h/.c` (Layer B)

- **Purpose:** Parameter identification executed by the state machine's IDENTIFY state. Produces `foc_motor_params_t` candidate + quality report.
- **Sub-procedures (sequenced internally, each with timeout + quality gate):**
  1. **Rs:** lock θ_forced = 0, close current loop on d-axis at `ident.i_rs_a` (default 30% rated), wait settle (LPF'd |di/dt| below threshold), average `vd/id` over `rs_avg_ms` (default 200 ms). Repeat at second current level; Rs = slope; linearity check = quality metric. Compensate known inverter drop model (dead-time + Rds(on) constant from params) — document as first-order correction.
  2. **Ls:** with rotor still locked, superimpose square-wave vd excitation at `ident.ls_freq_hz` (default 1 kHz) amplitude `ident.v_ls`; measure fundamental di amplitude via Goertzel-style correlation in fixed point; `Ls = V/(2π·f·I)` minus Rs effect. SM-PMSM: report single Ls (Ld=Lq=Ls). Quality = SNR of correlation.
  3. **λm/Ke:** spin open-loop at `ident.ke_speed_radps_m` (mechanical rad/s; well above observer floor, e.g. 30% rated) with current regulated; run observer in shadow; `λm = |ψ|` averaged over `ke_avg_ms`, cross-checked with `(vq − rs·iq)/ω`. Quality = agreement of the two estimates (≤10% ⇒ pass).
- **API:** `foc_ident_init(ctx, params_in, tuning, limits)`, `foc_ident_fast_step(ctx, meas, foc_outputs*)` **[ISR]** (generates excitation references while IDENTIFY active), `foc_ident_slow_step(ctx)` (1 kHz task: sequencing, averaging, quality gates), `foc_ident_result(ctx, foc_motor_params_t *out, foc_ident_quality_t *q)`, `foc_ident_abort(ctx)`.
- **Quality/acceptance logic:** each stage returns `{value, quality_q16, pass}`; overall pass requires all stages pass AND values within plausibility bounds (Rs ∈ [5 mΩ, 20 Ω], Ls ∈ [1 µH, 50 mH], λm ∈ bounds derived from rated speed/voltage). On fail → `IDENT_FAIL` fault (AUTO_RETRY once, then FATAL) and fall back to profile lookup if enabled.
- **Tests:** `test_ident_synthetic.c`: replay vectors generated from float model with known Rs/Ls/λm (+ noise + ADC quantization): estimates within ±5% (Rs), ±10% (Ls), ±5% (λm); quality gates reject vectors with injected disturbance (e.g., rotor moves during Rs stage).

### 5.13 `motor_modeler.h/.c` (Layer B)

- **Purpose:** Motor profile table + matching. One `const` table of candidate profiles; runtime params come either from measured identification or from best-match profile.
- **Types:** `foc_motor_profile_t { const char *name; foc_motor_params_t params; foc_tuning_t tuning; foc_startup_t startup; motor_limits_t limits; }`; `foc_profile_table_t { const foc_motor_profile_t *entries; size_t count; }`.
- **API:**
  - `const foc_motor_profile_t* foc_modeler_match(const foc_profile_table_t*, const foc_motor_params_t *measured, q16_t *match_score_out);` — normalized distance over (Rs, Ls, λm) with per-quantity log-scaled weights; score ≥ `MODELER_MIN_SCORE` (param) required.
  - `foc_status_t foc_modeler_resolve(const foc_profile_table_t*, const foc_ident_result*, foc_param_source_t policy, foc_params_t *out);` — policy ∈ {MEASURED_ONLY, PROFILE_ONLY(index), MEASURED_THEN_PROFILE, PROFILE_TUNING_MEASURED_PLANT (use measured Rs/Ls/λm but profile's PI/observer gains scaled by ratio rules)}. Scaling rules: PI current gains recomputed from measured Ls, Rs and desired bandwidth `f_bw_current` (default 1 kHz): `kp = 2π·f_bw·Ls`, `ki = 2π·f_bw·Rs` (document fixed-point computation path at init — 64-bit intermediates allowed at init time).
  - `foc_modeler_default_table()` — ships with ≥4 profiles: small gimbal, mid drone, 48 V eBike class, plus **`user_motor_01`** (normative reference motor, must be matchable by `foc_modeler_match` from IDENTIFY output):
    - `user_motor_01`: Rs = 0.149 Ω per phase; Ls = 96 mH; back-EMF constant Ke = 0.982 V_peak/Hz (electrical) → λm = 0.982/(2π) ≈ **156.34 mWb** (store as `lambda_m_mwb = Q16(156.34)`); `pole_pairs = 1`; bus voltage 12 V class; current limit 7 A (`i_phase_max_a = Q16(7.0)`, trip margin per limits policy, e.g. `i_phase_trip_a = Q16(8.0)`); rated current ≤ 7 A. With 1 pole pair, electrical speed = mechanical speed. Note for tuning derivation: Ls = 96 mH is a high-inductance machine — current-loop `kp = 2π·f_bw·Ls` will be large and `f_bw_current` may need reduction below the 1 kHz default to keep vd/vq within the 12 V bus; document the chosen f_bw in the profile entry.
- **Tests:** `test_modeler_match.c`: exact-match returns right profile with high score; ambiguous input returns best with low score below threshold; resolve() gain-recompute matches float reference within tolerance; policy matrix covered; **`user_motor_01` acceptance case**: synthetic ident result at (0.149 Ω, 96 mH, 156.34 mWb) ± ident tolerances (§5.12: Rs ±5%, Ls ±10%, λm ±5%) must match `user_motor_01` with score ≥ `MODELER_MIN_SCORE`.

### 5.14 `motor_sm.h/.c` (Layer B)

- **Purpose:** The main motor state machine (Section 8 is the authoritative spec). Pure logic: consumes inputs struct, emits outputs/actions struct; no HAL, no RTOS.
- **API:** `foc_sm_init(sm, const foc_params_t*)`, `foc_sm_input(sm, const foc_sm_in_t*)` (commands, faults, ident status, observer conf, timers), `foc_sm_step_fast(sm)` **[ISR]** (only what must be cycle-accurate: angle-source selection flag, open-loop angle integration control, emergency transitions on fatal fault bit), `foc_sm_step_slow(sm, uint32_t ms_now)` (timeouts, sequenced transitions), `foc_sm_state(sm)`, `foc_sm_actions(sm)` → bitset consumed by `motor_ctrl` (e.g. `ACT_PWM_ENABLE, ACT_RUN_CURRENT_LOOP, ACT_ANGLE_FORCED, ACT_OBS_SHADOW, ACT_OBS_ACTIVE, ACT_IDENT_ACTIVE, ACT_TUNE_ACTIVE, ACT_ZERO_DUTY`).
- **Tests:** `test_sm_transitions.c`: full transition matrix (Section 8 table) driven as a table-driven test; every timeout path; fault in every state → FAULT with correct actions; restart policy budget.

### 5.15 `motor_ctrl.h/.c` (Layer B)

- **Purpose:** Top-level orchestration and the library's public facade. Owns the control block that binds SM + FOC + observer + ident + faults + params; exposes the fast/slow entry points that Layer D wires to ISR/tasks; exposes the user command API.
- **Types:** `foc_ctrl_t` (aggregates all module contexts + `foc_cmd_t` latest command + telemetry snapshot struct + loop WCET stats); `foc_cmd_t { enum {CMD_STOP, CMD_RUN_SPEED, CMD_RUN_TORQUE, CMD_IDENTIFY, CMD_TUNE, CMD_CLEAR_FAULTS} op; q16_t value; foc_tune_mode_t tune_mode; }`; `foc_telemetry_t` (id, iq, vd, vq, θ̂, ω̂, conf, vbus, state, faults, wcet_cycles — single-writer by ISR, seq-lock for readers).
- **Public API:**
  - `foc_status_t foc_ctrl_init(foc_ctrl_t*, const foc_params_t*, const foc_profile_table_t*);`
  - `void foc_ctrl_fast_loop(foc_ctrl_t*);` **[ISR]** — the entire §3.3 pipeline; MUST be branch-bounded; calls `hw_*` only via `motor_hw_if.h`.
  - `void foc_ctrl_current_task_step(foc_ctrl_t*);` / `foc_ctrl_speed_task_step(foc_ctrl_t*);` / `foc_ctrl_supervisor_step(foc_ctrl_t*, uint32_t ms);`
  - `foc_status_t foc_ctrl_command(foc_ctrl_t*, const foc_cmd_t*);` (task context; validated then handed to SM at supervisor rate).
  - `void foc_ctrl_get_telemetry(foc_ctrl_t*, foc_telemetry_t*);` (seq-lock read).
  - `foc_status_t foc_ctrl_apply_params(foc_ctrl_t*, const foc_params_t*);` — double-buffered commit, swap only in IDLE/FAULT or at supervisor boundary with loops reset.
- **Tests:** host integration tests using `hw_if_mock` + `sim_motor_model`: command → PWM duty path; ident-to-runtime parameter flow; ISR/task handshake sequencing (mock ticks).

### 5.16 `board_support.h/.c` (Layer C)

- **Purpose:** All HAL calls. Implements clock/timer/ADC/SPI/GPIO operational control on top of Cube-generated init. Publishes nothing HAL-typed upward.
- **Responsibilities:** start/stop TIM1 PWM with MOE handling; configure+start injected ADC with TIM trigger; read injected results; SPI 16-bit transfer for DRV; EXTI nFAULT wiring to a registered callback; DWT init; microsecond delay; ADC/CSA offset calibration measurement helper (`bsp_measure_current_offsets(n_samples, out_raw_avgs)` executed with PWM off / 0% duty per §6.7).
- **Rule:** includes `constants.h` and HAL headers; nothing else in the repo may.
- **Tests:** HIL only (Section 11.5) + CI include-guard grep.

### 5.17 `constants.h` (Layer C, header only)

- **Purpose:** The single bridge file. Maps Cube-generated names to library-internal `BSP_*` macros. Contains: `BSP_TIM_PWM_HANDLE (&htim1)`, channel macros, `BSP_ADC_HANDLE`, injected rank mapping per phase, `BSP_SPI_DRV_HANDLE`, `BSP_GPIO_DRV_EN_PORT/PIN`, `BSP_GPIO_NFAULT_PORT/PIN/EXTI_IRQn`, DMA symbols if used, `BSP_PWM_PERIOD_TICKS`, `BSP_ADC_FULLSCALE`, `BSP_SHUNT_MOHM`, `BSP_CSA_GAIN_DEFAULT`, `BSP_VBUS_DIVIDER_*`, clock frequencies. These board values are the **compile-time seed** for `hw_cal_t` (§5.8); user hardware-specific ADC tuning values supplied at runtime via `hw_cal_set()` take precedence over the seed (nominal design values vs. measured board realities).
- **Rule:** *only* `board_support.c`, `motor_hw_if.c`, `drv8323_transport.c`, `isr_motor.c`, `freertos_tasks.c` may include it. Porting to a new board = edit this file + possibly `board_support.c`.
- **Tests:** compile check + CI grep rule (Section 11.7).

### 5.18 `drv8323_transport.c` (Layer C)

- **Purpose:** Fills `drv8323_transport_t` with HAL SPI xfer (16-bit frame, mode 1, ≤10 MHz), enable-pin GPIO, nFAULT read, µs delay — all via `board_support`/`constants.h`.
- **Tests:** HIL SPI verification (read DRIVER_CTRL after write; scripted in `hil_tests.c`).

### 5.19 `isr_motor.c` (Layer D)

- **Purpose:** Real ISR bodies: ADC injected EOC → `foc_ctrl_fast_loop()`; TIM break IRQ → fault raise; EXTI nFAULT → `hw_pwm_outputs_disable()` + fault raise + `osThreadFlagsSet` to supervisor (the *only* RTOS call allowed in Layer D ISRs, wrapped in one function). Measures WCET via `hw_cycles_now` and records max into telemetry.
- **Tests:** HIL timing capture (toggle debug pin around fast loop; scope/logic analyzer procedure in `hil_checklist.md`).

### 5.20 `freertos_tasks.c` (Layer D)

- **Purpose:** Creates all RTOS objects and tasks per Section 9; runs the internal init sequence (§6.8); owns the command queue front-end (`foc_app_command()` convenience wrapper).
- **Tests:** target integration tests (task jitter measurement, queue overflow behavior).

### 5.21 `motor_tune.h/.c` (Layer B)

- **Purpose:** User-initiated calibration/tuning mode driven by the SM TUNE state. Generates deterministic excitation sequences for torque-PI and speed-PI tuning, exposes synchronized observer signals for observer tuning, and provides a high-rate capture buffer plus a live-gain-update path. Pure logic; no HAL, no RTOS.
- **Sub-modes (`foc_tune_mode_t`):** `TUNE_TORQUE`, `TUNE_SPEED`, `TUNE_OBSERVER`.
- **Types:**
  - `foc_tune_cfg_t { /* torque */ q16_t tq_i_hi_a; /*dflt 1.0 A*/ q16_t tq_i_lo_a; /*dflt 0.5 A*/ uint16_t tq_step_ms; /*dflt 50*/ uint8_t tq_reps; /*dflt 4*/ /* speed */ q16_t sp_min_pct, sp_max_pct; /*of max_speed, dflts 20%/60%*/ uint16_t sp_dwell_ms; /*dflt 500*/ uint8_t sp_reps; /*dflt 6*/ /* observer */ q16_t obs_speed_radps_m; /*mechanical rad/s, dflt 30% rated*/ uint16_t obs_capture_ms; /* capture */ uint16_t decim; }` — every field clamped against `motor_limits_t` at init (`FOC_EINVAL` on violation).
  - `foc_capture_t` — fixed-size ring buffer (default 2048 samples × 4 × int32, compile-time `FOC_TUNE_CAP_DEPTH`), single writer (fast ISR), decimation `cfg.decim`, seq-lock/watermark drain API. Channel map depends on sub-mode (below). No dynamic allocation.
  - `foc_tune_t` — sequencer state: sub-mode, phase index, tick counters, done/abort flags, capture ctx.
- **Sequence contracts (normative):**
  - **TUNE_TORQUE:** rotor held at forced angle θ=0 (SM routes ALIGN first); speed loop **off**, id* = 0 (no flux command); iq* profile per repetition: `tq_i_hi_a` for `tq_step_ms` → `tq_i_lo_a` for `tq_step_ms` → 0 for `tq_step_ms`, repeated `tq_reps` times. Capture channels: {iq*, iq_meas, vq, id_meas} at full fast-loop rate (decim=1 default). Complete → done flag.
  - **TUNE_SPEED:** requires closed-loop operation — SM runs the normal startup path first (§6.2) with tune-pending latch, then enters TUNE. Speed reference is a square wave alternating `sp_min_pct·max_speed` ↔ `sp_max_pct·max_speed`, dwell `sp_dwell_ms`, for `sp_reps` half-cycles. The reference **bypasses the normal speed ramp generator** (step applied in one speed-task tick — the sharpest realizable reference) while iq* remains bounded by `motor_limits`; `sp_min_pct·max_speed` must be ≥ `min_run_speed` (validated at init). Capture channels: {ω*, ω̂_filt, iq*, iq_meas} decimated to 1 kHz.
  - **TUNE_OBSERVER:** SM runs startup to closed loop (tune-pending latch), holds constant speed `obs_speed_radps_m`; capture channels: {i_a_meas, e_a_est, θ̂, conf} where `e_a_est` is the estimated back-EMF projected onto phase a (`e_a = e_α` by convention, derived from observer ψ/EMF state — expose via a new `foc_obs_get_emf_ab()` inline getter added to `motor_observer.h`). This gives the user time-aligned measured phase-A current vs estimated back-EMF for phase/gain comparison while adjusting observer tuning live.
- **Live gain updates:** `foc_tune_set_gains(foc_tune_t*, const foc_tune_gains_t*)` — task-context staging of {pi_iq kp/ki, pi_speed kp/ki, obs hpf fc, pll kp/ki, lpf_speed_hz}; committed by `motor_ctrl` at the next current-task boundary with optional PI reset flag. Allowed **only while SM state == TUNE**; rejected otherwise (`FOC_ENOTREADY`). Gains changed here are volatile until the user commits them via `foc_ctrl_apply_params`.
- **API:** `foc_tune_init(ctx, mode, const foc_tune_cfg_t*, const foc_params_t*, const motor_limits_t*)`, `foc_tune_fast_step(ctx, const foc_meas_t*, foc_tune_refs_t *out)` **[ISR]** (emits iq*/id* or ω* override + capture write), `foc_tune_slow_step(ctx)` (sequencing, dwell timing), `foc_tune_done(ctx)`, `foc_tune_abort(ctx)`, `foc_tune_capture_read(ctx, dst, max, *n_out)` (task context, seq-lock consistent).
- **Dependencies:** `motor_types.h`, `motor_math.h`, `motor_params.h`, `motor_limits.h`, `motor_observer.h` (getter only).
- **Tests:** `test_tune_sequences.c` (§11.2): exact excitation timing (step edges within one fast-loop tick), amplitude exactness, repetition counts, limit clamping, capture integrity (no lost/duplicated samples across wrap, deterministic contents on replay), gain-update commit-at-boundary semantics, abort → clean zeroed references. TUNE_SPEED sub-mode also exercised in the closed SIL loop (extends `test_startup_transition.c` harness).

---

## 6. Control strategy

### 6.1 Cascaded structure and rates

| Loop | Rate | Bandwidth target | Runs in | Inputs | Outputs |
|---|---|---|---|---|---|
| Current (Id, Iq) | 20 kHz | ~1 kHz (recomputed from Ls/Rs) | Fast ISR | id/iq meas, id*/iq* | vd, vq → SVPWM |
| Torque/current supervision | 1 kHz | n/a | Current task | iq* raw, limits, I²t | iq* shaped, id* (=0 for SM-PMSM; FW hook stub) |
| Speed | 1 kHz | ~50–100 Hz | Speed task | ω̂ filtered, ω* ramped | iq* raw |
| Supervision/SM | 100 Hz | n/a | Supervisor | commands, faults, conf | state, actions |

Design rule: ≥10× rate separation between cascaded loops; PI gains for current loop derived from plant (`kp=2π·f_bw·Ls`, `ki=2π·f_bw·Rs`); speed PI from profile or conservative default with ramp-limited reference.

### 6.2 Startup sequence (sensorless)

1. **ALIGN:** force θ_forced = 0 (or `align_theta`), regulate id = ramp 0→`align_current_a` over `align_ms/2`, hold for `align_ms/2`. Rotor locks to d-axis. Guard: current loop must track within 10% by end of ramp, else `SM_TIMEOUT`.
2. **OPEN_LOOP_START:** initialize forced-angle integrator at θ_forced, ω_forced = 0; switch current vector to iq = `ol_current_a`, id = 0 **in the forced frame** (i.e., torque current in an imposed rotating frame).
3. **OPEN_LOOP_RAMP:** `ω_forced += ol_accel·Ts` each fast loop until `ω_forced ≥ ol_target` (default 15% rated, must be ≥ 1.5× observer floor). Observer runs in **shadow**: fed real iαβ and applied vαβ, produces θ̂/ω̂/conf but does not commutate.
4. **TRANSITION:** entry guard: `ω_forced ≥ ol_target` AND `conf ≥ trans_min_conf` AND `|ω̂ − ω_forced| ≤ 20%` held for `trans_hold_ms`. Action: over `trans_blend_ms` (default 100 ms), blend commutation angle `θ_used = slerp(θ_forced, θ̂, k)` with k ramped 0→1 (angle blend = shortest-path wrap-aware interpolation on `angle_t`); simultaneously hand iq* ownership to the speed PI, whose integrator is preloaded (`foc_pi_reset(pi_speed, iq_current)`) for bumpless takeover. Exit → CLOSED_LOOP_RUN. Failure (conf collapse or angle divergence > 45° during blend) → STOPPING then AUTO_RETRY per policy.
5. **CLOSED_LOOP_RUN:** full cascade active; observer confidence monitored continuously.

Low-speed operation note for the implementer: closed-loop minimum speed = `ol_target·0.6` (param `min_run_speed`); a speed command below it while running triggers controlled STOPPING (no attempt to run the observer below its floor). Reversal = STOP → restart in opposite direction.

### 6.3 Observer strategy

As specified in §5.11: flux-linkage form with HPF drift mitigation, PLL tracking, speed-dependent HPF phase compensation, confidence metric from flux magnitude agreement + PLL residual. Observer is stepped **every fast loop in every powered state** (shadow or active) so transition handoff needs no cold start.

### 6.4 Identification strategy

As specified in §5.12 (Rs two-point DC, Ls square-wave correlation, λm open-loop spin with dual estimate cross-check). IDENTIFY is a command-initiated state; results flow: `foc_ident_result` → `foc_modeler_resolve(policy)` → `foc_ctrl_apply_params` (double-buffered) → persisted by application (library exposes the struct; storage is out of scope).

### 6.5 Protection strategy

| Check | Where | Rate | Action |
|---|---|---|---|
| HW overcurrent (DRV VDS/OCP → nFAULT) | EXTI ISR | async | PWM kill ≤ 2 µs, FAULT(OC_HW), FATAL_LATCH after retry budget |
| SW overcurrent `|i_phase| > i_phase_trip_a` (debounced `oc_trip_count`) | Fast ISR | 20 kHz | PWM kill, FAULT(OC_SW), AUTO_RETRY ×2 |
| Bus OV/UV | Fast ISR (LPF'd vbus) | 20 kHz | OV: PWM kill (never brake into OV); UV: STOPPING |
| I²t thermal proxy | Current task | 1 kHz | Fold back iq*; sustained → FAULT(OT) |
| Observer lost (conf low in CLOSED_LOOP) | Current task | 1 kHz | STOPPING + FAULT(OBS_LOST), AUTO_RETRY |
| Loop-deadline watchdog (fast loop overrun / missed tick) | ISR + supervisor | mixed | FAULT(WDG_MISSED_LOOP), FATAL |
| DRV SPI readback mismatch (periodic reverify every 1 s) | Supervisor | 1 Hz | FAULT(DRV_SPI_FAIL), FATAL |

### 6.6 Voltage limiting policy

Circle limit `|v_dq| ≤ modulation_max·vbus/√3`; d-axis priority (clamp vq to remaining headroom). SVPWM min-pulse clamp guarantees the low-side sampling window (`min_lowside_ns` param → ticks at init).

### 6.7 Current-offset calibration (CALIBRATE state)

Sequence: PWM outputs disabled → DRV8323 CSA_CAL asserted (shorts CSA inputs) → average `cal_samples` (default 1024) raw readings per channel → store raw offsets in `hw_cal_t` → deassert CSA_CAL → second pass with PWM enabled at exactly 50/50/50% duty, zero vector, verify residual current reading < `cal_max_residual_a` (default 2% rated). Fail → `ADC_CAL_FAIL` (FATAL). Offsets applied in `hw_current_scale3`. This routine refines only `i_offset_raw[]`; the gain/polarity fields of `hw_cal_t` come from the `constants.h` seed and any user `hw_cal_set()` overrides (§5.8) and are never modified by CALIBRATE. If the user has supplied offset overrides, CALIBRATE still runs and its measured offsets win (with residual check as the acceptance gate).

### 6.8 Internal initialization plan (non-Cube init — normative order)

Cube-generated code initializes peripherals to a *configured but idle* state only. Everything below is created by this library, executed by `freertos_tasks.c` / `motor_ctrl`:

1. `bsp_init()` — DWT, verify Cube handles non-NULL (via `constants.h`), bind EXTI callback.
2. RTOS object creation: command queue, telemetry seq-lock init, event flags, mutexes (§9).
3. Task creation (§9) — tasks start blocked on init-done flag.
4. `drv8323_init()` — enable pin, register config, read-back verify, CSA config.
5. `foc_ctrl_init()` — params defaults/profile, sub-module inits: math LUT sanity self-check, filters, PIs, observer reset, ident ctx, modeler table bind, fault map, SM init → SM enters INIT.
6. PWM prepared: compare = 50%, outputs **disabled**, TIM started, ADC injected armed on trigger; verify first EOC interrupt arrives within 2 PWM periods (else FATAL init fail).
7. Offset calibration (§6.7) driven by SM CALIBRATE.
8. Init-done flag set → tasks run → SM to IDLE awaiting commands.
9. Controlled shutdown path: STOPPING ramps iq*→0 (rate `stop_ramp`), then ω-independent: zero vector for `stop_zero_ms`, PWM outputs disabled, DRV kept enabled; full power-down API additionally drops DRV enable pin.

### 6.9 Tuning mode (TUNE state)

User-initiated from IDLE via `CMD_TUNE(mode)`; excitation contracts, capture channel maps, and the live-gain path are normative in §5.21. Fast-loop wiring: when `ACT_TUNE_ACTIVE`, `motor_ctrl` sources iq*/id* (TORQUE) or the speed reference (SPEED) from `foc_tune_fast_step` instead of the normal cascade; TUNE_OBSERVER leaves the cascade intact and only arms capture. Capture writes occur inside the fast ISR after the SVPWM update (bounded, branch-free ring write). All protections of §6.5 remain fully active during TUNE.

---

## 7. Fixed-point design

### 7.1 Formats

| Format | Type | Use |
|---|---|---|
| Q16.16 (`q16_t`, int32) | general values | currents, voltages, speeds, params, gains |
| Q1.15 (`q15_t`, int16) | trig outputs | sin/cos, blend factors |
| Q1.31 / q31 accumulators (int32/int64 interm.) | integrators, filter states, flux integrator | PI integ, LPF/HPF state, ψαβ |
| `angle_t` (uint32, 2^32 = 1 e-rev) | all angles | wrap-free add/sub; degrees only at API edges |

### 7.2 Scaling table (units embedded in the value; **no per-unit system**, SI-in-Q16.16)

| Quantity | Unit carried | Range @ Q16.16 | Resolution | Notes |
|---|---|---|---|---|
| Current | A | ±32767 A | 15 µA | far exceeds needs; headroom for math |
| Voltage | V | ±32767 V | 15 µV | |
| Speed | rad/s (**mechanical**) | ±32767 rad/s_m | 15 µrad/s | 32767 rad/s_m ≈ 312 krpm at any pole count — motors >50 krpm supported. Electrical speed (ω_e = ω_m·pole_pairs) is **never stored** in q16_t: it is computed transiently in 64-bit at the two conversion points (angle increment `Δθ_e = ω_m·pp·Ts` in the forced-angle/observer integrators, and back-conversion `ω_m = ω_e_pll / pp` at the PLL output). Assert at init that `max_speed_radps_m` fits Q16.16 |
| Rs | Ω | 0–32k Ω | 15 µΩ | |
| Ls | **mH** | 0–32k mH | ~15 nH | stored in mH to keep resolution; conversions localized in observer/ident init (document every place the mH↔H factor 1000 appears) |
| Flux λm | **mWb** | | ~15 nWb | same mH-style rule |
| Angle | 2^32/rev | full rev | 8.4e-8 rev | |
| Duty | timer ticks (uint16) | 0–period | 1 tick | produced only by `foc_svpwm` |

### 7.3 Rules the implementer must follow

1. Every multiply of two Q16.16 values uses `q16_mul` (int64 intermediate, round-half-away, saturate) unless the call site has a written range proof and uses `q16_mul_ns`.
2. Division only via `q16_div`; never in the fast loop except the SVPWM vbus normalization, which is precomputed as reciprocal `inv_vbus` once per fast loop with one `q16_div`.
3. Mixed-format ops (q16×q15) get dedicated helpers (`q16_mul_q15`) — no ad-hoc shifts in control files.
4. Saturation is silent but counted: a per-context saturation event counter incremented by the saturating helpers via optional hook (compiled in for tests, out for release) — module tests assert zero unexpected saturations on nominal vectors.
5. Init-time computations may use 64-bit integer math freely (never float) — e.g., gain derivations, filter alpha, tick conversions.
6. Trig: 256-entry quarter-wave q15 LUT + linear interpolation (≤4 LSB error); `foc_atan2` ≤0.05° — both bounds asserted by `test_trig.c` exhaustively at 2^16 grid + random.
7. Angle arithmetic uses unsigned wraparound exclusively; signed angle differences via `(int32_t)(a−b)`.
8. Overflow verification: unit tests exercise INT32_MIN/MAX corners for every helper; module tests replay worst-case amplitude vectors (max current, max vbus) asserting saturation counter behavior.

### 7.4 Float policy

Floats appear only in `test/reference`, `test/sim`, and `tools/`. CI greps `src/` and `include/` for `float`/`double` tokens (allowlist: none) — build fails on hit.

---

## 8. State machine

States and transitions (authoritative; `motor_sm.c` implements exactly this):

| State | Entry actions | Exit condition(s) | Timeout → | Fault behavior |
|---|---|---|---|---|
| **IDLE** | PWM off, loops reset, actions=none | CMD_RUN/CMD_IDENTIFY → INIT | none | any FATAL latched blocks exits until CMD_CLEAR |
| **INIT** | verify drv8323 status, arm ADC path, reset observer/PIs/filters, load committed params | success → CALIBRATE | 200 ms → FAULT(SM_TIMEOUT) | → FAULT |
| **CALIBRATE** | §6.7 offset routine (PWM off then zero-vector pass) | residual pass → (IDENTIFY if pending cmd/no valid params, else ALIGN if run or tune pending, else IDLE) | 500 ms | ADC_CAL_FAIL → FAULT |
| **IDENTIFY** | ident ctx reset; PWM on; forced angle | ident pass → modeler resolve → params commit → IDLE (report) or ALIGN (if run pending) | per-stage (Rs 1 s, Ls 1 s, λm 3 s) | IDENT_FAIL → FAULT(AUTO_RETRY×1) |
| **ALIGN** | forced θ, id ramp | align_ms elapsed AND id tracking ≤10% err → OPEN_LOOP_START (or TUNE if tune_torque pending) | 2×align_ms | → FAULT |
| **OPEN_LOOP_START** | forced-frame iq engaged, ω_forced=0, observer reset(seed θ_forced,0) + shadow | first fast-loop cycle complete → OPEN_LOOP_RAMP | 10 ms | → FAULT |
| **OPEN_LOOP_RAMP** | ramp ω_forced | ω_forced ≥ ol_target → TRANSITION | `ol_target/ol_accel × 2` | overcurrent/UV → STOPPING |
| **TRANSITION** | blend per §6.2.4, speed PI preload | blend complete AND conf ≥ thr → CLOSED_LOOP_RUN (or TUNE if tune_speed / tune_observer pending) | trans_hold+blend ×3 → STOPPING + AUTO_RETRY | conf collapse → STOPPING |
| **CLOSED_LOOP_RUN** | full cascade | CMD_STOP or ω* < min_run_speed → STOPPING | none | per §6.5 |
| **TUNE** | per sub-mode (§5.21): TORQUE = forced-angle iq step sequence, speed loop off; SPEED = closed-loop square-wave ω* steps (ramp bypassed); OBSERVER = closed-loop constant speed, i_a/BEMF capture; capture armed; live gain updates enabled | sequence done or CMD_STOP → STOPPING → IDLE (capture retained for drain until next TUNE/RUN) | 2× nominal sequence duration → STOPPING + FAULT(SM_TIMEOUT) | per §6.5 (all run-state protections active; any fault → PWM kill → FAULT) |
| **STOPPING** | iq* ramp→0, then zero vector `stop_zero_ms`, PWM off | complete → IDLE (or FAULT if stop caused by fault) | 2 s → force PWM off → IDLE/FAULT | escalate |
| **FAULT** | PWM off (already killed), DRV faults read+logged, retry policy evaluated | AUTO_RETRY budget available + cooldown elapsed → INIT (auto) ; FATAL → wait CMD_CLEAR_FAULTS → IDLE | none | absorbing until cleared |

Restart policy: per-fault retry counters (`motor_faults`), default OC_SW×2, OBS_LOST×2, TRANSITION-fail×3, cooldown 500 ms, counters reset after 30 s of healthy CLOSED_LOOP_RUN. FATAL faults (OC_HW after budget, DRV_SPI_FAIL, ADC_CAL_FAIL, WDG) never auto-restart.

TUNE entry: `CMD_TUNE(mode)` accepted **only in IDLE** with valid committed params and no FATAL latched; the SM latches `tune_pending(mode)` and routes INIT → CALIBRATE → ALIGN → TUNE (torque) or the full startup chain → TUNE (speed/observer). CMD_TUNE in any other state → `FOC_ENOTREADY`. TUNE never auto-retries; any fault during TUNE latches per normal policy and returns to IDLE only via CMD_CLEAR.

Guards evaluated in `foc_sm_step_slow` except: fatal-fault→FAULT and angle-source switching, which are fast-path (`foc_sm_step_fast`) to bound reaction latency to one PWM period.

---

## 9. RTOS design

### 9.1 Tasks (CMSIS-RTOS v2)

| Task | Priority (osPriority) | Period/trigger | Stack | Calls |
|---|---|---|---|---|
| `tsk_current` | Realtime (highest task) | osThreadFlags from fast ISR every 20th cycle (1 kHz) | 1 KB | `foc_ctrl_current_task_step` |
| `tsk_speed` | High | osDelayUntil 1 ms | 1 KB | `foc_ctrl_speed_task_step` |
| `tsk_supervisor` | Normal | osDelayUntil 10 ms + flag on fault | 2 KB | `foc_ctrl_supervisor_step`, DRV poll, command queue drain |
| (app tasks) | ≤ Low | — | — | telemetry/UI, out of scope |

Fast loop is **not** a task; it is the ADC EOC ISR (NVIC priority above FreeRTOS `configMAX_SYSCALL_INTERRUPT_PRIORITY` — it makes **no** RTOS calls; the notification to `tsk_current` is issued from a lower-priority follow-up mechanism: implementer must set the fast ISR to a non-RTOS priority and issue the thread flag from a PENDSV-safe deferred ISR OR run the fast ISR at a maskable priority and call `osThreadFlagsSet` directly every Nth cycle — **choose the second**, simpler option, and document the ≤ few-µs jitter cost; nFAULT EXTI stays at non-maskable-by-RTOS priority and only writes registers via `hw_pwm_outputs_disable` plus an atomic fault bit, with supervisor polling picking it up within 10 ms).

### 9.2 RTOS objects

`q_cmd` (osMessageQueue, depth 8, `foc_cmd_t`), `evt_init_done` (osEventFlags), `mtx_params` (osMutex guarding the staging params buffer only — never taken by ISR), thread flags for 1 kHz kick and fault kick.

### 9.3 ISR→task handoff

Fast ISR increments cycle counter; every 20th sets thread flag to `tsk_current`. Telemetry: ISR writes `foc_telemetry_t` under a seq-lock (increment counter, write, increment); readers retry on odd/changed counter. No queues from ISR.

### 9.4 Data ownership (normative)

| Data | Writer | Readers | Mechanism |
|---|---|---|---|
| PWM compare, applied vαβ, measured iabc/idq, θ_used, observer state | fast ISR only | tasks via telemetry | seq-lock snapshot |
| iq*/id* references | speed/current tasks | fast ISR | single 32-bit aligned writes (atomic on M33), double-word pairs via seq-lock |
| Committed `foc_params_t` | supervisor (commit swap) | ISR reads active pointer | pointer swap in critical section + loops reset at swap |
| Fault bits | any (atomic OR) | all | `hw_crit_enter/exit` wrapped RMW |
| SM state | supervisor (slow) + ISR (fast fatal path) | all | uint8 atomic + action bitset regenerated per fast loop |
| Command queue | app → supervisor | supervisor | osMessageQueue |

Rule: no mutex is ever taken in the fast ISR; no field has two writers.

---

## 10. Hardware abstraction boundary

Already constrained in §3.1, §5.16–5.18. Summary of the porting story (must appear in README):

- **Cube-generated init** (clocks, TIM1 base+PWM channels+dead time+break, ADC injected config, SPI, GPIO, NVIC) is owned by the application project, not the library. The library consumes handles **only through `constants.h`**.
- **`constants.h`** = macro bridge (§5.17). **`board_support.c`** = only file with HAL calls. **`motor_hw_if.h`** = the portable contract; its mock makes the whole core host-testable.
- **DRV8323 split:** register logic portable in `drv8323.c` (Layer B); wires/SPI in `drv8323_transport.c` (Layer C). CSA gain participates in current scaling via `drv8323_amps_per_lsb` feeding `hw_cal_t`.
- Board change checklist (goes in README): edit `constants.h`; adjust `board_support.c` if peripheral topology differs; adjust `drv8323_transport.c` if SPI/GPIO wiring differs; core untouched.
- CI enforcement: grep rules (§11.7) fail the build if any `src/core` or `include/foc` file includes `constants.h`, `stm32*`, `main.h`, `FreeRTOS`, or `cmsis_os2`.

---

## 11. Test strategy

Tests are deliverables of equal rank with the modules. A module is "complete" only when its tests pass in CI.

### 11.1 Unit tests (host, `test/unit`)

| Test file | Covers | Key assertions |
|---|---|---|
| `test_motor_math.c` | q16 mul/div/clamp/sqrt, macros | saturation at all INT32 corners; 1e6 random ops vs double, tol §11.6; div-by-zero policy |
| `test_trig.c` | sin/cos/atan2 | exhaustive 2^16 angle grid: sin/cos ≤4 LSB q15; atan2 ≤0.05° on 1e5 random vectors incl. axes/quadrant edges |
| `test_motor_filter.c` | LPF/HPF | τ within ±5%; DC gain; zero-input limit-cycle free over 1e6 steps |
| `test_motor_pi.c` | PI + anti-windup | tracking vs float ref; windup recovery bound; bumpless reset exact |
| `test_transforms.c` | Clarke/Park/iPark | round-trip ≤2 LSB; amplitude invariance; 2- vs 3-current variants agree on balanced input |
| `test_limits_sat.c` | circle limit, min-pulse clamp | vd priority; angle preservation; boundary exactness |
| `test_modeler_match.c` | profile distance/resolve | match/ambiguity/threshold; gain recompute vs float ref |
| `test_drv8323_regs.c` | pack/unpack, init seq, fault decode, amps/LSB | round-trip all fields; scripted-transport order + readback-fail path; datasheet fault vectors |

### 11.2 Module tests (host, `test/module`, uses `sim_motor_model` + `replay`)

| Test | Scenario | Pass criteria |
|---|---|---|
| `test_observer_convergence.c` | synthetic traces at 10/25/50/100% speed, ±20% Rs/Ls param error cases, noise per §11.4 | angle RMS ≤3° e (≥25%), ≤8° (10%); convergence ≤100 ms from seed; 60 s drift-free; conf drops ≤50 ms on EMF loss |
| `test_ident_synthetic.c` | known-parameter vectors + noise + quantization; disturbance-injected vectors | Rs ±5%, Ls ±10%, λm ±5%; quality gates reject disturbed vectors |
| `test_sm_transitions.c` | table-driven full matrix from §8 | every legal transition, every timeout, fault-in-every-state, retry budget/cooldown, restart-counter reset |
| `test_faults.c` | §5.7 matrix | debounce, severity, escalation |
| `test_startup_transition.c` | closed SIL loop: sim model + full `foc_ctrl` fast/slow steps via mock hw_if | reaches CLOSED_LOOP_RUN; blend angle step ≤ 5° e at handoff; speed dip ≤ 15% during transition; failed-conf vector → STOPPING + retry |
| `test_tune_sequences.c` | all three TUNE sub-modes: torque steps open-harness, speed/observer sub-modes in the closed SIL loop | step edge timing ±1 fast tick; amplitudes exact (1.0/0.5/0 A defaults); rep counts; speed square wave bypasses ramp (reference settles in 1 speed tick); capture lossless across wrap, bit-identical on replay; gain update applies at task boundary only; abort zeroes refs within 1 tick |

### 11.3 Integration tests

Host (mock-driven): command→duty pipeline (CMD_RUN_SPEED produces monotonic duty evolution consistent with ramp); ident→resolve→apply_params→PI-gain-change verified in-flight blocked / committed at boundary; ISR/task handshake ordering (mock scheduler ticks fast loop 20×, expects exactly one current-task kick).
Target: task jitter (GPIO toggles, scope), queue overflow returns `FOC_EBUSY`, seq-lock reader starvation absent under load.

### 11.4 SIL harness (`test/sim`)

- `sim_motor_model.c`: float, continuous SM-PMSM (Rs, Ls, λm, J, B, pole pairs), RK4 at 1 MHz internal, sampled at 20 kHz; injects: ADC quantization (12-bit over configured full scale), gaussian current noise (σ configurable, default 0.5% FS), offset error, dead-time voltage distortion (first-order).
- `replay.c`: deterministic driver — feeds vectors or live sim outputs into module APIs cycle-by-cycle; fixed RNG seed; produces CSV logs for regression diffing.
- `hw_if_mock.c`: records `hw_*` calls; scriptable ADC values; provides deterministic `hw_cycles_now`.
- `tools/vector_gen.py` regenerates `test/vectors/*` from the model; vectors are committed so CI doesn't require Python.

### 11.5 Hardware bring-up tests (`test/hil`, run in order — each gates the next; 8 steps)

1. **DRV SPI:** init + read-back all regs; toggle a benign field, verify; induced fault (pull nFAULT via OCP threshold set absurdly low with no load) → fault decode correct; restore.
2. **PWM sequence:** outputs-disabled default proven (probe gates); enable → 50% symmetric with dead time visible; `hw_pwm_outputs_disable` from a test EXTI kills gates ≤ 2 µs (scope).
3. **ADC offset cal:** run CALIBRATE; offsets within datasheet CSA offset band; zero-vector residual < 2% rated; repeatability over 10 runs σ < 0.2% FS.
4. **Locked-rotor current loop:** ALIGN state only, id step 25%→50% rated: no oscillation, overshoot < 20%, settles < 5 τ — validates gains and scaling end-to-end.
5. **No-load startup:** full sequence to CLOSED_LOOP_RUN; log telemetry; transition speed dip ≤ 15%; WCET of fast loop recorded < budget.
6. **Fault/recovery:** trigger SW OC (lower trip temporarily) mid-run → PWM kill, FAULT, auto-retry succeeds; exhaust budget → FATAL, CMD_CLEAR path works.
7. **TUNE mode:** run all three sub-modes on the reference motor: torque steps visible in captured iq with expected settle; speed square wave shows step reference (no ramp) and captured response; observer capture shows time-aligned i_a vs estimated BEMF with plausible phase relation at test speed; live gain change measurably alters the next captured step.
8. **Ident on real motor:** compare Rs to DMM/LCR meter (±10% given inverter-drop model), Ls to LCR (±20%), λm to spin-down/back-EMF scope measurement (±10%).

### 11.6 Numeric verification tolerances (fixed-point vs float reference)

| Module | Reference | Tolerance |
|---|---|---|
| q16 arithmetic | double | ±1 LSB Q16.16 per op |
| trig | double sin/cos | ≤4 LSB q15; atan2 ≤0.05° |
| filters | double IIR | ≤0.1% FS after 1000 steps |
| PI | double PI | ≤0.5% FS trajectory RMS |
| transforms | double | ≤2 LSB per output |
| observer | `ref_observer.c` double | θ̂ diff ≤0.5° e RMS on identical input vectors |
| SVPWM duty | double | ±1 tick |

### 11.7 CI structure & static rules

Jobs: (1) host build `-Wall -Wextra -Werror`, run unit+module+SIL; (2) target cross-compile `libfoc.a` (arm-none-eabi, no HAL sources needed — compile core only, plus port with a stub `constants.h` in `examples/`); (3) static rules: grep-fail on `float|double` in `src|include`; grep-fail on forbidden includes in core (`constants.h`, `stm32`, `main.h`, `cmsis_os2`, `FreeRTOS`); `#include` graph check that only whitelisted Layer C/D files include `constants.h`.

---

## 12. Implementation phases (execution order for the coding model)

Each phase = implement listed files + their tests; phase exits only when its tests pass. Host-first, hardware last.

| Phase | Deliverables | Depends on | Host/target | Exit criteria |
|---|---|---|---|---|
| **0. Skeleton** | repo tree, CMake host+target, CI config, Unity (or assert harness), empty headers with include guards, static-rule scripts | — | host | both builds compile empty lib; CI green |
| **1. Foundations** | `motor_types.h`, `motor_math.*` (+ `gen_trig_lut.py` + committed LUT), `test_motor_math.c`, `test_trig.c` | 0 | host | §11.1 rows 1–2 pass |
| **2. Blocks** | `motor_filter.*`, `motor_pi.*`, `motor_params.h`, `motor_limits.h`, tests | 1 | host | filter/PI/limits tests pass |
| **3. FOC core** | `motor_foc.*`, `test_transforms.c`, SVPWM tests, `ref_math.c` | 2 | host | transforms/SVPWM tolerances met |
| **4. Sim harness** | `sim_motor_model`, `replay`, `hw_if_mock`, `vector_gen.py`, initial vectors, `ref_observer.c` | 3 | host | model energy-sane (documented checks); replay determinism (bit-identical reruns) |
| **5. Observer** | `motor_observer.*`, `test_observer_convergence.c` | 4 | host | §11.2 row 1 |
| **6. Faults + SM** | `motor_faults.*`, `motor_sm.*`, their tests | 2 | host | §11.2 rows 3–4 |
| **7. Ident + modeler** | `motor_ident.*`, `motor_modeler.*`, tests, default profile table | 5,6 | host | §11.2 row 2, §11.1 row 7 |
| **8. Orchestrator** | `motor_ctrl.*`, `motor_hw_if.h`, `motor_tune.*`, host integration tests, `test_startup_transition.c`, `test_tune_sequences.c` | 5–7 | host | full SIL startup passes; command/param-flow tests pass; TUNE sequences pass |
| **9. DRV8323 logic** | `drv8323.h/.c`, `test_drv8323_regs.c` | 1 | host | §11.1 row 8 (can run parallel to 5–8) |
| **10. Port layer** | `constants.h` (template + example), `board_support.*`, `motor_hw_if.c`, `drv8323_transport.c`, `isr_motor.c`, `freertos_tasks.c`, example project wiring doc | 8,9 | target | target lib compiles; static rules pass |
| **11. HIL bring-up** | `hil_tests.c`, `hil_checklist.md`, execute §11.5 order 1→7 | 10 | hardware | all HIL gates pass; WCET < budget recorded |
| **12. Hardening** | fault-injection sweep, 1 h soak run, docs (README porting guide, tuning guide), version 1.0 tag | 11 | both | §13 fully met |

Risk register (mitigation is part of the phase that owns it): (a) low-side sampling window vs min-pulse at high modulation → mitigated by SVPWM clamp + 2-of-3 phase reconstruction (Phase 3, HIL 5); (b) HPF phase error in observer at low speed → compensation LUT + shadow-mode convergence gate (Phase 5); (c) fast-loop WCET overrun → `q16_mul_ns` in proven hot spots, WCET counter + watchdog fault (Phases 8, 11); (d) dead-time distortion corrupting Rs ident → inverter-drop correction + two-point method (Phase 7); (e) RTOS priority/ISR misconfig → single documented NVIC map in `constants.h` + init self-check (Phase 10).

---

## 13. Acceptance criteria

The library is done when all of the following hold:

1. **Builds:** host build and target build both compile with `-Wall -Wextra -Werror`; no `float`/`double` and no forbidden includes in `src/core`+`include/foc` (CI static rules green).
2. **Tests:** 100% of unit, module, SIL, and host-integration tests pass in CI; committed vectors reproduce bit-identical outputs across two consecutive CI runs.
3. **Numerics:** every tolerance row in §11.6 satisfied.
4. **HIL:** all seven §11.5 gates pass on the reference board; recorded fast-loop WCET ≤ 25 µs (report actual).
5. **Function:** no-load motor reaches commanded speed via full IDLE→…→CLOSED_LOOP_RUN sequence with transition speed dip ≤ 15%; runs 1 h soak without fault; STOP returns to IDLE cleanly; auto-retry and FATAL latch behave per §8.
6. **Ident:** on the reference motor, Rs ±10% of LCR, Ls ±20%, λm ±10%; modeler resolve produces runnable gains from measured values.
7. **Tuning mode:** all three TUNE sub-modes run on the reference board per HIL step 7; captured data drains losslessly; live gain updates take effect within one task period; CMD_STOP mid-sequence returns to IDLE cleanly.
8. **Portability proof:** changing `constants.h` timer/ADC/SPI macro names (mock second board header) requires zero edits outside Layer C/D — demonstrated by a CI compile of the port against an alternate `constants.h`.
9. **Docs:** README covers integration steps, porting checklist, tuning guide (including TUNE-state workflow), init sequence, and the HAL-isolation rules.

---

## 14. Handoff instructions for the coding model

Follow these exactly:

1. **Implement in Phase order (§12).** Do not start a file whose dependencies' tests are not yet green. Within a phase: header first (exact APIs from §5), then tests, then implementation until tests pass.
2. **Do not invent APIs or rename anything.** Function names, struct fields, file names, state names, and fault names in this plan are normative. If something is genuinely underspecified, choose the simplest option consistent with §3.1 layering, and record the decision in `DECISIONS.md` (one line per decision).
3. **HAL confinement is absolute.** If you find yourself typing `HAL_`, `htim`, `hadc`, `hspi`, or including a `stm32*` header anywhere outside `src/port/stm32u5/`, stop — that code belongs in `board_support.c` behind `motor_hw_if.h`, with names mapped in `constants.h`.
4. **No floats at runtime.** All control math via `motor_math` helpers. If a formula seems to need float, precompute at init with 64-bit integer math or add a helper to `motor_math` with tests.
5. **No dynamic allocation, no recursion, no unbounded loops** in Layers A–C. Every ISR-tagged function must be reviewed against: no RTOS calls, no division except where §7.3 allows, bounded branches.
6. **Every module ships with its tests in the same phase.** A pull request/commit that adds `src/core/x.c` without `test/**/test_x*.c` passing is incomplete.
7. **Comment discipline:** every struct field carries unit + Q-format; every `q16_mul_ns` call site carries a range proof comment; every state-machine guard cites its §8 row.
8. **Keep the fast loop lean:** target ≤ 15 µs at 160 MHz. Profile with `hw_cycles_now` from day one of Phase 8; if over budget, first remove divisions, then consider decimating observer confidence math (not the observer itself).
9. **Determinism in tests:** fixed RNG seeds, committed vectors, no wall-clock dependence. If a test is flaky, the test is wrong — fix it before proceeding.
10. **When wiring the example project (Phase 10):** Cube generates peripheral init only; all §6.8 initialization is created by you in `freertos_tasks.c`/library init — never edit inside Cube USER CODE markers except to call `foc_app_init()` and route the three ISR bodies to `isr_motor.c` handlers.
11. **Ask-by-assuming:** you will not have a human in the loop. For ambiguities, prefer: simpler algorithm, tighter safety, portable code, and add a `TODO(vN)` marker plus a `DECISIONS.md` entry rather than blocking.

**End of plan.**
