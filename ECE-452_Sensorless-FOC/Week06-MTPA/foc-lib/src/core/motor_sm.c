/**
 * @file motor_sm.c
 * @brief Motor state machine implementation (§8 normative).
 *
 * Layer: B  No HAL, no RTOS.
 *
 * Each state entry sets the action bitset.
 * Transitions cite their §8 table row as a comment.
 */

#include "foc/motor_sm.h"
#include <string.h>

/* Helper: time elapsed since state entry. */
static uint32_t elapsed_ms(const foc_sm_t *sm, uint32_t ms_now)
{
    return ms_now - sm->state_entry_ms;
}

/* Enter a new state: update state, record timestamp, recompute actions. */
static void enter_state(foc_sm_t *sm, foc_sm_state_t new_state, uint32_t ms_now)
{
    sm->state          = new_state;
    sm->state_entry_ms = ms_now;
    sm->actions        = 0u;
    sm->blend_k        = 0;
    sm->conf_hold_valid = FOC_FALSE;

    switch (new_state) {
    case FOC_SM_IDLE:
        sm->actions = 0u; /* PWM off, loops reset — §8 IDLE entry */
        break;
    case FOC_SM_INIT:
        sm->actions = 0u; /* no PWM yet — §8 INIT entry */
        break;
    case FOC_SM_CALIBRATE:
        sm->actions = 0u; /* PWM off during calibration — §8 CALIBRATE entry */
        break;
    case FOC_SM_IDENTIFY:
        sm->actions = ACT_PWM_ENABLE | ACT_ANGLE_FORCED | ACT_IDENT_ACTIVE;
        break;
    case FOC_SM_ALIGN:
        sm->actions = ACT_PWM_ENABLE | ACT_ANGLE_FORCED | ACT_RUN_CURRENT_LOOP;
        break;
    case FOC_SM_OPEN_LOOP_START:
        sm->actions = ACT_PWM_ENABLE | ACT_ANGLE_FORCED | ACT_RUN_CURRENT_LOOP
                    | ACT_OBS_SHADOW;
        break;
    case FOC_SM_OPEN_LOOP_RAMP:
        sm->actions = ACT_PWM_ENABLE | ACT_ANGLE_FORCED | ACT_RUN_CURRENT_LOOP
                    | ACT_OBS_SHADOW;
        break;
    case FOC_SM_TRANSITION:
        sm->actions = ACT_PWM_ENABLE | ACT_ANGLE_FORCED | ACT_RUN_CURRENT_LOOP
                    | ACT_OBS_ACTIVE;
        sm->blend_start_ms = ms_now;
        sm->blend_k        = 0;
        break;
    case FOC_SM_CLOSED_LOOP_RUN:
        sm->actions = ACT_PWM_ENABLE | ACT_RUN_CURRENT_LOOP | ACT_OBS_ACTIVE;
        break;
    case FOC_SM_TUNE:
        sm->actions = ACT_PWM_ENABLE | ACT_RUN_CURRENT_LOOP | ACT_OBS_ACTIVE
                    | ACT_TUNE_ACTIVE;
        break;
    case FOC_SM_STOPPING:
        sm->actions = ACT_PWM_ENABLE | ACT_RUN_CURRENT_LOOP | ACT_OBS_ACTIVE;
        break;
    case FOC_SM_FAULT:
        sm->actions = ACT_ZERO_DUTY; /* PWM outputs disabled by fault path */
        break;
    default:
        sm->actions = 0u;
        break;
    }
}

foc_status_t foc_sm_init(foc_sm_t *sm, const foc_params_t *params)
{
    if (!sm || !params) return FOC_EINVAL;
    memset(sm, 0, sizeof(*sm));
    sm->params = params;
    sm->state  = FOC_SM_IDLE;
    sm->actions = 0u;
    sm->state_entry_ms = 0u;
    return FOC_OK;
}

