/**
 * @file board_support.c
 * @brief ALL HAL calls for the motor control library live here.
 *        No other library file may call HAL functions.
 *
 * Layer: C  Includes constants.h.
 *
 * Target: STM32U5 with CubeMX-generated peripheral init.
 * The file compiles only on target (not in the host build).
 */

#include "constants.h"
#include "board_support.h"

/*
 * On target, include the CubeMX-generated headers:
 *   #include "main.h"
 *   #include "stm32u5xx_hal.h"
 *
 * These are left as comments for the host build stub.
 * See examples/stm32u5_nucleo for the wiring pattern.
 */

/* -------------------------------------------------------------------------
 * nFAULT callback registered at init
 * ---------------------------------------------------------------------- */
static void (*s_nfault_cb)(void) = NULL;

void bsp_init(void (*nfault_cb)(void))
{
    s_nfault_cb = nfault_cb;
    /*
     * Target implementation:
     *   CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
     *   DWT->CYCCNT       = 0u;
     *   DWT->CTRL        |= DWT_CTRL_CYCCNTENA_Msk;
     *   // Register nFAULT EXTI callback via HAL_GPIO_EXTI_Callback
     */
}

/* -------------------------------------------------------------------------
 * PWM
 * ---------------------------------------------------------------------- */
void bsp_pwm_set_compare(uint16_t ca, uint16_t cb, uint16_t cc)
{
    /*
     * Target:
     *   BSP_TIM_PWM_HANDLE->Instance->CCR1 = ca;
     *   BSP_TIM_PWM_HANDLE->Instance->CCR2 = cb;
     *   BSP_TIM_PWM_HANDLE->Instance->CCR3 = cc;
     */
    (void)ca; (void)cb; (void)cc;
}

uint16_t bsp_pwm_period_ticks(void)
{
    return BSP_PWM_PERIOD_TICKS;
}

void bsp_pwm_outputs_enable(void)
{
    /*
     * Target:
     *   __HAL_TIM_MOE_ENABLE(BSP_TIM_PWM_HANDLE);
     */
}

void bsp_pwm_outputs_disable(void)
{
    /*
     * Target (break/MOE clear, single instruction on Cortex-M):
     *   BSP_TIM_PWM_HANDLE->Instance->BDTR &= ~TIM_BDTR_MOE;
     */
}

/* -------------------------------------------------------------------------
 * ADC
 * ---------------------------------------------------------------------- */
void bsp_adc_read_injected(uint16_t results[5])
{
    /*
     * Target:
     *   results[0] = (uint16_t)HAL_ADCEx_InjectedGetValue(BSP_ADC_HANDLE, BSP_ADC_RANK_IA);
     *   results[1] = (uint16_t)HAL_ADCEx_InjectedGetValue(BSP_ADC_HANDLE, BSP_ADC_RANK_IB);
     *   results[2] = (uint16_t)HAL_ADCEx_InjectedGetValue(BSP_ADC_HANDLE, BSP_ADC_RANK_IC);
     *   results[3] = (uint16_t)HAL_ADCEx_InjectedGetValue(BSP_ADC_HANDLE, BSP_ADC_RANK_VBUS);
     *   results[4] = 0u;  // temperature — add ADC channel if needed
     */
    results[0] = results[1] = results[2] = 2048u;
    results[3] = (uint16_t)(24u * BSP_ADC_FULLSCALE / 60u);
    results[4] = 0u;
}

/* -------------------------------------------------------------------------
 * Timing
 * ---------------------------------------------------------------------- */
uint32_t bsp_cycles_now(void)
{
    /*
     * Target:
     *   return DWT->CYCCNT;
     */
    static uint32_t s_cnt = 0u;
    s_cnt += 100u;
    return s_cnt;
}

void bsp_delay_us(uint32_t us)
{
    /*
     * Target: DWT-based busy loop.
     *   uint32_t start = DWT->CYCCNT;
     *   uint32_t ticks = us * (BSP_CPU_FREQ_HZ / 1000000u);
     *   while ((DWT->CYCCNT - start) < ticks);
     */
    (void)us;
}

/* -------------------------------------------------------------------------
 * GPIO
 * ---------------------------------------------------------------------- */
void bsp_gpio_drv_enable(bool on)
{
    /*
     * Target:
     *   HAL_GPIO_WritePin(BSP_GPIO_DRV_EN_PORT, BSP_GPIO_DRV_EN_PIN,
     *                     on ? GPIO_PIN_SET : GPIO_PIN_RESET);
     */
    (void)on;
}

bool bsp_gpio_drv_nfault(void)
{
    /*
     * Target:
     *   return (HAL_GPIO_ReadPin(BSP_GPIO_NFAULT_PORT, BSP_GPIO_NFAULT_PIN) == GPIO_PIN_SET);
     */
    return true; /* No fault in stub. */
}

/* -------------------------------------------------------------------------
 * SPI
 * ---------------------------------------------------------------------- */
int bsp_spi_xfer16(uint16_t tx, uint16_t *rx)
{
    /*
     * Target:
     *   uint16_t buf = tx;
     *   HAL_GPIO_WritePin(BSP_GPIO_DRV_CS_PORT, BSP_GPIO_DRV_CS_PIN, GPIO_PIN_RESET);
     *   HAL_StatusTypeDef st = HAL_SPI_TransmitReceive(BSP_SPI_DRV_HANDLE,
     *       (uint8_t*)&buf, (uint8_t*)rx, 1, 10);
     *   HAL_GPIO_WritePin(BSP_GPIO_DRV_CS_PORT, BSP_GPIO_DRV_CS_PIN, GPIO_PIN_SET);
     *   return (st == HAL_OK) ? 0 : -1;
     */
    if (rx) *rx = 0u;
    (void)tx;
    return 0;
}

/* -------------------------------------------------------------------------
 * Current offset calibration
 * ---------------------------------------------------------------------- */
void bsp_measure_current_offsets(uint32_t n_samples, int32_t offsets[3])
{
    /*
     * Target: with PWM outputs disabled and CSA_CAL asserted,
     * average n_samples readings from injected ADC.
     */
    offsets[0] = offsets[1] = offsets[2] = (int32_t)(BSP_ADC_FULLSCALE / 2);
    (void)n_samples;
}

/* -------------------------------------------------------------------------
 * nFAULT EXTI interrupt handler (called by HAL_GPIO_EXTI_Callback in app)
 * ---------------------------------------------------------------------- */
void bsp_nfault_exti_handler(void)
{
    if (s_nfault_cb != NULL) {
        s_nfault_cb();
    }
}
