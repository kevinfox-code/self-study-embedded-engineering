# FOC Library — Decisions Log

Each entry records an ambiguity resolved during implementation.

## D-001: Test harness — simple assert instead of Unity
Unity is not present in the project. A lightweight assert-based harness
(`test/test_harness.h`) is used for all host unit and module tests. It mirrors
the essential Unity macros (`TEST_ASSERT_EQUAL_INT`, `TEST_ASSERT_TRUE`, etc.)
on top of `assert()` and `printf`, keeping the same usage pattern.

## D-002: `q16_mul_ns` saturation hook
The spec calls for an optional saturation-event counter hook. The hook is a
macro `FOC_SAT_HOOK(ctx, tag)` defined as empty in release (`NDEBUG`) and as an
increment into a test-global counter in test builds. Every `q16_mul` and `q16_div`
call the hook on saturation via this macro.

## D-003: Ls stored in mH, flux in mWb — conversion locations
Per §7.2, Ls is stored in mH and λm in mWb. All conversions (mH → H factor
1000, mWb → Wb) are performed once at module-init time (observer init, ident
init). Every conversion site carries a comment "/* Ls: mH→H ÷1000 */".

## D-004: `foc_atan2` implementation
Implemented as 8-octant LUT + linear interpolation on angle_t. Exhaustive test
against double reference confirms ≤ 0.05° max error over 1e5 random vectors.

## D-005: `q16_mul_q15` helper location
Placed in `motor_math.h/.c` alongside other mixed-format helpers as the plan
(§7.3 rule 3) requires a dedicated helper and the natural home is the math kernel.

## D-006: `hw_cal_t` struct defined in `motor_hw_if.h`
The plan (§5.8) says `hw_cal_t` is "defined here" in `motor_hw_if.h`. Confirmed.

## D-007: `foc_meas_t` struct (used by `motor_tune` fast step)
Not explicitly defined in the plan. Defined in `motor_types.h` as
`{ foc_abc_t i_abc; foc_dq_t i_dq; q16_t vbus; angle_t theta; q16_t omega; q16_t conf; }`
— minimal measurement snapshot passed to TUNE fast step.

## D-008: `foc_tune_refs_t` struct (output of `foc_tune_fast_step`)
Defined in `motor_tune.h` as `{ q16_t id_ref; q16_t iq_ref; q16_t omega_ref; bool override_refs; }`.

## D-009: `foc_ident_quality_t` struct
Defined in `motor_ident.h` as `{ q16_t rs_quality; q16_t ls_quality; q16_t lm_quality; bool rs_pass; bool ls_pass; bool lm_pass; bool overall_pass; }`.

## D-010: `foc_param_source_t` enum defined in `motor_modeler.h`
The plan references it without a dedicated type definition location. Placed in
`motor_modeler.h` next to the API that uses it.

## D-011: `foc_ident_result` struct name
The plan references `foc_ident_result` (without `_t`). Treated as a typedef alias:
`typedef foc_ident_result_t foc_ident_result;` in `motor_ident.h` for compatibility.

## D-012: `foc_bool_t` defined as `bool` wrapper
`foc_bool_t` typedef is `uint8_t` (not `bool`) to avoid stdbool dependency at
struct-layout level, with `FOC_TRUE/FOC_FALSE` companion macros. `stdbool.h` is
still included for direct `bool` use in function parameters.

## D-013: `ref_observer.c` uses double reference observer
The reference observer implements the same flux-linkage algorithm in double
precision, used only in `test_observer_convergence.c` for numeric comparison.

## D-014: SIL vector files seeded with placeholder CSV
`test/vectors/` CSV files committed with minimal header rows; `vector_gen.py`
regenerates them. CI does not call Python — it uses the committed files.

## D-015: `sim_motor_model` RK4 internal step
Internal RK4 runs at the sim_dt passed to `sim_motor_model_init`; default 5 µs
(200 kHz internal / 20 kHz output = 10 sub-steps). Documented in `sim_motor_model.h`.

## D-016: `motor_hw_if.h` `hw_adc_raw_t` struct definition
Defined in `motor_hw_if.h` as `{ uint16_t ia, ib, ic, vbus, temp; }`.

## D-017: DRV8323 register IDs
`drv8323_reg_id_t` enum: REG_FAULT_STATUS1=0, REG_VGS_STATUS2=1, REG_DRIVER_CTRL=2,
REG_GATE_HS=3, REG_GATE_LS=4, REG_OCP_CTRL=5, REG_CSA_CTRL=6.

## D-018: `foc_sm_in_t` and `foc_sm_actions_t` struct definitions
Defined in `motor_sm.h`. `foc_sm_in_t` carries: cmd, faults, obs_conf,
obs_omega, ident_status, ms_now. `foc_sm_actions_t` is a uint32_t bitmask with
named bit-position macros matching the plan §5.14 ACT_* names.

## D-019: `foc_telemetry_t` seq-lock implemented as two uint32_t counters
`seq` counter incremented before and after write. Readers retry when `seq` is
odd or changed between two reads. No RTOS primitives used.

## D-020: `FOC_TUNE_CAP_DEPTH` default 2048
Compile-time constant defined in `motor_tune.h`. Buffer holds 2048 × 4 × int32_t.

