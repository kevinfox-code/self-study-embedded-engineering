/**
 * @file ref_observer.h
 * @brief Float-precision reference observer for numeric comparison.  HOST ONLY.
 *
 * Implements the same flux-linkage + PLL observer algorithm as the fixed-point
 * motor_observer.c, but in double precision.  Used by test_observer_convergence.c
 * to verify fixed-point accuracy within the §11.6 tolerance (≤ 0.5° RMS).
 *
 * Mechanical-rad/s decision (§7.2): to remain a valid reference for the
 * fixed-point observer's now-mechanical speed output, this reference's
 * speed I/O (ref_obs_reset seed, obs->omega) is MECHANICAL rad/s. Internally
 * the float back-EMF/PLL physics still operate on electrical angle/speed
 * (theta is electrical, and omega is converted ×pole_pairs at the two
 * points where electrical speed is needed: the PLL angle-update term and
 * any back-EMF-magnitude computation), matching motor_observer.c's split.
 */
#ifndef FOC_REF_OBSERVER_H
#define FOC_REF_OBSERVER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    /* Plant parameters */
    double rs;       /**< [Ω]   */
    double ls;       /**< [H]   */
    double lambda_m; /**< [Wb]  */
    int    pole_pairs; /**< Pole pairs — used to convert mechanical I/O
                        *   speed to/from the internal electrical PLL state */

    /* Tuning */
    double hpf_fc;   /**< HPF cutoff [Hz]    */
    double pll_kp;   /**< PLL Kp [rad/s]     */
    double pll_ki;   /**< PLL Ki [rad/s²]    */
    double ts;       /**< Sample period [s]  */

    /* State */
    double psi_a;    /**< Flux linkage α [Wb]  */
    double psi_b;    /**< Flux linkage β [Wb]  */
    double psi_a_hpf;/**< HPF state α          */
    double psi_b_hpf;/**< HPF state β          */
    double x_a_hpf;  /**< HPF input state α    */
    double x_b_hpf;  /**< HPF input state β    */
    double theta;    /**< Estimated ELECTRICAL angle [rad] (internal)   */
    double omega_e;  /**< Estimated ELECTRICAL speed [rad/s] (internal) */
    double omega_m;  /**< Estimated MECHANICAL speed [rad/s] — exported,
                       *   matches fixed-point foc_obs_get_omega() (§7.2) */
    double conf;     /**< Confidence [0,1]        */

    double v_a_prev; /**< Previous applied Vα [V] */
    double v_b_prev; /**< Previous applied Vβ [V] */
    double i_a_prev; /**< Previous iα [A]          */
    double i_b_prev; /**< Previous iβ [A]          */
} ref_obs_t;

void ref_obs_init(ref_obs_t *obs,
                  double rs, double ls, double lambda_m, int pole_pairs,
                  double hpf_fc, double pll_kp, double pll_ki,
                  double ts);

void ref_obs_step(ref_obs_t *obs,
                  double i_alpha, double i_beta,
                  double v_alpha_prev, double v_beta_prev);

/** @param omega0_m  Seed speed, MECHANICAL rad/s (matches fixed-point). */
void ref_obs_reset(ref_obs_t *obs, double theta0, double omega0_m);

#ifdef __cplusplus
}
#endif

#endif /* FOC_REF_OBSERVER_H */
