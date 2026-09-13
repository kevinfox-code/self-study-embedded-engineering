/**
 * @file sim_motor_model.c
 * @brief Float-precision PMSM simulation.  HOST ONLY.
 */
#include "sim_motor_model.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SIM_DEFAULT_DT      (5e-6)   /* 5 µs sub-step (200 kHz) */
#define SIM_SAMPLE_DT       (50e-6)  /* 20 kHz output */
#define ADC_BITS            12
#define ADC_COUNTS          ((double)(1 << ADC_BITS))

/* Simple LCG RNG. */
static double rng_gaussian(uint32_t *state)
{
    /* Box-Muller using two LCG values */
    *state = (*state) * 1664525u + 1013904223u;
    double u1 = ((double)(*state) + 1.0) / 4294967297.0;
    *state = (*state) * 1664525u + 1013904223u;
    double u2 = ((double)(*state) + 1.0) / 4294967297.0;
    return sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
}

/* PMSM derivatives in αβ frame.
 * theta/omega here are MECHANICAL (§7.2) — the plant's natural physical
 * state (torque = J·dω_m/dt). Electrical angle (θ_e = pp·θ_m) is derived
 * inside deriv() only where needed for back-EMF / torque-angle trig. */
typedef struct { double i_a, i_b, theta, omega; } state4_t;

static state4_t deriv(const sim_motor_t *m, const state4_t *s,
                      double v_alpha, double v_beta, double tl)
{
    state4_t d;
    /* Electrical angle/speed derived transiently from mechanical state. */
    double theta_e = (double)m->pole_pairs * s->theta;
    double omega_e = (double)m->pole_pairs * s->omega;
    double sin_t = sin(theta_e);
    double cos_t = cos(theta_e);
    /* Back-EMF components (proportional to ELECTRICAL speed) */
    double e_alpha = -m->lambda_m * omega_e * sin_t;
    double e_beta  =  m->lambda_m * omega_e * cos_t;
    d.i_a    = (v_alpha - m->rs * s->i_a - e_alpha) / m->ls;
    d.i_b    = (v_beta  - m->rs * s->i_b - e_beta)  / m->ls;
    /* Electromagnetic torque; mechanical speed dynamics: J·dω_m/dt = Te - B·ω_m - Tl */
    double te = (double)m->pole_pairs * m->lambda_m *
                (s->i_b * cos_t - s->i_a * sin_t);
    d.omega  = (te - m->friction_b * s->omega - tl) / m->inertia_j;
    d.theta  = s->omega;
    return d;
}

static state4_t rk4_step(const sim_motor_t *m, const state4_t *s,
                          double v_alpha, double v_beta, double tl,
                          double dt)
{
    state4_t k1 = deriv(m, s, v_alpha, v_beta, tl);
    state4_t s2 = {s->i_a  + 0.5*dt*k1.i_a,
                   s->i_b  + 0.5*dt*k1.i_b,
                   s->theta+ 0.5*dt*k1.theta,
                   s->omega+ 0.5*dt*k1.omega};
    state4_t k2 = deriv(m, &s2, v_alpha, v_beta, tl);
    state4_t s3 = {s->i_a  + 0.5*dt*k2.i_a,
                   s->i_b  + 0.5*dt*k2.i_b,
                   s->theta+ 0.5*dt*k2.theta,
                   s->omega+ 0.5*dt*k2.omega};
    state4_t k3 = deriv(m, &s3, v_alpha, v_beta, tl);
    state4_t s4 = {s->i_a  + dt*k3.i_a,
                   s->i_b  + dt*k3.i_b,
                   s->theta+ dt*k3.theta,
                   s->omega+ dt*k3.omega};
    state4_t k4 = deriv(m, &s4, v_alpha, v_beta, tl);
    state4_t sn;
    sn.i_a   = s->i_a   + dt/6.0*(k1.i_a  + 2*k2.i_a  + 2*k3.i_a  + k4.i_a);
    sn.i_b   = s->i_b   + dt/6.0*(k1.i_b  + 2*k2.i_b  + 2*k3.i_b  + k4.i_b);
    sn.theta = s->theta + dt/6.0*(k1.theta + 2*k2.theta + 2*k3.theta + k4.theta);
    sn.omega = s->omega + dt/6.0*(k1.omega + 2*k2.omega + 2*k3.omega + k4.omega);
    /* Wrap theta (mechanical angle) */
    while (sn.theta >  M_PI) sn.theta -= 2.0 * M_PI;
    while (sn.theta < -M_PI) sn.theta += 2.0 * M_PI;
    return sn;
}

