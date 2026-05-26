/*
    * adc.c
    *
    *  Created on: May 26, 2026
    AHB2 - ADC12 (ADC1_IN8 PA3 - AO on Zio connector) clock enable bit 0 for GPIOA
*/
#include "adc.h"

#define GPIOA_EN      (1U << 0)  // AHB2ENR1 bit 0: GPIOA clock
#define ADC12_EN      (1U << 10) // AHB2ENR1 bit 10: ADC12 clock
#define ADC_CHANNEL_8 (8U)       // ADC1_IN8 = PA3


void ADC_Init(void) {
    // Enable clocks for GPIOA and ADC12
    RCC_NS->AHB2ENR1 |= GPIOA_EN | ADC12_EN;

    // Set PA3 as analog mode (MODER3 = 11)
    GPIOA_NS->MODER |= (3U << 6);

    // Select HCLK as ADC/DAC kernel clock (CCIPR3 ADCDACSEL = 000 = bits[14:12]).
    // HCLK is always running and has no separate kernel-clock enable, making it
    // the safest choice. STM32U575 ADC has no CKMODE field — all clocking is
    // asynchronous via CCIPR3, so HCLK here is still the async kernel source.
    RCC_NS->CCIPR3 &= ~(7U << 12);

    // STM32U575 ADC resets into deep power down (DEEPPWD=1 in CR at reset).
    // Must exit DEEPPWD and enable internal voltage regulator before calibration.
    ADC1_NS->CR &= ~(1U << 29); // Clear DEEPPWD
    ADC1_NS->CR |= (1U << 28);  // Set ADVREGEN (voltage regulator enable)

    // Wait for voltage regulator startup (tADCVREG_STUP = 20 us; 16 MHz -> 320 cycles)
    for (volatile uint32_t i = 0; i < 500; i++);

    // CONT must be written while ADEN=0
    ADC1_NS->CFGR1 |= (1U << 13); // CONT: continuous conversion mode

    // Run self-calibration (requires ADEN=0, ADSTART=0, DEEPPWD=0, ADVREGEN=1)
    ADC1_NS->CR |= (1U << 31);        // Set ADCAL
    while (ADC1_NS->CR & (1U << 31)); // Wait for calibration to finish

    // STM32U5-specific: preselect channel 8 in PCSEL before configuring SQR1.
    // Without this the analog mux is not connected and conversions never complete.
    ADC1_NS->PCSEL = (1U << ADC_CHANNEL_8); // Preselect IN8 (PA3)

    // Configure sequence: channel 8 as first (and only) conversion
    ADC1_NS->SQR1 = (ADC_CHANNEL_8 << 6); // L=0 (1 conversion), SQ1=8

    // Enable ADC and wait until it signals ready
    ADC1_NS->CR |= (1U << 0);             // Set ADEN
    while (!(ADC1_NS->ISR & (1U << 0))); // Wait for ADRDY
}

void Start_Conversion(void) {
    ADC1_NS->CR |= (1U << 2); // Set ADSTART — begins continuous conversions
}

uint32_t ADC_Read(void) {
    while (!(ADC1_NS->ISR & (1U << 2))); // Wait for EOC
    return ADC1_NS->DR;
}
