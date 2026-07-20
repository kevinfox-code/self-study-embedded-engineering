/**
 * @file CoreAppMain.c
 * @brief Application entry: system init policy + app task creation.
 *
 * Layer: App/Entry (portable).
 *
 * Decoupling rule upheld: includes only App-level headers — no HAL,
 * no constants.h.  Hardware bring-up is delegated to SystemInit; the
 * FOC library's own tasks are created inside foc_app_init()
 * (foc-lib/src/port/stm32u5/freertos_tasks.c), never here.
 */
#include "SystemInit.h"
#include "app_tasks.h"

void CoreAppMain(void)
{
    System_PreScheduler();

    /* Library bring-up (§6.8 order) — creates the three motor tasks. */
    (void)System_InitMotorControl();

    /* Application-level tasks (telemetry, CLI, ...). */
    (void)App_Tasks_Create();
}
