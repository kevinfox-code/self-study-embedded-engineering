/**
 * @file sim_motor_model.h
 * @brief Float-precision PMSM simulation model.  HOST ONLY.
 *
 * Implements a continuous SM-PMSM model in αβ frame using RK4 integration.
 * (Decision D-015: 10 sub-steps at 200 kHz internal, output sampled at 20 kHz).
 *
 * Mechanical-rad/s decision (§7.2): the plant's natural physical state is
 * MECHANICAL speed/angle (torque = J·dω_m/dt; electrical angle = mechanical
 * angle × pole_pairs). State: [i_alpha, i_beta, theta_m, omega_m]; electrical
 * angle/speed (theta_e, omega_e) are derived (θ_e = pp·θ_m, ω_e = pp·ω_m) and
 * exposed for back-EMF computation and to callers that need electrical
 * quantities (e.g. commutation-angle-based test harnesses).
 * Input: [v_alpha, v_beta, load_torque_nm]
 *
 * Model equations (αβ frame, rotor reference via Park; θ_e = pp·θ_m):
 *   di_alpha/dt = (v_alpha - Rs·i_alpha - λm·ωe·(-sin θe)) / Ls
 *   di_beta/dt  = (v_beta  - Rs·i_beta  - λm·ωe·( cos θe)) / Ls
 *   dω_m/dt     = (1/J)·(p·λm·(i_beta·cos θe - i_alpha·sin θe) - B·ω_m - Tl)
 *   dθ_m/dt     = ω_m
 */
#ifndef FOC_SIM_MOTOR_MODEL_H
#define FOC_SIM_MOTOR_MODEL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    /* Plant parameters */
    double rs;          /**< Stator resistance [Ω]       */
    double ls;          /**< Stator inductance [H]       */
    double lambda_m;    /**< Flux linkage [Wb]           */
    int    pole_pairs;  /**< Pole pairs                  */
    double inertia_j;   /**< Rotor inertia [kg·m²]       */
    double friction_b;  /**< Viscous friction [N·m·s/rad] */

    /* Noise / distortion model */
    double noise_sigma_a;  /**< Gaussian current noise σ [A] */
    uint32_t rng_seed;     /**< RNG seed (fixed for determinism) */

    /* Internal RK4 sub-step */
    double sim_dt;     /**< Sub-step period [s] (Decision D-015: 5 µs default) */
    double sample_dt;  /**< Output sample period [s] (default 50 µs = 20 kHz) */

    /* State (internal) — MECHANICAL, the plant's natural physical state */
    double i_alpha;
    double i_beta;
    double theta_m;   /**< MECHANICAL angle [rad], wraps modulo 2π */
    double omega_m;   /**< MECHANICAL speed [rad/s]                */

    /* ADC quantisation model */
    double adc_fullscale_a;  /**< ADC FS current [A] (for 12-bit quantisation) */
    double adc_vbus_fs;      /**< ADC FS vbus    [V]                            */
    double dead_time_s;      /**< Dead-time [s] for first-order voltage distortion */
    double current_dir_a[3]; /**< Sign estimate for dead-time correction          */

    /* Internal RNG state */
    uint32_t rng_state;
} sim_motor_t;

typedef struct {
    double i_alpha;    /**< Phase-α current [A]    */
    double i_beta;     /**< Phase-β current [A]    */
    double i_a, i_b, i_c; /**< Phase currents [A]   */
    double theta_e;    /**< ELECTRICAL angle [rad] = pole_pairs × theta_m */
    double omega_e;    /**< ELECTRICAL speed [rad/s] = pole_pairs × omega_m */
    double theta_m;    /**< MECHANICAL angle [rad] (§7.2 — matches
                        *   foc_obs_get_theta()'s electrical convention is
                        *   NOT this; theta_m is provided for harnesses
                        *   comparing against mechanical references)      */
    double omega_m;    /**< MECHANICAL speed [rad/s] — matches the
                        *   fixed-point observer's exported ω̂ (§7.2)       */
    uint16_t raw_ia, raw_ib, raw_ic, raw_vbus; /**< Quantised ADC counts */
} sim_motor_output_t;

/**
 * Initialise the motor model.
 * @param m     Model state to initialise.
 * @param rs    Stator resistance [Ω].
 * @param ls    Stator inductance [H].
 * @param lm    Flux linkage [Wb].
 * @param pp    Pole pairs.
 * @param J     Inertia [kg·m²].
 * @param B     Friction [N·m·s/rad].
 * @param seed  RNG seed.
 */
void sim_motor_model_init(sim_motor_t *m,
                          double rs, double ls, double lm,
                          int pp, double J, double B,
                          uint32_t seed);

/**
 * Advance one output sample (20 kHz period).
 * Runs sub_steps RK4 steps internally.
 * @param m     Model state.
 * @param v_alpha Applied voltage α [V].
 * @param v_beta  Applied voltage β [V].
 * @param tl      Load torque [N·m].
 * @param out     Quantised output.
 */
void sim_motor_model_step(sim_motor_t *m,
                          double v_alpha, double v_beta, double tl,
                          sim_motor_output_t *out);

/**
 * Reset rotor to known state (for convergence tests).
 * @param theta_m  MECHANICAL angle [rad] (§7.2).
 * @param omega_m  MECHANICAL speed [rad/s].
 */
void sim_motor_model_set_state(sim_motor_t *m,
                               double theta_m, double omega_m,
                               double i_alpha, double i_beta);

#ifdef __cplusplus
}
#endif

#endif /* FOC_SIM_MOTOR_MODEL_H */
