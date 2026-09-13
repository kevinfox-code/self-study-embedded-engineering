/**
 * @file motor_modeler.h
 * @brief Motor profile table and parameter matching/resolution (§5.13).
 *
 * Layer: B  Dependencies: motor_types.h, motor_params.h, motor_ident.h.
 */
#ifndef FOC_MOTOR_MODELER_H
#define FOC_MOTOR_MODELER_H

#include "motor_types.h"
#include "motor_params.h"
#include "motor_ident.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Motor profile entry
 * ---------------------------------------------------------------------- */
typedef struct {
    const char         *name;
    foc_motor_params_t  params;
    foc_tuning_t        tuning;
    foc_startup_t       startup;
    motor_limits_t      limits;
} foc_motor_profile_t;

/* -------------------------------------------------------------------------
 * Profile table
 * ---------------------------------------------------------------------- */
typedef struct {
    const foc_motor_profile_t *entries;
    uint32_t                   count;
} foc_profile_table_t;

/* -------------------------------------------------------------------------
 * Resolution policy (Decision D-010)
 * ---------------------------------------------------------------------- */
typedef enum {
    FOC_POLICY_MEASURED_ONLY              = 0,
    FOC_POLICY_PROFILE_ONLY               = 1,
    FOC_POLICY_MEASURED_THEN_PROFILE      = 2,
    FOC_POLICY_PROFILE_TUNING_MEASURED_PLANT = 3
} foc_param_source_t;

#define MODELER_MIN_SCORE  Q16(0.6)  /**< Minimum match score to accept profile. */

/* -------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------- */

/**
 * Find the best-matching profile by normalised distance over (Rs, Ls, λm).
 * @param table       Profile table.
 * @param measured    Measured motor parameters.
 * @param score_out   Match score [0,1] Q16.16 (set to 0 on no match).
 * @return Pointer to best match, or NULL if table empty.
 */
const foc_motor_profile_t *foc_modeler_match(
        const foc_profile_table_t  *table,
        const foc_motor_params_t   *measured,
        q16_t                      *score_out);

/**
 * Resolve final parameter set from ident result + profile table.
 * PI gains recomputed from measured Ls, Rs when policy requires it:
 *   kp = 2π·f_bw·Ls,  ki = 2π·f_bw·Rs.
 *   f_bw_current = 1000 Hz (default).
 *   All in 64-bit integer init math (no float).
 *
 * @param table       Profile table.
 * @param ident_res   Identification result.
 * @param policy      Resolution policy.
 * @param profile_idx Profile index for PROFILE_ONLY/PROFILE_TUNING policies.
 * @param out         Output resolved parameter set.
 * @return FOC_OK, FOC_EINVAL, or FOC_EFAULT (score below threshold).
 */
foc_status_t foc_modeler_resolve(
        const foc_profile_table_t  *table,
        const foc_ident_result     *ident_res,
        foc_param_source_t          policy,
        uint32_t                    profile_idx,
        foc_params_t               *out);

/** Return the default profile table (≥3 example profiles). */
const foc_profile_table_t *foc_modeler_default_table(void);

#ifdef __cplusplus
}
#endif

#endif /* FOC_MOTOR_MODELER_H */