void sim_motor_model_init(sim_motor_t *m,
                          double rs, double ls, double lm,
                          int pp, double J, double B,
                          uint32_t seed)
{
    memset(m, 0, sizeof(*m));
    m->rs = rs; m->ls = ls; m->lambda_m = lm;
    m->pole_pairs = pp; m->inertia_j = J; m->friction_b = B;
    m->sim_dt = SIM_DEFAULT_DT;
    m->sample_dt = SIM_SAMPLE_DT;
    m->rng_seed = seed;
    m->rng_state = seed;
    m->noise_sigma_a = 0.005; /* 0.5% FS noise default */
    m->adc_fullscale_a = 5.0; /* 5 A FS */
    m->adc_vbus_fs = 60.0;    /* 60 V FS */
    m->dead_time_s = 200e-9;  /* 200 ns dead time */
}

void sim_motor_model_step(sim_motor_t *m,
                          double v_alpha, double v_beta, double tl,
                          sim_motor_output_t *out)
{
    int sub_steps = (int)(m->sample_dt / m->sim_dt + 0.5);
    if (sub_steps < 1) sub_steps = 1;

    state4_t s = {m->i_alpha, m->i_beta, m->theta_m, m->omega_m};

    double dt = m->sample_dt / (double)sub_steps;
    for (int i = 0; i < sub_steps; i++) {
        s = rk4_step(m, &s, v_alpha, v_beta, tl, dt);
    }

    m->i_alpha = s.i_a;
    m->i_beta  = s.i_b;
    m->theta_m = s.theta;
    m->omega_m = s.omega;

    /* Reconstruct phase currents (amplitude-invariant Clarke inverse).
     * α = ia, β = (ia + 2·ib) / √3  → ib = (β·√3 - ia) / 2
     *                                   ic = -(ia + ib)
     */
    double ia = s.i_a;
    double ib = (s.i_b * sqrt(3.0) - s.i_a) / 2.0;
    double ic = -(ia + ib);

    /* Add noise */
    double noise_a = m->noise_sigma_a * rng_gaussian(&m->rng_state);
    double noise_b = m->noise_sigma_a * rng_gaussian(&m->rng_state);
    double noise_c = m->noise_sigma_a * rng_gaussian(&m->rng_state);

    out->i_alpha = s.i_a;
    out->i_beta  = s.i_b;
    out->i_a     = ia + noise_a;
    out->i_b     = ib + noise_b;
    out->i_c     = ic + noise_c;
    out->theta_m = s.theta;
    out->omega_m = s.omega;
    /* Electrical angle/speed derived for callers needing commutation-angle
     * or back-EMF-magnitude quantities (§7.2). */
    double theta_e = (double)m->pole_pairs * s.theta;
    while (theta_e >  M_PI) theta_e -= 2.0 * M_PI;
    while (theta_e < -M_PI) theta_e += 2.0 * M_PI;
    out->theta_e = theta_e;
    out->omega_e = (double)m->pole_pairs * s.omega;

    /* Quantise to 12-bit ADC counts (biased at mid-scale for signed current). */
    double scale_a = ADC_COUNTS / (2.0 * m->adc_fullscale_a);
    double mid = ADC_COUNTS / 2.0;
    double ia_raw_d = (out->i_a) * scale_a + mid;
    double ib_raw_d = (out->i_b) * scale_a + mid;
    double ic_raw_d = (out->i_c) * scale_a + mid;
    if (ia_raw_d < 0) ia_raw_d = 0; if (ia_raw_d > ADC_COUNTS-1) ia_raw_d = ADC_COUNTS-1;
    if (ib_raw_d < 0) ib_raw_d = 0; if (ib_raw_d > ADC_COUNTS-1) ib_raw_d = ADC_COUNTS-1;
    if (ic_raw_d < 0) ic_raw_d = 0; if (ic_raw_d > ADC_COUNTS-1) ic_raw_d = ADC_COUNTS-1;
    out->raw_ia = (uint16_t)ia_raw_d;
    out->raw_ib = (uint16_t)ib_raw_d;
    out->raw_ic = (uint16_t)ic_raw_d;
    /* Vbus: fixed at 24 V mapped to ADC */
    out->raw_vbus = (uint16_t)(24.0 / m->adc_vbus_fs * ADC_COUNTS);
}

void sim_motor_model_set_state(sim_motor_t *m,
                               double theta_m, double omega_m,
                               double i_alpha, double i_beta)
{
    m->theta_m = theta_m;
    m->omega_m = omega_m;
    m->i_alpha = i_alpha;
    m->i_beta  = i_beta;
}
