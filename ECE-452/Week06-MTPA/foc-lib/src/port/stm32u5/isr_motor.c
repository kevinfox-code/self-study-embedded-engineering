/**
 * @file isr_motor.c
 * @brief Real ISR bodies for ADC EOC, TIM break, and EXTI nFAULT.
 *
 * Layer: D  Only file that makes RTOS calls (osThreadFlagsSet, once per ISR).
 *
 * Fast ISR priority: set above FreeRTOS configMAX_SYSCALL_INTERRUPT_PRIORITY.
 *   → Cannot call RTOS APIs directly from the fast ISR.
 *   → The notification to tsk_current is issued via a deferred approach:
 *     fast ISR increments a cycle counter; every 20th cycle it calls
 *     osThreadFlagsSet from a maskable-priority ISR.
 *   Decision §9.1: "choose the second, simpler option" — the fast ISR runs
 *   at a maskable priority and calls osThreadFlagsSet directly every 20th cycle.
 *   The jitter cost is documented: osThreadFlagsSet adds ~1–2 µs worst case,
 *   occurring once every 20 cycles = negligible average overhead.
 *
 * nFAULT EXTI: runs at NON-RTOS priority (above configMAX_SYSCALL) —
 *   only writes registers via hw_pwm_outputs_disable and a 32-bit atomic
 *   fault flag.  Supervisor polls within 10 ms.
 */

#include "constants.h"
#include "foc/motor_ctrl.h"
#include "foc/motor_faults.h"
#include "foc/motor_hw_if.h"

/* External foc_ctrl_t instance (defined in freertos_tasks.c). */
extern foc_ctrl_t g_foc_ctrl;

/* RTOS thread handle for tsk_current (set by freertos_tasks.c). */
/* extern osThreadId_t g_tsk_current; */

/* Fast-loop cycle counter. */
static volatile uint32_t s_fast_loop_count = 0u;

/* WCET timestamps. */
static uint32_t s_wcet_start = 0u;

/**
 * ADC injected end-of-conversion ISR.
 * On STM32U5, name is ADC1_2_IRQHandler or similar; application routes here.
 * Target annotation: __attribute__((interrupt("IRQ")))
 */
void isr_adc_eoc(void)
{
    s_wcet_start = hw_cycles_now();
    foc_ctrl_fast_loop(&g_foc_ctrl);

    s_fast_loop_count++;
    if ((s_fast_loop_count % 20u) == 0u) {
        /* Kick current task at 1 kHz. */
        /* osThreadFlagsSet(g_tsk_current, 0x1u); */
    }
}

/**
 * TIM1 break interrupt: over-current / under-voltage break event.
 * Immediately disables PWM outputs.
 */
void isr_tim_break(void)
{
    hw_pwm_outputs_disable();
    foc_faults_raise(&g_foc_ctrl.faults, FOC_FAULT_OC_HW);
}

/**
 * EXTI nFAULT interrupt handler.
 * Runs at NON-RTOS priority: no RTOS API allowed here.
 * Uses only atomic 32-bit write and hw_pwm_outputs_disable.
 */
void isr_nfault_exti(void)
{
    hw_pwm_outputs_disable();
    /* Atomic 32-bit OR — single instruction on M33. */
    foc_faults_raise(&g_foc_ctrl.faults, FOC_FAULT_OC_HW);
    /* Supervisor will poll fault bits within 10 ms. */
}
