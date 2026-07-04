/**
 * @file constants.h
 * @brief THE ONLY bridge file between library core and CubeMX/HAL symbols.
 *        Edit this file when porting to a new board.
 *
 * Layer: C  Included ONLY by: board_support.c, motor_hw_if.c,
 *            drv8323_transport.c, isr_motor.c, freertos_tasks.c.
 *            CI grep rule enforces this — any core file including this
 *            header fails the build.
 *
 * Porting checklist:
 *   1. Set BSP_TIM_PWM_HANDLE to the CubeMX-generated TIM handle name.
 *   2. Set ADC handle and injected rank macros.
 *   3. Set SPI handle for DRV8323.
 *   4. Set GPIO port/pin macros for ENABLE and nFAULT.
 *   5. Set BSP_PWM_PERIOD_TICKS to match timer ARR value.
 *   6. Set current-scaling constants (shunt, CSA gain, VREF).
 *   7. Set clock frequency BSP_CPU_FREQ_HZ.
 */
#ifndef FOC_CONSTANTS_H
#define FOC_CONSTANTS_H

/* NOTE: On target, include the CubeMX-generated main.h here so that
 * htim1, hadc1, hspi1 etc. are visible.  Left as placeholder. */
/* #include "main.h"  */

/* -------------------------------------------------------------------------
 * PWM Timer (TIM1, advanced, center-aligned, 20 kHz)
 * ---------------------------------------------------------------------- */
/* #define BSP_TIM_PWM_HANDLE  (&htim1) */
#define BSP_TIM_CHANNEL_A   TIM_CHANNEL_1
#define BSP_TIM_CHANNEL_B   TIM_CHANNEL_2
#define BSP_TIM_CHANNEL_C   TIM_CHANNEL_3
/** Timer auto-reload register value (period ticks).
 *  For 160 MHz / 2 (center-aligned) / 20 kHz = 4000 ticks. */
#define BSP_PWM_PERIOD_TICKS  4000u

/* -------------------------------------------------------------------------
 * ADC (injected conversions, hardware-triggered by TIM1 update event)
 * ---------------------------------------------------------------------- */
/* #define BSP_ADC_HANDLE          (&hadc1) */
#define BSP_ADC_RANK_IA         ADC_INJECTED_RANK_1
#define BSP_ADC_RANK_IB         ADC_INJECTED_RANK_2
#define BSP_ADC_RANK_IC         ADC_INJECTED_RANK_3
#define BSP_ADC_RANK_VBUS       ADC_INJECTED_RANK_4
#define BSP_ADC_FULLSCALE       4096u   /**< 12-bit ADC */

/* -------------------------------------------------------------------------
 * Current sensing
 * ---------------------------------------------------------------------- */
#define BSP_SHUNT_MOHM          Q16(10.0)  /**< 10 mΩ shunt         Q16.16 */
#define BSP_CSA_GAIN_DEFAULT    DRV_CSA_GAIN_20 /**< 20 V/V            */
#define BSP_VBUS_DIVIDER_R1     Q16(51000.0)  /**< Upper divider R [Ω] Q16.16 */
#define BSP_VBUS_DIVIDER_R2     Q16(3300.0)   /**< Lower divider R [Ω] Q16.16 */
#define BSP_ADC_VREF_V          Q16(3.3)     /**< ADC reference [V]   Q16.16 */

/* -------------------------------------------------------------------------
 * SPI (DRV8323, mode 1, ≤10 MHz, 16-bit frame)
 * ---------------------------------------------------------------------- */
/* #define BSP_SPI_DRV_HANDLE      (&hspi1) */

/* -------------------------------------------------------------------------
 * GPIO
 * ---------------------------------------------------------------------- */
/* #define BSP_GPIO_DRV_EN_PORT    GPIOA */
/* #define BSP_GPIO_DRV_EN_PIN     GPIO_PIN_8 */
/* #define BSP_GPIO_NFAULT_PORT    GPIOB */
/* #define BSP_GPIO_NFAULT_PIN     GPIO_PIN_0 */
/* #define BSP_NFAULT_EXTI_IRQn    EXTI0_IRQn */

/* -------------------------------------------------------------------------
 * Clock
 * ---------------------------------------------------------------------- */
#define BSP_CPU_FREQ_HZ     160000000u  /**< 160 MHz Cortex-M33 */

/* -------------------------------------------------------------------------
 * RTOS (CMSIS-RTOS v2 task stack sizes)
 * ---------------------------------------------------------------------- */
#define BSP_STACK_CURRENT_WORDS  256u
#define BSP_STACK_SPEED_WORDS    256u
#define BSP_STACK_SUPERVISOR_WORDS 512u

#endif /* FOC_CONSTANTS_H */
