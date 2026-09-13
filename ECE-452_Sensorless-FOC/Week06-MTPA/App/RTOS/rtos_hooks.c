/**
 * @file rtos_hooks.c
 * @brief FreeRTOS application hooks — safe-stop on fatal RTOS errors.
 *
 * Layer: App/RTOS.  Compiled into the firmware only when a CubeMX
 * profile (which provides FreeRTOS) is present; empty translation unit
 * on the host stub build.
 *
 * Decoupling rule upheld: reaches hardware only through the BSP facade
 * (bsp_pwm_outputs_disable) — no HAL, no constants.h.
 *
 * NOTE: enable these hooks in FreeRTOSConfig.h (CubeMX-generated into
 * the profile): configCHECK_FOR_STACK_OVERFLOW = 2,
 * configUSE_MALLOC_FAILED_HOOK = 1.
 */
#if defined(APP_HAVE_CUBEMX_PROFILE)

#include "bsp.h"
#include "FreeRTOS.h"
#include "task.h"

/* Fatal RTOS errors: kill the power stage first, then halt. */

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    bsp_pwm_outputs_disable();
    taskDISABLE_INTERRUPTS();
    for (;;) { }
}

void vApplicationMallocFailedHook(void)
{
    bsp_pwm_outputs_disable();
    taskDISABLE_INTERRUPTS();
    for (;;) { }
}

#else /* !APP_HAVE_CUBEMX_PROFILE */

/* Host stub build: keep the translation unit non-empty (ISO C). */
typedef int app_rtos_hooks_host_stub_t;

#endif /* APP_HAVE_CUBEMX_PROFILE */
