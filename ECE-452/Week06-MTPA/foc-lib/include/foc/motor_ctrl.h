/**
 * @file motor_ctrl.h
 * @brief Top-level orchestration — library's public facade (§5.15).
 *
 * Layer: B  Dependencies: all Layer A/B modules.
 *
 * `foc_ctrl_t` owns all module contexts.  The fast-loop, current-task,
 * speed-task, and supervisor-task entry points are here.
 */
#ifndef FOC_MOTOR_CTRL_H
#define FOC_MOTOR_CTRL_H

#include "motor_types.h"
#include "motor_params.h"
#include "motor_limits.h"
#include "motor_math.h"
#include "motor_filter.h"
#include "motor_pi.h"
#include "motor_faults.h"
#include "motor_foc.h"
#include "motor_observer.h"
#include "motor_ident.h"
#include "motor_modeler.h"
#include "motor_sm.h"
#include "motor_tune.h"
#include "motor_hw_if.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Telemetry snapshot (seq-lock, single writer = ISR)
 * ---------------------------------------------------------------------- */
typedef struct {
    uint32_t        seq;      /**< Seq-lock counter (even = stable)       */
    q16_t           id;       /**< d-axis current [A] Q16.16              */
    q16_t           iq;       /**< q-axis current [A] Q16.16              */
    q16_t           vd;       /**< d-axis voltage [V] Q16.16              */
    q16_t           vq;       /**< q-axis voltage [V] Q16.16              */
    angle_t         theta;    /**< Electrical angle (angle_t)             */
    q16_t           omega;    /**< MECHANICAL speed [rad/s_m] Q16.16 (§7.2) */
    q16_t           conf;     /**< Observer confidence [0,1] Q16.16       */
    q16_t           vbus;     /**< Bus voltage [V] Q16.16                 */
    foc_sm_state_t  state;    /**< SM state                               */
    uint32_t        faults;   /**< Active fault bitmask                   */
    uint32_t        wcet_cycles; /**< Fast-loop WCET in CPU cycles        */
} foc_telemetry_t;

/* -------------------------------------------------------------------------
 * Open-loop angle/speed generator state (in ctrl, updated in fast loop)
 * ---------------------------------------------------------------------- */
typedef struct {
    angle_t  theta_forced;     /**< Forced commutation angle               */
    q16_t    omega_forced;     /**< Forced MECHANICAL speed [rad/s_m] Q16.16.
                                *   Electrical angle increment per fast loop
                                *   is derived transiently (64-bit):
                                *   Δθ_e = ω_m × pole_pairs × Ts (§7.2).    */
    q16_t    omega_target;     /**< Ramp target [rad/s_m] Q16.16           */
    q16_t    omega_accel;      /**< Ramp rate [rad/s²_m × Ts] Q16.16       */
    q16_t    blend_k;          /**< Transition blend factor [0,1] Q16.16   */
} foc_ol_gen_t;

/* -------------------------------------------------------------------------
 * Main control block
 * ---------------------------------------------------------------------- */
typedef struct {
    /* Sub-module contexts */
    foc_core_t       foc;
    foc_obs_t        obs;
    foc_faults_t     faults;
    foc_sm_t         sm;
    foc_ident_t      ident;
    foc_tune_t       tune;

    /* Active parameters (double-buffered; pointer swap in critical section) */
    foc_params_t     params_a;
    foc_params_t     params_b;
    foc_params_t    *params_active;  /**< Read by ISR (pointer, M33-atomic) */
    foc_params_t    *params_staging; /**< Written by supervisor then swapped */

    /* Profile table reference */
    const foc_profile_table_t *profiles;

    /* Current references (written by speed/current tasks, read by ISR) */
    q16_t    id_ref;   /**< Q16.16 A, atomic aligned 32-bit */
    q16_t    iq_ref;   /**< Q16.16 A, atomic aligned 32-bit */
    q16_t    speed_ref;/**< MECHANICAL rad/s Q16.16, written by supervisor */

    /* ADC calibration */
    hw_cal_t cal;

    /* Open-loop generator */
    foc_ol_gen_t ol;

    /* Previous applied voltage (for observer) */
    foc_ab_t v_ab_prev;

    /* Speed PI */
    foc_pi_t pi_speed;

    /* I²t integrator */
    q16_t    i2t_acc;

    /* Telemetry */
    foc_telemetry_t telem;

    /* Command queue front-end (single slot; supervisor polls) */
    foc_cmd_t cmd_pending;
    foc_bool_t cmd_valid;

    /* WCET tracking */
    uint32_t fast_loop_wcet_max;

    /* SM input for current slow step */
    foc_sm_in_t sm_in;
    uint32_t    ms_now;

    /* Calibration done/pass flags (set during CALIBRATE by supervisor) */
    foc_bool_t cal_done;
    foc_bool_t cal_pass;
} foc_ctrl_t;

/* -------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------- */

/**
 * Initialise the entire control block.
 * Params are copied from p; profiles table bound by pointer.
 * SM enters INIT state.
 * @return FOC_OK or FOC_EINVAL.
 */
foc_status_t foc_ctrl_init(foc_ctrl_t                *ctx,
                            const foc_params_t        *p,
                            const foc_profile_table_t *profiles);

/**
 * Fast-loop entry point.  [ISR]
 * Called from ADC EOC ISR.  Bounded WCET — no blocking, no divisions
 * except the one SVPWM vbus reciprocal (§7.3 rule).
 */
void foc_ctrl_fast_loop(foc_ctrl_t *ctx);

/** Current task step (1 kHz). */
void foc_ctrl_current_task_step(foc_ctrl_t *ctx);

/** Speed task step (1 kHz). */
void foc_ctrl_speed_task_step(foc_ctrl_t *ctx);

/** Supervisor step (100 Hz).  @param ms  Monotonic millisecond clock. */
void foc_ctrl_supervisor_step(foc_ctrl_t *ctx, uint32_t ms);

/**
 * Submit a command (task context).  Validated then latched for supervisor.
 * @return FOC_EBUSY if a command is already pending.
 */
foc_status_t foc_ctrl_command(foc_ctrl_t *ctx, const foc_cmd_t *cmd);

/** Thread-safe telemetry read (seq-lock). */
void foc_ctrl_get_telemetry(foc_ctrl_t *ctx, foc_telemetry_t *out);

/**
 * Double-buffered parameter commit.
 * Only accepted in IDLE/FAULT; staged swap at next supervisor boundary.
 */
foc_status_t foc_ctrl_apply_params(foc_ctrl_t *ctx, const foc_params_t *p);

#ifdef __cplusplus
}
#endif

#endif /* FOC_MOTOR_CTRL_H */
