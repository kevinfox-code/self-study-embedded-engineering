/*
 * Author:      Kevin Fox
 * Book:        Permanent Magnet Synchronous and Brushless DC Motor Drives
 *              by R. Krishnan — CRC Press, 2010
 * Description: Defines data types and function prototypes for the Park transform, which rotates stationary αβ vectors into the synchronous rotating dq reference frame aligned with the rotor flux.
 */

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
