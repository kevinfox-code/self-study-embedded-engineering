/**
 * @file app_tasks.c
 * @brief Application-level task creation and bodies.
 *
 * Layer: App/Tasks.  RTOS calls live inside the APP_HAVE_CUBEMX_PROFILE
 * gate (mirrors the commented pattern in freertos_tasks.c); the file
 * compiles as a portable stub on the host.
 *
 * Decoupling rule upheld: no HAL access, no constants.h — hardware is
 * reached only through the BSP facade / foc public API.
 */
#include "app_tasks.h"
#include "task_config.h"

#if defined(APP_HAVE_CUBEMX_PROFILE)
#include "cmsis_os2.h"

static osThreadId_t s_tsk_telemetry;

static void tsk_telemetry(void *arg)
{
    (void)arg;
    uint32_t tick = osKernelGetTickCount();
    for (;;) {
        tick += APP_TELEMETRY_PERIOD_TICKS;
        osDelayUntil(tick);
        /* TODO: stream foc state (g_foc_ctrl telemetry accessor) over
         * UART/CLI once a Communication/ subsystem exists. */
    }
}

app_status_t App_Tasks_Create(void)
{
    const osThreadAttr_t attr = {
        .name       = "tsk_telemetry",
        .stack_size = APP_STACK_TELEMETRY_WORDS * 4u,
        .priority   = (osPriority_t)APP_TASK_PRIO_TELEMETRY,
    };
    s_tsk_telemetry = osThreadNew(tsk_telemetry, NULL, &attr);
    return (s_tsk_telemetry != NULL) ? APP_OK : APP_ERR_RTOS;
}

#else /* !APP_HAVE_CUBEMX_PROFILE — host stub build */

app_status_t App_Tasks_Create(void)
{
    return APP_ERR_NO_PROFILE;
}

#endif /* APP_HAVE_CUBEMX_PROFILE */
