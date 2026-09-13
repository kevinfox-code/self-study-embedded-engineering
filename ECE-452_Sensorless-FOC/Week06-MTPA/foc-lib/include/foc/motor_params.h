/**
 * @file motor_params.h
 * @brief Runtime parameter set structs and defaults.
 *        Pure data — no functions, no HAL, no Cube symbols.
 *
 * Layer: A  Dependencies: motor_types.h, motor_pi.h.
 *
 * Unit comments: every field lists unit + Q format.
 * Default macro: FOC_PARAMS_DEFAULT — conservative values for a generic
 *   24 V gimbal-class PMSM (200 W class).
 */
#ifndef FOC_MOTOR_PARAMS_H
#define FOC_MOTOR_PARAMS_H

#include "motor_types.h"
#include "motor_pi.h"
#include "motor_limits.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Motor plant parameters
 * ---------------------------------------------------------------------- */
typedef struct {
    ohm_q16_t   rs_ohm;            /**< Stator resistance [Ω]   Q16.16  */
    mh_q16_t    ls_mh;             /**< Stator inductance [mH]  Q16.16  */
    mwb_q16_t   lambda_m_mwb;      /**< Flux linkage λm   [mWb] Q16.16  */
    uint8_t     pole_pairs;        /**< Pole pairs (dimensionless)       */
    amps_q16_t  rated_current_a;   /**< Rated phase current [A] Q16.16  */
    radps_q16_t max_speed_radps_m; /**< Max MECHANICAL speed [rad/s_m] Q16.16.
                                     *   Electrical speed is derived transiently
                                     *   (ω_e = ω_m × pole_pairs, 64-bit intermediate)
                                     *   and never stored in q16_t — see §7.2. */
} foc_motor_params_t;

/* -------------------------------------------------------------------------
 * Controller tuning parameters
 * ---------------------------------------------------------------------- */
typedef struct {
    foc_pi_cfg_t pi_id;          /**< d-axis current PI config            */
    foc_pi_cfg_t pi_iq;          /**< q-axis current PI config            */
    foc_pi_cfg_t pi_speed;       /**< Speed PI config                     */
    q16_t        obs_gain_g1;    /**< Observer flux integrator gain  Q16.16 */
    q16_t        obs_gain_g2;    /**< Observer correction gain       Q16.16 */
    q16_t        pll_kp;         /**< PLL proportional gain [rad/s]  Q16.16 */
    q16_t        pll_ki;         /**< PLL integral gain    [rad/s²] Q16.16 */
    q16_t        lpf_speed_hz;   /**< Speed LPF cutoff [Hz] Q16.16         */
    q16_t        lpf_vbus_hz;    /**< Vbus LPF cutoff  [Hz] Q16.16         */
    q16_t        hpf_bemf_hz;    /**< Back-EMF / flux HPF cutoff [Hz] Q16.16 */
} foc_tuning_t;

/* -------------------------------------------------------------------------
 * Startup / sensorless handoff parameters
 * ---------------------------------------------------------------------- */
typedef struct {
    amps_q16_t  align_current_a;      /**< Align d-axis current [A]         Q16.16 */
    uint16_t    align_ms;             /**< Align duration       [ms]               */
    amps_q16_t  ol_current_a;         /**< Open-loop q-axis current [A]     Q16.16 */
    radps_q16_t ol_accel_radps2_m;    /**< OL angular accel   [rad/s²_m]    Q16.16.
                                       *   Forced-angle integrator converts to
                                       *   electrical angle increment per fast
                                       *   loop via Δθ_e = ω_m × pole_pairs × Ts
                                       *   (64-bit intermediate at the conversion
                                       *   point only).                            */
    radps_q16_t ol_target_radps_m;    /**< OL target speed    [rad/s_m]     Q16.16 */
    q16_t       trans_min_conf;       /**< Min observer confidence to transition Q16.16 */
    uint16_t    trans_hold_ms;        /**< Confidence hold time  [ms]              */
    uint16_t    trans_blend_ms;       /**< Blend duration        [ms]              */
    radps_q16_t min_run_speed_radps_m;/**< Min closed-loop speed [rad/s_m]  Q16.16 */
} foc_startup_t;

