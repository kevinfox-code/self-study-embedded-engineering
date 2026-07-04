/**
 * @file ref_math.h
 * @brief Float reference math declarations.  HOST ONLY.
 */
#ifndef FOC_REF_MATH_H
#define FOC_REF_MATH_H

void ref_clarke(double ia, double ib, double ic,
                double *alpha, double *beta);
void ref_park(double alpha, double beta, double theta,
              double *d, double *q);
void ref_ipark(double d, double q, double theta,
               double *alpha, double *beta);
double ref_svpwm_duty(double v_phase_shifted, double vbus, int period);

#endif /* FOC_REF_MATH_H */