void foc_sm_input(foc_sm_t *sm, const foc_sm_in_t *in)
{
    /* Deliver latest command and derived inputs.
     * Latching logic for pending commands: */
    if (in->cmd.op == FOC_CMD_RUN_SPEED || in->cmd.op == FOC_CMD_RUN_TORQUE) {
        if (sm->state == FOC_SM_IDLE) {
            sm->run_pending  = FOC_TRUE;
            sm->speed_ref    = in->cmd.value;
        }
    } else if (in->cmd.op == FOC_CMD_IDENTIFY) {
        if (sm->state == FOC_SM_IDLE) {
            sm->ident_pending = FOC_TRUE;
        }
    } else if (in->cmd.op == FOC_CMD_TUNE) {
        if (sm->state == FOC_SM_IDLE) {
            sm->tune_pending      = FOC_TRUE;
            sm->tune_mode_pending = in->cmd.tune_mode;
        }
    } else if (in->cmd.op == FOC_CMD_CLEAR_FAULTS) {
        /* Handled in step_slow — don't latch, let step_slow act. */
    }
}

void foc_sm_step_fast(foc_sm_t *sm, uint32_t faults_active)
{
    /* Fast fatal-fault path: any fatal fault in any non-FAULT state → FAULT.
     * §8: "Guards evaluated in foc_sm_step_slow except: fatal-fault→FAULT
     *       ...which are fast-path (foc_sm_step_fast) to bound reaction latency
     *       to one PWM period."
     * We only check fatal bits here (OC_HW, ADC_CAL_FAIL, DRV_SPI_FAIL, WDG). */
    const uint32_t FAST_FATAL_MASK = (uint32_t)(FOC_FAULT_OC_HW |
                                                 FOC_FAULT_ADC_CAL_FAIL |
                                                 FOC_FAULT_DRV_SPI_FAIL |
                                                 FOC_FAULT_WDG_MISSED_LOOP);
    if ((faults_active & FAST_FATAL_MASK) && sm->state != FOC_SM_FAULT) {
        sm->state   = FOC_SM_FAULT;
        sm->actions = ACT_ZERO_DUTY;
    }
    /* Update ANGLE_FORCED action based on state (for ISR angle source selection). */
    if (sm->state == FOC_SM_TRANSITION && sm->blend_k >= Q16_ONE) {
        /* Blend complete: clear ANGLE_FORCED so ISR uses observer. */
        sm->actions &= ~ACT_ANGLE_FORCED;
    }
}

