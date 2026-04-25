#include "park.h"
#include <math.h>

Park_Output park_forward(float32_t alpha, float32_t beta, float32_t theta)
{
    float32_t c = cosf(theta);
    float32_t s = sinf(theta);
    Park_Output out;
    out.d =  alpha * c + beta * s;
    out.q = -alpha * s + beta * c;
    return out;
}

AlphaBeta_Vector park_inverse(float32_t d, float32_t q, float32_t theta)
{
    float32_t c = cosf(theta);
    float32_t s = sinf(theta);
    AlphaBeta_Vector out;
    out.alpha = d * c - q * s;
    out.beta  = d * s + q * c;
    return out;
}
