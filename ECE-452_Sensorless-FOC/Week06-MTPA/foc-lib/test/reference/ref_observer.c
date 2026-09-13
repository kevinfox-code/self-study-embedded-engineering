/**
 * @file ref_observer.c
 * @brief Double-precision reference observer.  HOST ONLY.
 *
 * Algorithm: flux-linkage form with HPF drift mitigation + PLL.
 * See §5.11 and motor_observer.c for the fixed-point counterpart.
 */
#include "ref_observer.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void ref_obs_init(ref_obs_t *obs,
                  double rs, double ls, double lambda_m, int pole_pairs,
                  double hpf_fc, double pll_kp, double pll_ki,
                  double ts)
{
    memset(obs, 0, sizeof(*obs));
    obs->rs         = rs;
    obs->ls         = ls;
    obs->lambda_m   = lambda_m;
    obs->pole_pairs = (pole_pairs > 0) ? pole_pairs : 1;
    obs->hpf_fc     = hpf_fc;
    obs->pll_kp     = pll_kp;
    obs->pll_ki     = pll_ki;
    obs->ts         = ts;
}

void ref_obs_reset(ref_obs_t *obs, double theta0, double omega0_m)
{
    obs->theta   = theta0;
    obs->omega_m = omega0_m;
    obs->omega_e = omega0_m * (double)obs->pole_pairs;
    obs->psi_a = obs->lambda_m * cos(theta0);
    obs->psi_b = obs->lambda_m * sin(theta0);
}

/* First-order HPF: alpha = 1 / (1 + 2π·fc·Ts) */
static double hpf(double *state_x, double *state_y, double x, double alpha)
{
    double y = alpha * (*state_y + x - *state_x);
    *state_x = x;
    *state_y = y;
    return y;
}

void ref_obs_step(ref_obs_t *obs,
                  double i_alpha, double i_beta,
                  double v_alpha_prev, double v_beta_prev)
{
    double ts = obs->ts;

    /* Flux integrator:  ψ[n] = ψ[n-1] + (v_prev − Rs·i)·Ts − Ls·Δi */
    double di_a = i_alpha - obs->i_a_prev;
    double di_b = i_beta  - obs->i_b_prev;

    double dpsi_a = (v_alpha_prev - obs->rs * i_alpha) * ts - obs->ls * di_a;
    double dpsi_b = (v_beta_prev  - obs->rs * i_beta)  * ts - obs->ls * di_b;
    obs->psi_a += dpsi_a;
    obs->psi_b += dpsi_b;

    /* HPF drift removal */
    double alpha_hpf = 1.0 / (1.0 + 2.0 * M_PI * obs->hpf_fc * ts);
    double psi_a_hpf = hpf(&obs->x_a_hpf, &obs->psi_a_hpf, obs->psi_a, alpha_hpf);
    double psi_b_hpf = hpf(&obs->x_b_hpf, &obs->psi_b_hpf, obs->psi_b, alpha_hpf);

    /* Angle from flux */
    double theta_flux = atan2(psi_b_hpf, psi_a_hpf);

    /* PLL: wrap angle error (electrical angle) */
    double err = theta_flux - obs->theta;
    while (err >  M_PI) err -= 2.0 * M_PI;
    while (err < -M_PI) err += 2.0 * M_PI;

    /* ω̂_e += pll_ki·err·Ts  (electrical, matches motor_observer.c algorithm);
     * ω̂_m is the persisted/exported state, obtained by dividing down. */
    obs->omega_e += obs->pll_ki * err * ts;
    obs->omega_m  = obs->omega_e / (double)obs->pole_pairs;
    obs->theta   += (obs->omega_e + obs->pll_kp * err) * ts;
    /* Wrap theta */
    while (obs->theta >  M_PI) obs->theta -= 2.0 * M_PI;
    while (obs->theta < -M_PI) obs->theta += 2.0 * M_PI;

    /* Confidence: flux magnitude vs expected λm */
    double psi_mag = sqrt(psi_a_hpf * psi_a_hpf + psi_b_hpf * psi_b_hpf);
    double lm = obs->lambda_m;
    if (lm < 1e-9) lm = 1e-9;
    double ratio = psi_mag / lm;
    if (ratio > 1.0) ratio = 1.0 / ratio;
    obs->conf = ratio;

    /* Store for next step */
    obs->i_a_prev = i_alpha;
    obs->i_b_prev = i_beta;
}
