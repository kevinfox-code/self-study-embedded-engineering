/**
 * @file task_config.h
 * @brief Application task priorities/stacks and the NVIC priority map.
 *
 * Layer: App/Tasks (portable).
 *
 * Decoupling rule upheld: pure configuration constants — no HAL, no
 * constants.h.  RTOS priority values are plain integers matching
 * CMSIS-RTOS v2 osPriority_t levels so this header stays portable.
 *
 * The FOC library's tasks (tsk_current, tsk_speed, tsk_supervisor) are
 * created by foc-lib/src/port/stm32u5/freertos_tasks.c inside
 * foc_app_init().  NEVER create them here or in CubeMX — doing so
 * duplicates them (cubemx_setup_guide.md §7).
 *
 * NVIC priority map (single source of truth: cubemx_setup_guide.md §8;
 * configMAX_SYSCALL_INTERRUPT_PRIORITY = 5, 4 priority bits):
 *
 *   IRQ                  Prio  RTOS calls?  Role
 *   TIM1 Break             0   No           Hardware PWM kill (nFAULT/BKIN)
 *   EXTI (nFAULT)          2   No           Software fault latch + PWM disable
 *   ADC1 (JEOC) fast loop  5   Yes          foc_ctrl_fast_loop() + thread flag
 *   SPI (if IRQ mode)     10   Yes          DRV8323 transfers
 *   SysTick / PendSV      15   —            RTOS
 */
#ifndef APP_TASK_CONFIG_H
#define APP_TASK_CONFIG_H

/* Application-task priorities (CMSIS-RTOS v2 osPriority_t values).
 * Keep app tasks BELOW the library's control tasks. */
#define APP_TASK_PRIO_TELEMETRY   24u  /* osPriorityNormal */

/* Application-task stack sizes (32-bit words). */
#define APP_STACK_TELEMETRY_WORDS 256u

/* Telemetry period in RTOS ticks (1 kHz tick → ms). */
#define APP_TELEMETRY_PERIOD_TICKS 100u

#endif /* APP_TASK_CONFIG_H */