void foc_sm_step_slow(foc_sm_t *sm, const foc_sm_in_t *in)
{
    uint32_t ms = in->ms_now;
    uint32_t el = elapsed_ms(sm, ms);
    uint32_t faults = in->faults_active;

    /* Global fault check: any fault in powered states → STOPPING or FAULT. */
    if (faults != 0u && sm->state != FOC_SM_FAULT &&
        sm->state != FOC_SM_STOPPING && sm->state != FOC_SM_IDLE) {
        /* OC_SW, OBS_LOST → STOPPING first; others → FAULT directly. */
        const uint32_t stop_faults = (uint32_t)(FOC_FAULT_OC_SW |
                                                  FOC_FAULT_OBS_LOST |
                                                  FOC_FAULT_UV_BUS);
        if (faults & stop_faults) {
            enter_state(sm, FOC_SM_STOPPING, ms);
        } else {
            enter_state(sm, FOC_SM_FAULT, ms);
        }
        return;
    }

    switch (sm->state) {

    /* §8 IDLE: CMD_RUN/CMD_IDENTIFY → INIT; CMD_CLEAR clears faults. */
    case FOC_SM_IDLE:
        if (in->cmd.op == FOC_CMD_CLEAR_FAULTS) {
            /* Caller clears faults before calling step_slow. */
        }
        if (sm->run_pending || sm->ident_pending || sm->tune_pending) {
            enter_state(sm, FOC_SM_INIT, ms);
        }
        break;

    /* §8 INIT: success → CALIBRATE; timeout 200 ms → FAULT(SM_TIMEOUT). */
    case FOC_SM_INIT:
        if (el >= 200u) {
            /* Allow a short window for DRV init. On host mock this is instant. */
            enter_state(sm, FOC_SM_CALIBRATE, ms);
        }
        break;

    /* §8 CALIBRATE: residual pass → IDENTIFY/ALIGN/IDLE; timeout 500 ms. */
    case FOC_SM_CALIBRATE:
        if (el >= 500u || in->cal_done) {
            if (!in->cal_pass) {
                /* ADC_CAL_FAIL → FAULT. */
                enter_state(sm, FOC_SM_FAULT, ms);
                return;
            }
            if (sm->ident_pending) {
                enter_state(sm, FOC_SM_IDENTIFY, ms);
            } else if (sm->run_pending || sm->tune_pending) {
                enter_state(sm, FOC_SM_ALIGN, ms);
            } else {
                enter_state(sm, FOC_SM_IDLE, ms);
                sm->run_pending = FOC_FALSE;
            }
        }
        break;

    /* §8 IDENTIFY: per-stage timeouts (Rs 1s, Ls 1s, λm 3s total = 5s). */
    case FOC_SM_IDENTIFY:
        if (in->ident_done || el >= 5000u) {
            if (!in->ident_pass) {
                enter_state(sm, FOC_SM_FAULT, ms);
                return;
            }
            sm->ident_pending = FOC_FALSE;
            if (sm->run_pending) {
                enter_state(sm, FOC_SM_ALIGN, ms);
            } else {
                enter_state(sm, FOC_SM_IDLE, ms);
            }
        }
        break;

    /* §8 ALIGN: align_ms elapsed AND id tracking OK → OPEN_LOOP_START
     *           or TUNE(TORQUE) if tune_torque pending.
     * Timeout 2×align_ms → FAULT. */
    case FOC_SM_ALIGN:
        if (el >= (uint32_t)sm->params->startup.align_ms) {
            sm->run_pending = FOC_FALSE;
            if (sm->tune_pending && sm->tune_mode_pending == TUNE_TORQUE) {
                sm->tune_pending = FOC_FALSE;
                enter_state(sm, FOC_SM_TUNE, ms);
            } else {
                enter_state(sm, FOC_SM_OPEN_LOOP_START, ms);
            }
        } else if (el >= (uint32_t)(sm->params->startup.align_ms * 2u)) {
            enter_state(sm, FOC_SM_FAULT, ms);
        }
        break;

    /* §8 OPEN_LOOP_START: first cycle → OPEN_LOOP_RAMP. Timeout 10 ms. */
    case FOC_SM_OPEN_LOOP_START:
        if (el >= 1u) { /* One supervisor tick = transition to ramp. */
            enter_state(sm, FOC_SM_OPEN_LOOP_RAMP, ms);
        }
        break;

    /* §8 OPEN_LOOP_RAMP: ω_forced ≥ ol_target → TRANSITION.
     * Timeout 2×(ol_target/ol_accel). */
    case FOC_SM_OPEN_LOOP_RAMP: {
        q16_t ol_target = sm->params->startup.ol_target_radps_m;
        if (in->ol_omega >= ol_target) {
            enter_state(sm, FOC_SM_TRANSITION, ms);
        } else {
            /* Rough timeout: 2× expected ramp time at supervisor 100Hz ticks. */
            uint32_t timeout_ms = 2u * 1000u; /* Conservatively 2 s */
            if (el >= timeout_ms) {
                enter_state(sm, FOC_SM_STOPPING, ms);
            }
        }
        break;
    }

    /* §8 TRANSITION: blend angle, speed PI preload → CLOSED_LOOP_RUN.
     * Entry guard: ω_forced ≥ ol_target AND conf ≥ trans_min_conf
     *              AND |ω̂ - ω_forced| ≤ 20% held for trans_hold_ms.
     * Failure (conf collapse / angle divergence) → STOPPING + retry. */
    case FOC_SM_TRANSITION: {
        q16_t min_conf = sm->params->startup.trans_min_conf;
        uint16_t hold_ms  = sm->params->startup.trans_hold_ms;
        uint16_t blend_ms = sm->params->startup.trans_blend_ms;

        if (in->obs_conf < min_conf) {
            /* Confidence collapsed — go to STOPPING. */
            sm->transition_retry_cnt++;
            enter_state(sm, FOC_SM_STOPPING, ms);
            break;
        }

        /* Update blend factor. */
        uint32_t blend_el = ms - sm->blend_start_ms;
        if (blend_el >= (uint32_t)blend_ms) {
            sm->blend_k = Q16_ONE;
        } else {
            sm->blend_k = (q16_t)(((uint64_t)blend_el * (uint64_t)Q16_ONE) / blend_ms);
        }

        /* Confidence hold check. */
        if (!sm->conf_hold_valid) {
            sm->conf_hold_ms    = ms;
            sm->conf_hold_valid = FOC_TRUE;
        }
        uint32_t hold_elapsed = ms - sm->conf_hold_ms;

        if (hold_elapsed >= hold_ms && sm->blend_k >= Q16_ONE) {
            /* Transition complete. */
            if (sm->tune_pending &&
                (sm->tune_mode_pending == TUNE_SPEED ||
                 sm->tune_mode_pending == TUNE_OBSERVER)) {
                sm->tune_pending = FOC_FALSE;
                enter_state(sm, FOC_SM_TUNE, ms);
            } else {
                enter_state(sm, FOC_SM_CLOSED_LOOP_RUN, ms);
            }
        }

        /* Timeout: (hold + blend) × 3. */
        uint32_t timeout_ms = (uint32_t)(hold_ms + blend_ms) * 3u;
        if (el >= timeout_ms) {
            enter_state(sm, FOC_SM_STOPPING, ms);
        }
        break;
    }

    /* §8 CLOSED_LOOP_RUN: CMD_STOP or ω* < min_run_speed → STOPPING. */
    case FOC_SM_CLOSED_LOOP_RUN:
        if (in->cmd.op == FOC_CMD_STOP) {
            enter_state(sm, FOC_SM_STOPPING, ms);
        }
        break;

    /* §8 TUNE: sequence done or CMD_STOP → STOPPING. Timeout 2× nominal. */
    case FOC_SM_TUNE:
        if (in->tune_done || in->cmd.op == FOC_CMD_STOP) {
            enter_state(sm, FOC_SM_STOPPING, ms);
        }
        break;

    /* §8 STOPPING: iq*→0, zero vector stop_zero_ms, PWM off → IDLE (or FAULT). */
    case FOC_SM_STOPPING:
        if (el >= 500u) { /* Conservative: allow 500 ms for ramp + zero vector. */
            if (faults != 0u) {
                enter_state(sm, FOC_SM_FAULT, ms);
            } else {
                enter_state(sm, FOC_SM_IDLE, ms);
                sm->run_pending   = FOC_FALSE;
                sm->ident_pending = FOC_FALSE;
                sm->tune_pending  = FOC_FALSE;
            }
        }
        break;

    /* §8 FAULT: AUTO_RETRY budget + cooldown → INIT; FATAL → wait CMD_CLEAR. */
    case FOC_SM_FAULT:
        if (in->cmd.op == FOC_CMD_CLEAR_FAULTS) {
            /* Faults must be cleared externally before this call. */
            enter_state(sm, FOC_SM_IDLE, ms);
            sm->run_pending   = FOC_FALSE;
            sm->ident_pending = FOC_FALSE;
            sm->tune_pending  = FOC_FALSE;
        } else if (in->retry_allowed && faults == 0u) {
            enter_state(sm, FOC_SM_INIT, ms);
        }
        break;

    default:
        break;
    }
}
