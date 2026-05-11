#ifndef CLARKE_H
#define CLARKE_H

/* float32_t matches ARM CMSIS-DSP type; on PC this is plain float. */
typedef float float32_t;

typedef struct {
    float32_t alpha;
    float32_t beta;
    float32_t zero;
} Clarke_Output;

typedef struct {
    float32_t a;
    float32_t b;
    float32_t c;
} ABC_Vector;

/*
 * Amplitude-invariant Clarke transform: converts 3-phase abc to αβ0.
 *
 * Forward:
 *   α = (2/3)( ia  − ib/2       − ic/2      )
 *   β = (2/3)(       (√3/2)·ib  − (√3/2)·ic )
 *   0 = (1/3)( ia  + ib          + ic        )
 *
 * Inverse:
 *   ia =  α              + zero
 *   ib = −α/2 + (√3/2)β + zero
 *   ic = −α/2 − (√3/2)β + zero
 */
Clarke_Output clarke_forward(float32_t ia, float32_t ib, float32_t ic);
ABC_Vector    clarke_inverse(float32_t alpha, float32_t beta, float32_t zero);

#endif /* CLARKE_H */
