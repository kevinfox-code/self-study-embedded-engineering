/**
 * @file SystemInit.h
 * @brief System_* init-policy API — the App's view of system bring-up.
 *
 * Layer: App glue (portable declarations).
 *
 * Decoupling rule upheld: init *policy* lives in App; the generated
 * main.c keeps ownership of SystemClock_Config() and the MX_*_Init()
 * calls (the library consumes peripherals, it never re-initializes
 * them — cubemx_setup_guide.md §9).  SystemInit.c is, besides the
 * foc-lib port layer, the only App file allowed to include constants.h.
 */
#ifndef APP_SYSTEM_INIT_H
#define APP_SYSTEM_INIT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** App-level status codes (portable — no dependency on foc_status_t). */
typedef enum {
    APP_OK = 0,
    APP_ERR_NO_PROFILE,  /**< Built without a CubeMX profile (host stub). */
    APP_ERR_FOC,         /**< foc-lib init reported a fault. */
    APP_ERR_RTOS         /**< RTOS object/task creation failed. */
} app_status_t;

/** True when built with a CubeMX profile (APP_HAVE_CUBEMX_PROFILE). */
bool System_ProfilePresent(void);

/** Pre-scheduler policy hook: anything that must run after MX_*_Init()
 *  but before RTOS objects exist (pin sanity, debug UART banner, ...). */
void System_PreScheduler(void);

/**
 * Bring up the motor-control stack: delegates to the port layer's
 * foc_app_init() (BSP init → DRV8323 init → foc_ctrl_init → PWM safe,
 * §6.8 normative order).  Call from CoreAppMain().
 */
app_status_t System_InitMotorControl(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_SYSTEM_INIT_H */
