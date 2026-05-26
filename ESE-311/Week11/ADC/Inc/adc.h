/*
    * adc.h
    *
    *  Created on: May 26, 2026
    AHB2 - ADC12 (ADC1_IN8 PA3 - AO on Zio connector) clock enable bit 0 for GPIOA
*/

#ifndef ADC_H
#define ADC_H

#include "stm32u575xx.h"

// Function prototypes
void ADC_Init(void);
void Start_Conversion(void);
uint32_t ADC_Read(void);

#endif // ADC_H