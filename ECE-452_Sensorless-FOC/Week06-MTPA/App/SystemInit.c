/**
 * @file SystemInit.c
 * @brief Glue layer between the generated profile and the App.
 *
 * Layer: App glue.  Besides the foc-lib port layer, this is the ONLY
 * App file allowed to include constants.h (and, through it, main.h).
 *
 * Decoupling rule upheld: SystemClock_Config() and MX_*_Init() stay in
 * the generated main.c and run before CoreAppMain() is called; this
 * file owns only post-init *policy* and the library bring-up call.
 */
#include "SystemInit.h"

#if defined(APP_HAVE_CUBEMX_PROFILE)

#include "constants.h"
#include "foc/motor_types.h"

/* Defined in foc-lib/src/port/stm32u5/freertos_tasks.c (no public
 * header by design — only the glue layer may call it). */
extern foc_status_t foc_app_init(void);

bool System_ProfilePresent(void)
{
    return true;
}

void System_PreScheduler(void)
{
    /* Post-MX_*_Init(), pre-RTOS policy goes here (pin sanity checks,
     * debug banner, ...).  Nothing required yet. */
}

app_status_t System_InitMotorControl(void)
{
    return (foc_app_init() == FOC_OK) ? APP_OK : APP_ERR_FOC;
}

#else /* !APP_HAVE_CUBEMX_PROFILE — host stub build */

bool System_ProfilePresent(void)
{
    return false;
}

void System_PreScheduler(void)
{
}

app_status_t System_InitMotorControl(void)
{
    return APP_ERR_NO_PROFILE;
}

#endif /* APP_HAVE_CUBEMX_PROFILE */
