/*
 * Author:      Kevin Fox
 * Book:        Making Embedded Systems: Design Patterns for Great Software
 *              by Elecia White — O'Reilly Media
 * Description: Application entry point for Week 5 — wires the scheduler, sensor, and LED modules into the pub/sub bus, demonstrating interrupt-driven multi-rate task dispatch without a traditional RTOS.
 */

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Publish/Subscribe pattern demonstration on STM32U5
  *
  * Three topics are in play:
  *   TOPIC_TICK_100MS  -> samples the temperature sensor, blinks green LED
  *   TOPIC_TICK_1S     -> toggles blue LED (1 Hz heartbeat)
  *   TOPIC_TEMPERATURE -> logs the latest reading (placeholder)
  *
  * The scheduler is the sole publisher of tick topics.  It is driven by the
  * 1 ms TIM17 interrupt that also feeds HAL_IncTick().  The publisher knows
  * nothing about its subscribers; it simply fires the topic at the right time.
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "icache.h"
#include "tim.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "pubsub.h"
#include "scheduler.h"
#include "sensor.h"
/* USER CODE END Includes */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void SystemPower_Config(void);

/* USER CODE BEGIN PFP */
static void OnTick100Ms(const PUBSUB_Message *msg);
static void OnTick1S(const PUBSUB_Message *msg);
static void OnTemperature(const PUBSUB_Message *msg);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* ----- Subscribers -------------------------------------------------------- */

static void OnTick100Ms(const PUBSUB_Message *msg)
{
    (void)msg;
    SENSOR_Update();
    HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
}

static void OnTick1S(const PUBSUB_Message *msg)
{
    (void)msg;
    HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
}

static void OnTemperature(const PUBSUB_Message *msg)
{
    /* Replace with UART printf or display update on real hardware. */
    int16_t temp = msg->data.temperature_decidegc;
    (void)temp;
}

/* USER CODE END 0 */

int main(void)
{
    /* USER CODE BEGIN 1 */
    /* USER CODE END 1 */

    HAL_Init();
    SystemPower_Config();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_ICACHE_Init();
    MX_TIM1_Init();

    /* USER CODE BEGIN 2 */
    PUBSUB_Init();
    SCHEDULER_Init();
    SENSOR_Init();

    PUBSUB_Subscribe(TOPIC_TICK_100MS,  OnTick100Ms);
    PUBSUB_Subscribe(TOPIC_TICK_1S,     OnTick1S);
    PUBSUB_Subscribe(TOPIC_TEMPERATURE, OnTemperature);
    /* USER CODE END 2 */

    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* All work is event-driven through the pub/sub system.
         * The main loop stays empty; add low-priority background tasks here
         * if needed (e.g. a command-line shell, power management). */
    }
    /* USER CODE END WHILE */
}

/* USER CODE BEGIN 4 */

/*
 * HAL_TIM_PeriodElapsedCallback is called from TIM17's ISR every 1 ms.
 * We feed both HAL (for HAL_Delay / uwTick) and our scheduler here.
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM17)
    {
        HAL_IncTick();
        SCHEDULER_Tick();
    }
}

/* USER CODE END 4 */

void SystemClock_Config(void)
{
    uint8_t use_hsi_fallback = 0U;
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMBOOST  = RCC_PLLMBOOST_DIV1;
    RCC_OscInitStruct.PLL.PLLM       = 1;
    RCC_OscInitStruct.PLL.PLLN       = 10;
    RCC_OscInitStruct.PLL.PLLP       = 2;
    RCC_OscInitStruct.PLL.PLLQ       = 2;
    RCC_OscInitStruct.PLL.PLLR       = 1;
    RCC_OscInitStruct.PLL.PLLRGE     = RCC_PLLVCIRANGE_1;
    RCC_OscInitStruct.PLL.PLLFRACN   = 0;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        use_hsi_fallback = 1U;
    }

    if (use_hsi_fallback == 0U)
    {
        RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                         | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2
                                         | RCC_CLOCKTYPE_PCLK3;
        RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
        RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
        RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
        RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
        RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;
        if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
        {
            use_hsi_fallback = 1U;
        }
    }

    if (use_hsi_fallback != 0U)
    {
        RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
        RCC_OscInitStruct.HSEState       = RCC_HSE_OFF;
        RCC_OscInitStruct.HSIState       = RCC_HSI_ON;
        RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_OFF;
        if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
        {
            Error_Handler();
        }
        RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                         | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2
                                         | RCC_CLOCKTYPE_PCLK3;
        RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_HSI;
        RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
        RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
        RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
        RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;
        if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
        {
            Error_Handler();
        }
    }
}

static void SystemPower_Config(void)
{
    HAL_PWREx_EnableVddIO2();
    HAL_PWREx_DisableUCPDDeadBattery();
    if (HAL_PWREx_ConfigSupply(PWR_SMPS_SUPPLY) != HAL_OK)
    {
        Error_Handler();
    }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
}
#endif
