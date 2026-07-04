/**
 * @file motor_tune.h
 * @brief User-initiated calibration/tuning mode (§5.21).
 *        TUNE_TORQUE / TUNE_SPEED / TUNE_OBSERVER sub-modes.
 *
 * Layer: B  Dependencies: motor_types.h, motor_params.h, motor_limits.h,
 *            motor_observer.h (getter only).
 */
#ifndef FOC_MOTOR_TUNE_H
#define FOC_MOTOR_TUNE_H

#include "motor_types.h"
#include "motor_params.h"
#include "motor_limits.h"
#include "motor_observer.h"
#include "motor_sm.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Capture buffer depth (Decision D-020)
 * ---------------------------------------------------------------------- */
#ifndef FOC_TUNE_CAP_DEPTH
#define FOC_TUNE_CAP_DEPTH 2048u
#endif

/* Channels per capture sample (4 × int32). */
#define FOC_TUNE_CAP_CH 4u

/* -------------------------------------------------------------------------
 * Capture buffer (ring buffer, single-writer ISR, task drains)
 * ---------------------------------------------------------------------- */
typedef struct {
    int32_t  data[FOC_TUNE_CAP_DEPTH][FOC_TUNE_CAP_CH];
    uint32_t write_idx; /**< Next write position (wraps to 0)         */
    uint32_t read_idx;  /**< Next drain position                      */
    uint32_t seq;       /**< Seq-lock counter (even = stable)         */
    uint16_t decim;     /**< Decimation factor (1 = every fast-loop)  */
    uint16_t decim_cnt; /**< Countdown to next capture                */
} foc_capture_t;

/* -------------------------------------------------------------------------
 * Tune configuration (§5.21)
 * ---------------------------------------------------------------------- */
typedef struct {
    /* Torque sub-mode */
    q16_t   tq_i_hi_a;    /**< High-level current [A]   Q16.16, dflt 1.0 A */
    q16_t   tq_i_lo_a;    /**< Low-level current  [A]   Q16.16, dflt 0.5 A */
    uint16_t tq_step_ms;  /**< Step dwell          [ms], dflt 50           */
    uint8_t  tq_reps;     /**< Repetitions,              dflt 4            */
    /* Speed sub-mode */
    q16_t   sp_min_pct;   /**< Min speed fraction  [0,1] Q16.16            */
    q16_t   sp_max_pct;   /**< Max speed fraction  [0,1] Q16.16            */
    uint16_t sp_dwell_ms; /**< Square-wave dwell   [ms]                    */
    uint8_t  sp_reps;     /**< Half-cycles                                 */
    /* Observer sub-mode */
    radps_q16_t obs_speed_radps_m; /**< Capture speed, MECHANICAL rad/s Q16.16 */
    uint16_t    obs_capture_ms;    /**< Capture duration [ms]             */
    /* Capture */
    uint16_t    decim;             /**< Capture decimation factor         */
} foc_tune_cfg_t;

/* -------------------------------------------------------------------------
 * Tune gain update (live)
 * ---------------------------------------------------------------------- */
typedef struct {
    q16_t pi_iq_kp;
    q16_t pi_iq_ki;
    q16_t pi_speed_kp;
    q16_t pi_speed_ki;
    q16_t obs_hpf_fc;
    q16_t pll_kp;
    q16_t pll_ki;
    q16_t lpf_speed_hz;
    foc_bool_t reset_pis;
} foc_tune_gains_t;

/* -------------------------------------------------------------------------
 * Output refs from foc_tune_fast_step (Decision D-008)
 * ---------------------------------------------------------------------- */
typedef struct {
    q16_t      id_ref;        /**< d-axis current reference [A] Q16.16 */
    q16_t      iq_ref;        /**< q-axis current reference [A] Q16.16 */
    q16_t      omega_ref;     /**< Speed reference, MECHANICAL rad/s Q16.16 */
    foc_bool_t override_refs; /**< True if tune is overriding cascade  */
} foc_tune_refs_t;

/* -------------------------------------------------------------------------
 * Tune sequencer context
 * ---------------------------------------------------------------------- */
typedef struct {
    foc_tune_mode_t  mode;
    foc_tune_cfg_t   cfg;
    const foc_params_t     *params;
    const motor_limits_t   *limits;

    /* Sequencer state */
    uint8_t  phase;        /**< Step within sequence (rep index, etc.)   */
    uint8_t  rep;          /**< Current repetition                        */
    uint32_t phase_start_ms; /**< ms at phase entry                      */
    uint32_t ms_now;

    /* Done / abort flags */
    foc_bool_t done;
    foc_bool_t aborted;

    /* Live gain staging */
    foc_tune_gains_t staged_gains;
    foc_bool_t       gains_pending;

    /* Capture buffer */
    foc_capture_t    cap;

    /* Last computed refs */
    foc_tune_refs_t  last_refs;
} foc_tune_t;

/* -------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------- */

/**
 * Initialise tune module.
 * Clamps all cfg fields against limits; returns FOC_EINVAL on violation.
 */
foc_status_t foc_tune_init(foc_tune_t            *ctx,
                            foc_tune_mode_t        mode,
                            const foc_tune_cfg_t  *cfg,
                            const foc_params_t    *params,
                            const motor_limits_t  *limits);

/**
 * Fast-loop step: emit iq_ref/id_ref or omega_ref override + capture write.  [ISR]
 * @param ctx   Tune context.
 * @param meas  Current fast-loop measurement snapshot.
 * @param obs   Observer context (for TUNE_OBSERVER capture).
 * @param out   Output references.
 */
void foc_tune_fast_step(foc_tune_t           *ctx,
                         const foc_meas_t     *meas,
                         const foc_obs_t      *obs,
                         foc_tune_refs_t      *out);

/**
 * Slow-step sequencer: dwell timing, rep counting.
 * Call from current task at 1 kHz (or slow supervisor).
 * @param ctx    Tune context.
 * @param ms_now Monotonic ms.
 */
void foc_tune_slow_step(foc_tune_t *ctx, uint32_t ms_now);

/** Return true if sequence is done. */
static inline foc_bool_t foc_tune_done(const foc_tune_t *ctx)
{
    return ctx->done;
}

/** Abort: zero all refs, set done flag. */
void foc_tune_abort(foc_tune_t *ctx);

/**
 * Stage a gain update (task context only; rejected outside TUNE state).
 * Committed by motor_ctrl at next current-task boundary.
 * @return FOC_ENOTREADY if not in TUNE.
 */
foc_status_t foc_tune_set_gains(foc_tune_t           *ctx,
                                  const foc_tune_gains_t *gains,
                                  foc_sm_state_t          sm_state);

/**
 * Drain capture buffer (task context, seq-lock consistent).
 * @param ctx      Tune context.
 * @param dst      Destination buffer (4 int32 per sample).
 * @param max      Maximum samples to drain.
 * @param n_out    Actual samples drained.
 */
foc_status_t foc_tune_capture_read(foc_tune_t *ctx,
                                    int32_t (*dst)[FOC_TUNE_CAP_CH],
                                    uint32_t max, uint32_t *n_out);

#ifdef __cplusplus
}
#endif

#endif /* FOC_MOTOR_TUNE_H */
