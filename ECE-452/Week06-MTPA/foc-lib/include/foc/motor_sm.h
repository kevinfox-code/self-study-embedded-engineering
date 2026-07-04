/**
 * @file motor_sm.h
 * @brief Motor state machine (§8 authoritative spec).
 *
 * Layer: B  Dependencies: motor_types.h, motor_faults.h, motor_params.h.
 *
 * State names are normative (§8 table).
 * Action bits consumed by motor_ctrl to route ISR/task behaviour.
 */
#ifndef FOC_MOTOR_SM_H
#define FOC_MOTOR_SM_H

#include "motor_types.h"
#include "motor_faults.h"
#include "motor_params.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * States (§8 normative names)
 * ---------------------------------------------------------------------- */
typedef enum {
    FOC_SM_IDLE             = 0,
    FOC_SM_INIT             = 1,
    FOC_SM_CALIBRATE        = 2,
    FOC_SM_IDENTIFY         = 3,
    FOC_SM_ALIGN            = 4,
    FOC_SM_OPEN_LOOP_START  = 5,
    FOC_SM_OPEN_LOOP_RAMP   = 6,
    FOC_SM_TRANSITION       = 7,
    FOC_SM_CLOSED_LOOP_RUN  = 8,
    FOC_SM_TUNE             = 9,
    FOC_SM_STOPPING         = 10,
    FOC_SM_FAULT            = 11
} foc_sm_state_t;

/* -------------------------------------------------------------------------
 * Action bitset (§5.14 normative names)
 * ---------------------------------------------------------------------- */
#define ACT_PWM_ENABLE        (1u << 0)  /**< Enable PWM outputs           */
#define ACT_RUN_CURRENT_LOOP  (1u << 1)  /**< Execute Id/Iq PI             */
#define ACT_ANGLE_FORCED      (1u << 2)  /**< Use forced angle (not obs)   */
#define ACT_OBS_SHADOW        (1u << 3)  /**< Run observer in shadow mode  */
#define ACT_OBS_ACTIVE        (1u << 4)  /**< Observer controls commutation */
#define ACT_IDENT_ACTIVE      (1u << 5)  /**< Identification excitation on */
#define ACT_TUNE_ACTIVE       (1u << 6)  /**< Tuning mode excitation on    */
#define ACT_ZERO_DUTY         (1u << 7)  /**< Output zero vector           */

/* -------------------------------------------------------------------------
 * Commands
 * ---------------------------------------------------------------------- */
typedef enum {
    FOC_CMD_STOP          = 0,
    FOC_CMD_RUN_SPEED     = 1,
    FOC_CMD_RUN_TORQUE    = 2,
    FOC_CMD_IDENTIFY      = 3,
    FOC_CMD_TUNE          = 4,
    FOC_CMD_CLEAR_FAULTS  = 5,
    FOC_CMD_NONE          = 255
} foc_cmd_op_t;

typedef enum {
    TUNE_TORQUE    = 0,
    TUNE_SPEED     = 1,
    TUNE_OBSERVER  = 2
} foc_tune_mode_t;

typedef struct {
    foc_cmd_op_t   op;
    q16_t          value;        /**< Speed or torque setpoint Q16.16 */
    foc_tune_mode_t tune_mode;
} foc_cmd_t;

/* -------------------------------------------------------------------------
 * SM input struct
 * ---------------------------------------------------------------------- */
typedef struct {
    foc_cmd_t    cmd;            /**< Latest command                   */
    uint32_t     faults_active; /**< From foc_faults_active()         */
    q16_t        obs_conf;      /**< Observer confidence [0,1] Q16.16 */
    q16_t        obs_omega;     /**< Observer speed, MECHANICAL rad/s Q16.16 */
    q16_t        ol_omega;      /**< Open-loop forced speed, MECHANICAL rad/s Q16.16 */
    foc_bool_t   ident_done;    /**< Identification complete           */
    foc_bool_t   ident_pass;    /**< Identification passed quality     */
    foc_bool_t   cal_done;      /**< Calibration done                  */
    foc_bool_t   cal_pass;      /**< Calibration passed                */
    foc_bool_t   tune_done;     /**< Tune sequence done                */
    foc_bool_t   retry_allowed; /**< faults.retry_allowed()            */
    uint32_t     ms_now;        /**< Monotonic ms timestamp            */
} foc_sm_in_t;

/* -------------------------------------------------------------------------
 * SM context
 * ---------------------------------------------------------------------- */
typedef struct {
    foc_sm_state_t  state;
    uint32_t        actions;         /**< Current action bitset             */
    uint32_t        state_entry_ms;  /**< ms timestamp of last state entry  */
    const foc_params_t *params;      /**< Reference to committed params     */

    /* Pending command latched for state routing */
    foc_bool_t      run_pending;
    foc_bool_t      ident_pending;
    foc_bool_t      tune_pending;
    foc_tune_mode_t tune_mode_pending;
    q16_t           speed_ref;       /**< Requested speed (from CMD_RUN_SPEED) */

    /* Transition blend state */
    q16_t           blend_k;         /**< 0.0→1.0 over trans_blend_ms, Q16.16 */
    uint32_t        blend_start_ms;

    /* Convergence hold timer */
    uint32_t        conf_hold_ms;    /**< ms at which conf threshold was first met */
    foc_bool_t      conf_hold_valid;

    /* Retry tracking */
    uint8_t         transition_retry_cnt;
} foc_sm_t;

/* -------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------- */

/** Initialise SM. SM enters IDLE immediately. */
foc_status_t foc_sm_init(foc_sm_t *sm, const foc_params_t *params);

/**
 * Deliver inputs (commands, faults, sensor status).
 * Call from supervisor task before foc_sm_step_slow.
 */
void foc_sm_input(foc_sm_t *sm, const foc_sm_in_t *in);

/**
 * Fast-path SM step.  [ISR]
 * Handles: fatal-fault→FAULT transition, angle-source selection flag.
 * Updates actions.fault_path only.  Bounded, no RTOS calls.
 */
void foc_sm_step_fast(foc_sm_t *sm, uint32_t faults_active);

/**
 * Slow SM step — timeouts, sequenced transitions.
 * Call from supervisor task at 100 Hz.
 * @param sm     SM context.
 * @param in     Latest input snapshot.
 */
void foc_sm_step_slow(foc_sm_t *sm, const foc_sm_in_t *in);

/** Return current state. */
static inline foc_sm_state_t foc_sm_state(const foc_sm_t *sm)
{
    return sm->state;
}

/** Return current action bitset. */
static inline uint32_t foc_sm_actions(const foc_sm_t *sm)
{
    return sm->actions;
}

#ifdef __cplusplus
}
#endif

#endif /* FOC_MOTOR_SM_H */
