/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
//#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// Target is PB7 which is connected to a blue LED
#define PERIPH_BASE_NS 			(0X40000000UL)
#define AHB2PERIPH_OFFSET_NS 	(0x02020000UL)
#define AHB3PERIPH_OFFSET_NS 	(0x06020000UL)
#define AHB2PERIPH_BASE_NS 		(PERIPH_BASE_NS + AHB2PERIPH_OFFSET_NS)
#define AHB3PERIPH_BASE_NS 		(PERIPH_BASE_NS + AHB3PERIPH_OFFSET_NS)
#define GPIOB_OFFSET 			(0x00000400UL)
#define GPIOB_BASE 				(AHB2PERIPH_BASE_NS + GPIOB_OFFSET)
#define RCC_OFFSET 				(0x00000C00UL)
#define RCC_BASE 				(AHB3PERIPH_BASE_NS + RCC_OFFSET)
#define AHB2EN1_R_OFFSET 		(0x0000008CUL)
#define RCC_AHB2EN_R			(*(volatile unsigned int *)(RCC_BASE + AHB2EN1_R_OFFSET))
#define MODE_R_OFFSET 			(0x00000000UL)
#define GPIOB_MODE_R 			(*(volatile unsigned int *)(GPIOB_BASE + MODE_R_OFFSET))
#define OD_R_OFFSET 			(0x00000014UL)
#define GPIOB_OD_R 				(*(volatile unsigned int *)(GPIOB_BASE + OD_R_OFFSET))

#define GPIOB_EN 				(1U << 1)
#define PB7_OUTPUT_MODE			(1U << 14)
#define PB7_OUTPUT_MASK  		(3U << 14)
#define PIN7_ON 				(1U << 7)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
//void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */


  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
//  HAL_Init();

  /* USER CODE BEGIN Init */
	// Enable clock data to GPIOB
	RCC_AHB2EN_R |= GPIOB_EN;

	GPIOB_MODE_R &= ~PB7_OUTPUT_MASK;
	GPIOB_MODE_R |= PB7_OUTPUT_MODE;

	GPIOB_OD_R |= PIN7_ON;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
//  HAL_Init();

  /* USER CODE BEGIN Init */
	// Enable clock data to GPIOB
	RCC_AHB2EN_R |= GPIOB_EN;

	GPIOB_MODE_R &= ~PB7_OUTPUT_MASK;
	GPIOB_MODE_R |= PB7_OUTPUT_MODE;

	GPIOB_OD_R |= PIN7_ON;
  /* USER CODE END 1 */

  /* USER CODE END Init */

//  /* Configure the system clock */
//  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
//void SystemClock_Config(void)
//{
//  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
//  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
//
//  /** Configure the main internal regulator output voltage
//  */
//  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE4) != HAL_OK)
//  {
//    Error_Handler();
//  }
//
//  /** Initializes the CPU, AHB and APB buses clocks
//  */
//  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
//  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
//  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
//  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_4;
//  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
//  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
//  {
//    Error_Handler();
//  }
//
//  /** Initializes the CPU, AHB and APB buses clocks
//  */
//  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
//                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
//                              |RCC_CLOCKTYPE_PCLK3;
//  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
//  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
//  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
//  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
//  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;
//
//  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
//  {
//    Error_Handler();
//  }
//}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
//  /* User can add his own implementation to report the HAL error return state */
//  __disable_irq();
//  while (1)
//  {
//  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
