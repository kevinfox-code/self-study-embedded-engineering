/*
 * SPDX-License-Identifier: MIT
 * Author:      Kevin Fox
 * Book:        Bare-Metal Embedded C Programming
 *              by Israel Gbati — Packt, 2024
 * Description: ADC1 driver API — ADC_Init, Start_Conversion, and ADC_Read for single-channel
 *              conversions on ADC1_IN8 (PA3).
 */

#ifndef ADC_H
#define ADC_H

#include "stm32u575xx.h"

// Function prototypes
void ADC_Init(void);
void Start_Conversion(void);
uint32_t ADC_Read(void);

#endif // ADC_H