/**
 * @file app_tasks.h
 * @brief Application-level task creation (telemetry, CLI, ...).
 *
 * Layer: App/Tasks (portable).
 *
 * Decoupling rule upheld: exposes only portable declarations; RTOS
 * headers are confined to app_tasks.c (and only under
 * APP_HAVE_CUBEMX_PROFILE).
 */
#ifndef APP_TASKS_H
#define APP_TASKS_H

#include "SystemInit.h"   /* app_status_t */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create application-level tasks.  Call once from CoreAppMain(),
 * after System_InitMotorControl().
 *
 * @return APP_OK, or APP_ERR_NO_PROFILE when built without a CubeMX
 *         profile (host stub build).
 */
app_status_t App_Tasks_Create(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_TASKS_H */
