/**
 * @file ref_math.c
 * @brief Float reference implementations of trig, transforms, and PI.
 *        HOST ONLY — uses double precision throughout.
 *        Used by test_transforms.c and test_trig.c for tolerance comparison.
 */
#include "ref_math.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void ref_clarke(double ia, double ib, double ic,
                double *alpha, double *beta)
{
    (void)ic; /* amplitude-invariant 3-current: ic not needed if balanced */
    *alpha = ia;
    *beta  = (ia + 2.0 * ib) / sqrt(3.0);
}

void ref_park(double alpha, double beta, double theta,
              double *d, double *q)
{
    *d =  alpha * cos(theta) + beta * sin(theta);
    *q = -alpha * sin(theta) + beta * cos(theta);
}

void ref_ipark(double d, double q, double theta,
               double *alpha, double *beta)
{
    *alpha = d * cos(theta) - q * sin(theta);
    *beta  = d * sin(theta) + q * cos(theta);
}

double ref_svpwm_duty(double v_phase_shifted, double vbus, int period)
{
    /* duty = (v_normalized + 0.5) * period
     *      = (v_phase_shifted / vbus + 0.5) * period */
    return (v_phase_shifted / vbus + 0.5) * (double)period;
}
