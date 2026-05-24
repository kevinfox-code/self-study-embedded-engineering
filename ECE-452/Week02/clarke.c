/*
 * Author:      Kevin Fox
 * Book:        Permanent Magnet Synchronous and Brushless DC Motor Drives
 *              by R. Krishnan — CRC Press, 2010
 * Description: Implements the amplitude-invariant Clarke forward transform (abc → αβ0) and inverse transform (αβ0 → abc) used as the first stage in the FOC signal chain.
 */

#include "clarke.h"

/* √3/2 — used for β-axis projection coefficients */
#define SQRT3_OVER_2  0.8660254037844386f

Clarke_Output clarke_forward(float32_t ia, float32_t ib, float32_t ic)
{
    Clarke_Output out;
    out.alpha = (2.0f / 3.0f) * (ia - 0.5f * ib - 0.5f * ic);
    out.beta  = (2.0f / 3.0f) * (SQRT3_OVER_2 * ib - SQRT3_OVER_2 * ic);
    out.zero  = (1.0f / 3.0f) * (ia + ib + ic);
    return out;
}

ABC_Vector clarke_inverse(float32_t alpha, float32_t beta, float32_t zero)
{
    ABC_Vector out;
    out.a =  alpha                        + zero;
    out.b = -0.5f * alpha + SQRT3_OVER_2 * beta + zero;
    out.c = -0.5f * alpha - SQRT3_OVER_2 * beta + zero;
    return out;
}
