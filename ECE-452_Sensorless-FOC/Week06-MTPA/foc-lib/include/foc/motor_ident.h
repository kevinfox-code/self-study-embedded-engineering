/**
 * @file motor_ident.h
 * @brief Motor parameter identification (§5.12).
 *        Rs (two-point DC), Ls (square-wave Goertzel), λm (open-loop spin).
 *
 * Layer: B  Dependencies: motor_types.h, motor_params.h, motor_observer.h.
 */
#ifndef FOC_MOTOR_IDENT_H
#define FOC_MOTOR_IDENT_H

#include "motor_types.h"
#include "motor_params.h"
#include "motor_observer.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Ident configuration
 * ---------------------------------------------------------------------- */
typedef struct {
    amps_q16_t  i_rs_a;         /**< Rs excitation current [A]    Q16.16 */
    uint16_t    rs_avg_ms;      /**< Rs averaging window  [ms]           */
    q16_t       ls_freq_hz;     /**< Ls excitation freq   [Hz]    Q16.16 */
    volts_q16_t v_ls;           /**< Ls square-wave amplitude [V]  Q16.16 */
    radps_q16_t ke_speed_radps_m; /**< λm spin speed, MECHANICAL rad/s Q16.16.
                                   *   The λm cross-check uses electrical ω,
                                   *   computed transiently (64-bit) at that
                                   *   one computation point (§5.12, §7.2). */
    uint16_t    ke_avg_ms;      /**< λm averaging window  [ms]           */
} foc_ident_cfg_t;

/* -------------------------------------------------------------------------
 * Per-stage quality result
 * ---------------------------------------------------------------------- */
typedef struct {
    q16_t rs_quality;   /**< Rs quality metric [0,1] Q16.16 */
    q16_t ls_quality;   /**< Ls quality metric [0,1] Q16.16 */
    q16_t lm_quality;   /**< λm quality metric [0,1] Q16.16 */
    foc_bool_t rs_pass;
    foc_bool_t ls_pass;
    foc_bool_t lm_pass;
    foc_bool_t overall_pass;
} foc_ident_quality_t;

/* -------------------------------------------------------------------------
 * Ident result (Decision D-011: also aliased as foc_ident_result)
 * ---------------------------------------------------------------------- */
typedef struct {
    foc_motor_params_t params;   /**< Identified plant parameters     */
    foc_ident_quality_t quality; /**< Quality/acceptance report       */
} foc_ident_result_t;

/* Alias for plan §5.13 reference without _t suffix. */
typedef foc_ident_result_t foc_ident_result;

/* -------------------------------------------------------------------------
 * Ident stage sequencer
 * ---------------------------------------------------------------------- */
typedef enum {
    IDENT_STAGE_RS_1 = 0,
    IDENT_STAGE_RS_2,
    IDENT_STAGE_LS,
    IDENT_STAGE_LAMBDA,
    IDENT_STAGE_DONE,
    IDENT_STAGE_FAIL
} foc_ident_stage_t;

/* -------------------------------------------------------------------------
 * Ident context
 * ---------------------------------------------------------------------- */
typedef struct {
    foc_ident_cfg_t      cfg;
    const foc_motor_params_t *params_in;
    const foc_tuning_t   *tuning;

    foc_ident_stage_t    stage;
    uint32_t             stage_start_ms;
    uint32_t             ms_now;

    /* Rs accumulators */
    q16_t    vd_acc_1;   /**< Vd average at current level 1 */
    q16_t    id_acc_1;   /**< Id average at current level 1 */
    uint32_t acc_count_1;
    q16_t    vd_acc_2;
    q16_t    id_acc_2;
    uint32_t acc_count_2;

    /* Ls Goertzel correlation accumulators */
    q16_t    ls_corr_cos; /**< Cosine correlator Q16.16 */
    q16_t    ls_corr_sin; /**< Sine correlator   Q16.16 */
    uint32_t ls_corr_count;
    q16_t    ls_vd_sq_wave; /**< Current square-wave sign × V_ls */
    uint32_t ls_sq_count;   /**< Counter within square-wave half-period */

    /* λm accumulators */
    q16_t    psi_acc;    /**< |ψ| average from observer Q16.16 */
    q16_t    vq_omega_acc; /**< (vq - rs·iq)/ω average     Q16.16 */
    uint32_t lm_acc_count;

    /* Current references emitted to fast loop */
    foc_dq_t i_ref; /**< id_ref / iq_ref [A] Q16.16 */

    /* Result */
    foc_ident_result_t result;
    foc_bool_t         done;
    foc_bool_t         aborted;

    /* Observer reference for λm stage */
    foc_obs_t *obs;
} foc_ident_t;

/* -------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------- */

foc_status_t foc_ident_init(foc_ident_t              *ctx,
                             const foc_motor_params_t *params_in,
                             const foc_tuning_t       *tuning,
                             const motor_limits_t     *limits,
                             foc_obs_t                *obs);

/**
 * Fast-loop step: emit current references while IDENTIFY active.  [ISR]
 * @param ctx        Ident context.
 * @param meas       Current measurement snapshot.
 * @param i_ref_out  Output current reference (id,iq).
 */
void foc_ident_fast_step(foc_ident_t        *ctx,
                          const foc_meas_t   *meas,
                          foc_dq_t           *i_ref_out);

/**
 * Slow-step: sequencing, averaging, quality gates.
 * Call from current task at 1 kHz.
 * @param ctx     Ident context.
 * @param ms_now  Monotonic ms timestamp.
 */
void foc_ident_slow_step(foc_ident_t *ctx, uint32_t ms_now);

/** Fill result if done.  Returns FOC_ENOTREADY if not done yet. */
foc_status_t foc_ident_result_get(foc_ident_t              *ctx,
                                    foc_motor_params_t       *params_out,
                                    foc_ident_quality_t      *quality_out);

/** Abort identification cleanly. */
void foc_ident_abort(foc_ident_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* FOC_MOTOR_IDENT_H */