## D-021: All persisted/exported speed quantities store MECHANICAL rad/s
Per §7.2 (authority): Q16.16 caps at ±32767, and electrical rad/s (= mechanical
× pole_pairs) overflows above ~45 krpm at 7pp / ~22 krpm at 14pp, breaking
support for >50 krpm motors. All `*_radps_e` fields renamed to `*_radps_m` and
now store mechanical rad/s (`motor_params.h`, `motor_observer.h`, `motor_ident.h`,
`motor_tune.h`, `motor_ctrl.h`). Electrical angle/speed is used only
transiently via 64-bit intermediates at exactly two points: the forced-angle/
PLL angle-increment computation (`Δθ_e = ω_m × pole_pairs × Ts`) and the
back-EMF magnitude computation (`foc_obs_get_emf_ab`, ident λm cross-check).
The observer's PLL persists `omega_m` (mechanical) as its authoritative speed
state; `sim_motor_model`'s plant state was also corrected to be mechanical
(theta_m/omega_m) with electrical angle/speed derived for back-EMF, matching
physical torque/inertia dynamics (J·dω_m/dt = Te).

## D-022: `hw_cal_t` extended with per-channel gain/offset/polarity + vbus/temp
Per §5.8, `hw_cal_t` (`motor_hw_if.h`) replaced the flat
`amps_per_lsb/vbus_per_lsb/offset_i{a,b,c}_raw` fields with `i_gain[3]`,
`i_offset_raw[3]` (kept `int32_t`, not the plan's `int16_t`, to match the
existing codebase's offset width convention), `i_polarity[3]` (`int8_t`,
values constrained to +1/-1 by `hw_cal_set`), plus `vbus_gain`,
`vbus_offset_v`, `temp_gain`, `temp_offset_c`. `hw_vbus_scale()` gained a
`const hw_cal_t*` parameter (previously hardcoded per-implementation) so
`vbus_gain`/`vbus_offset_v` are actually used — the only caller
(`motor_ctrl.c`'s fast loop) and both implementations (target
`src/port/stm32u5/motor_hw_if.c`, host mock `test/sim/hw_if_mock.c`) were
updated. `hw_current_scale3()` in both implementations was also fixed to
apply `i_polarity` and to use `q16_from_raw_scaled` (integer count ×
Q16.16 gain, no bit-shift) instead of the previous `(raw*gain)>>16` pattern,
which was a latent scaling bug (a >>16 after an integer×Q16 multiply divides
the result by 65536 too many times); this was caught by the new
`test_hw_current_scale3_known_vector` unit test.

## D-023: `hw_cal_set`/`hw_cal_get` live in new `src/core/motor_hw_if.c` (Layer B)
These two functions are pure validation/copy logic with no HAL dependency, so
they were placed in a new host-buildable `src/core/motor_hw_if.c` (added to
`FOC_CORE_SOURCES` in `CMakeLists.txt`, built into `libfoc.a` for both host
and target). This keeps them host-testable per §5.8 without inventing any
new singleton/RTOS-owned config state — both functions take/return an
explicit caller-owned `hw_cal_t*`, matching the existing
`foc_ctrl_apply_params` pattern (caller owns storage, e.g. `foc_ctrl_t::cal`,
passed by pointer). The target-specific PWM/ADC/GPIO primitives remain in
`src/port/stm32u5/motor_hw_if.c`; nothing in the new file includes
board-config headers (enforced by the existing
`static_check_no_constants_in_core` CI rule).

## D-024: `user_motor_01` profile added to `motor_modeler.c`
Rs=0.149 Ω, Ls=96 mH, λm=156.34 mWb (from Ke=0.982 V_peak/Hz(elec) / 2π),
pole_pairs=1, rated_current_a=7.0 A. `max_speed_radps_m = Q16(60.0)`:
derived by capping peak back-EMF at 80% of the 12 V bus (9.6 V) ⇒
f_max = 9.6/0.982 ≈ 9.77 Hz ⇒ ω_max ≈ 2π·9.77 ≈ 61.4 rad/s_e = rad/s_m
(pp=1), rounded down to a conservative 60.0. Current-loop bandwidth
`f_bw_current` reduced from the library default of 1000 Hz to 60 Hz because
Ls=96 mH is unusually high inductance (kp = 2π·f_bw·Ls scales directly with
Ls); at 60 Hz, kp≈36.19, ki≈56.17 — chosen as a compromise: lower f_bw
values (e.g. ~0.6 Hz) would keep kp·rated_current within the same
percentage-of-bus-voltage margin used by the other three profiles, but that
bandwidth is impractically slow for a current loop, so 60 Hz was kept as a
documented trade-off (kp·7A ≈ 253 V is a full-scale-error artifact of the PI
structure, not a steady-state operating voltage; actual vd/vq are bounded by
`foc_vdq_limit`'s circle limit). Speed PI, PLL, and LPF/HPF gains were
scaled from the existing three profiles' ratios (gain vs. rated current /
max electrical speed / electrical fundamental frequency respectively).
Open-loop startup ramp (`ol_target_radps_m = Q16(12.0)`,
`min_run_speed_radps_m = Q16(7.2)` = 0.6× target, matching the ratio used
by all three existing profiles) was kept well below the 60 rad/s_m ceiling
since this is a slow, high-inductance machine. `motor_limits_t` for this
entry uses a literal struct initializer (not `FOC_LIMITS_DEFAULT`) since the
macro is not per-field parameterizable, overriding `i_phase_max_a`=7.0 A,
`i_phase_trip_a`=8.0 A, and a 12 V-class `vbus_min/max` of 9/15 V.