/* -------------------------------------------------------------------------
 * Aggregated parameter set passed to foc_ctrl_init
 * (motor_limits_t included below via forward inclusion)
 * ---------------------------------------------------------------------- */
typedef struct {
    foc_motor_params_t  motor;
    foc_tuning_t        tuning;
    foc_startup_t       startup;
    motor_limits_t      limits; /**< Protection thresholds — see motor_limits.h */
} foc_params_t;

/* -------------------------------------------------------------------------
 * Default parameter set — generic 24 V gimbal PMSM (~24 V, ~1 A rated)
 * All Q16.16 constants use the Q16() macro (compile-time fp64 literal,
 * no fp32 emitted at runtime — verified by static rules).
 * ---------------------------------------------------------------------- */
/* PWM at 20 kHz → fs = 20000 Hz for current PI. */
#define FOC_CURRENT_LOOP_HZ     Q16(20000.0)
#define FOC_SPEED_LOOP_HZ       Q16(1000.0)

/* Conservative PI defaults for a ~1 Ω / 1 mH motor at 20 kHz.
 * kp = 2π·f_bw·Ls  with f_bw = 500 Hz, Ls = 1e-3 H: kp ≈ 3.14
 * ki = 2π·f_bw·Rs  with f_bw = 500 Hz, Rs = 1.0 Ω:  ki ≈ 3141
 */
#define FOC_PI_ID_DEFAULT \
    { Q16(3.14), Q16(3141.0), FOC_CURRENT_LOOP_HZ, Q16(-24.0), Q16(24.0), Q16(0.1) }

#define FOC_PI_IQ_DEFAULT \
    { Q16(3.14), Q16(3141.0), FOC_CURRENT_LOOP_HZ, Q16(-24.0), Q16(24.0), Q16(0.1) }

#define FOC_PI_SPEED_DEFAULT \
    { Q16(0.05), Q16(5.0), FOC_SPEED_LOOP_HZ, Q16(-1.0), Q16(1.0), Q16(0.2) }

#define FOC_MOTOR_PARAMS_DEFAULT \
    { Q16(1.0),   /* rs_ohm    1 Ω          */ \
      Q16(1.0),   /* ls_mh     1 mH         */ \
      Q16(5.0),   /* lambda_m_mwb  5 mWb    */ \
      7u,         /* pole_pairs             */ \
      Q16(1.0),   /* rated_current_a  1 A   */ \
      Q16(104.7)  /* max_speed_radps_m 1000 rpm_m * 2π/60 ≈ 104.7 rad/s_m */ \
    }

#define FOC_TUNING_DEFAULT \
    { FOC_PI_ID_DEFAULT, FOC_PI_IQ_DEFAULT, FOC_PI_SPEED_DEFAULT, \
      Q16(1.0), Q16(1.0),   /* obs gains g1, g2   */ \
      Q16(500.0), Q16(5000.0), /* pll kp, ki       */ \
      Q16(20.0), Q16(10.0),  /* lpf speed, vbus hz */ \
      Q16(2.0)               /* hpf bemf hz        */ \
    }

/* Mechanical values below: prior electrical-rad/s constants divided by
 * pole_pairs=7 (FOC_MOTOR_PARAMS_DEFAULT), per §7.2 (mechanical rad/s
 * decision — fixes Q16.16 overflow for >50 krpm motors). */
#define FOC_STARTUP_DEFAULT \
    { Q16(0.3),   /* align_current_a   0.3 A       */ \
      200u,       /* align_ms          200 ms      */ \
      Q16(0.5),   /* ol_current_a      0.5 A       */ \
      Q16(14.2857), /* ol_accel_radps2_m 100/7 rad/s²_m */ \
      Q16(15.7143), /* ol_target_radps_m 110/7 rad/s_m  */ \
      Q16(0.5),   /* trans_min_conf    0.5         */ \
      50u,        /* trans_hold_ms     50 ms       */ \
      100u,       /* trans_blend_ms    100 ms      */ \
      Q16(9.4286) /* min_run_speed_radps_m 66/7 rad/s_m */ \
    }

#ifdef __cplusplus
}
#endif

#endif /* FOC_MOTOR_PARAMS_H */
