/*
 * Author:      Kevin Fox
 * Description: ADC driver header for STM32U575 — declares init, start conversion, and read
 *              functions for ADC1_IN8 single-channel polling conversion.
 */

#ifndef ADC_H
#define ADC_H

#include "stm32u575xx.h"

// Function prototypes
void     ADC_Init(void);
void     Start_Conversion(void);
uint32_t ADC_Read(void);

#endif // ADC_H