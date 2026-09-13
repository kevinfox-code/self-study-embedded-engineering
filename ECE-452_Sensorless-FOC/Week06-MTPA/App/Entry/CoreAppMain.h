/**
 * @file CoreAppMain.h
 * @brief Application entry point called from CubeMX-generated main().
 *
 * Layer: App/Entry (portable).
 *
 * Decoupling rule upheld: this is the single call the generated main.c
 * makes into the application; everything else flows through SystemInit
 * and the BSP.  No HAL types cross this boundary.
 */
#ifndef APP_CORE_APP_MAIN_H
#define APP_CORE_APP_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Application entry.  Call from the generated main.c inside the
 * USER CODE BEGIN 2 markers — after all MX_*_Init() calls, before
 * osKernelStart():
 *
 * @code
 *   CoreAppMain();
 * @endcode
 */
void CoreAppMain(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_CORE_APP_MAIN_H */
