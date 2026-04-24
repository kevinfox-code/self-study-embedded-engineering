#ifndef PARK_H
#define PARK_H

#include "clarke.h"  /* reuse float32_t */

typedef struct {
    float32_t d;
    float32_t q;
} Park_Output;

typedef struct {
    float32_t alpha;
    float32_t beta;
} AlphaBeta_Vector;

/*
 * Park transform: rotates the stationary αβ frame into the synchronous dq frame.
 *
 * Forward (αβ → dq):
 *   d =  α·cos(θ) + β·sin(θ)
 *   q = −α·sin(θ) + β·cos(θ)
 *
 * Inverse (dq → αβ):
 *   α = d·cos(θ) − q·sin(θ)
 *   β = d·sin(θ) + q·cos(θ)
 *
 * θ is the electrical angle of the rotor (radians).
 */
Park_Output      park_forward(float32_t alpha, float32_t beta, float32_t theta);
AlphaBeta_Vector park_inverse(float32_t d, float32_t q, float32_t theta);

#endif /* PARK_H */
