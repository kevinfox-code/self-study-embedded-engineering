/**
 * @file freertos_tasks.c
 * @brief CMSIS-RTOS v2 task and object creation + library init sequence (§6.8).
 *
 * Layer: D  Only file allowed to include CMSIS-RTOS v2 headers.
 *
 * Init sequence (§6.8 normative order):
 *   1. bsp_init() — DWT, EXTI callback bind.
 *   2. RTOS object creation.
 *   3. Task creation (start blocked on init_done flag).
 *   4. drv8323_init().
 *   5. foc_ctrl_init().
 *   6. PWM prepared, TIM started, ADC armed — verify first EOC within 2 periods.
 *   7. Offset calibration (SM CALIBRATE driven by supervisor).
 *   8. init_done flag → tasks run → SM to IDLE.
 */

#include "constants.h"
/* #include "cmsis_os2.h"  -- include on target */
#include "foc/motor_ctrl.h"
#include "foc/drv8323.h"
#include "board_support.h"

/* Defined in drv8323_transport.c (same layer, no public header). */
extern const drv8323_transport_t *drv8323_transport_get(void);

/* -------------------------------------------------------------------------
 * Global instances
 * ---------------------------------------------------------------------- */
foc_ctrl_t g_foc_ctrl;
static drv8323_t s_drv;

/* RTOS objects — commented for host build; active on target. */
/* static osThreadId_t g_tsk_current;                              */
/* static osThreadId_t g_tsk_speed;                               */
/* static osThreadId_t g_tsk_supervisor;                          */
/* static osEventFlagsId_t g_evt_init_done;                       */
/* static osMessageQueueId_t g_q_cmd;                             */
/* static osMutexId_t g_mtx_params;                               */

/* -------------------------------------------------------------------------
 * Default profile table and parameters
 * ---------------------------------------------------------------------- */
static const foc_params_t s_default_params = {
    FOC_MOTOR_PARAMS_DEFAULT,
    FOC_TUNING_DEFAULT,
    FOC_STARTUP_DEFAULT,
    FOC_LIMITS_DEFAULT
};

static const drv8323_cfg_t s_drv_cfg = {
    DRV_PWM_6X,
    DRV_IDRIVE_120MA, DRV_IDRIVE_120MA,
    DRV_IDRIVE_120MA, DRV_IDRIVE_120MA,
    DRV_TDRIVE_500NS,
    DRV_OCP_LATCHED, DRV_OCP_DEG_8US,
    DRV_VDS_0600,
    DRV_CSA_GAIN_20,
    false, /* csa_vref_div2 */
    0u,    /* sense_ocp_lvl */
    false  /* dis_sen */
};

/* -------------------------------------------------------------------------
 * nFAULT callback (from BSP EXTI, invoked from isr_nfault_exti)
 * ---------------------------------------------------------------------- */
static void nfault_cb(void)
{
    /* ISR context — only atomic ops. */
    hw_pwm_outputs_disable();
    foc_faults_raise(&g_foc_ctrl.faults, FOC_FAULT_OC_HW);
}

/* -------------------------------------------------------------------------
 * Public command API (application calls this)
 * ---------------------------------------------------------------------- */
foc_status_t foc_app_command(const foc_cmd_t *cmd)
{
    return foc_ctrl_command(&g_foc_ctrl, cmd);
}

/* -------------------------------------------------------------------------
 * Library init (call from main after RTOS scheduler start, or from init task)
 * ---------------------------------------------------------------------- */
foc_status_t foc_app_init(void)
{
    /* Step 1: BSP init. */
    bsp_init(nfault_cb);

    /* Step 4: DRV8323 init. */
    const drv8323_transport_t *t = drv8323_transport_get();
    foc_status_t st = drv8323_init(&s_drv, t, &s_drv_cfg);
    if (st != FOC_OK) return st;

    /* Step 5: foc_ctrl_init. */
    st = foc_ctrl_init(&g_foc_ctrl, &s_default_params,
                        foc_modeler_default_table());
    if (st != FOC_OK) return st;

    /* Step 6: PWM outputs disabled until SM enables them. */
    hw_pwm_outputs_disable();

    /* Steps 7–8 handled by SM CALIBRATE state in supervisor task. */
    return FOC_OK;
}

/* -------------------------------------------------------------------------
 * Task bodies (stub implementations for target; compiled only on target)
 * ---------------------------------------------------------------------- */

/* void tsk_current(void *arg) {
 *     (void)arg;
 *     osEventFlagsWait(g_evt_init_done, 0x1u, osFlagsWaitAny, osWaitForever);
 *     for (;;) {
 *         osThreadFlagsWait(0x1u, osFlagsWaitAny, osWaitForever);
 *         foc_ctrl_current_task_step(&g_foc_ctrl);
 *     }
 * }
 *
 * void tsk_speed(void *arg) {
 *     (void)arg;
 *     osEventFlagsWait(g_evt_init_done, 0x1u, osFlagsWaitAny, osWaitForever);
 *     uint32_t tick = osKernelGetTickCount();
 *     for (;;) {
 *         tick += 1u;
 *         osDelayUntil(tick);
 *         foc_ctrl_speed_task_step(&g_foc_ctrl);
 *     }
 * }
 *
 * void tsk_supervisor(void *arg) {
 *     (void)arg;
 *     foc_app_init();
 *     osEventFlagsSet(g_evt_init_done, 0x1u);
 *     uint32_t tick = osKernelGetTickCount();
 *     for (;;) {
 *         tick += 10u;
 *         osDelayUntil(tick);
 *         foc_ctrl_supervisor_step(&g_foc_ctrl, osKernelGetTickCount());
 *     }
 * }
 */
