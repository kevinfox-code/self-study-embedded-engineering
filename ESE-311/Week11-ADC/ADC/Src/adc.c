/*
    * adc.c
 * Book:        Bare-Metal Embedded C Programming
 *              by Israel Gbati — Packt, 2024
    AHB2 - ADC12 (ADC1_IN8 PA3 - AO on Zio connector) clock enable bit 0 for GPIOA
*/
#include "adc.h"
#include "main.h"

#define GPIOA_EN      (1U << 0)  // AHB2ENR1 bit 0: GPIOA clock
#define ADC12_EN      (1U << 10) // AHB2ENR1 bit 10: ADC12 clock
#define ADC_CHANNEL_8 (8U)       // ADC1_IN8 = PA3
#define ADC_CHANNEL_8_SAMPLE_TIME (ADC_SMPR1_SMP8_2) // 19.5 ADC cycles


static void adc_enable(void)
{
    ADC1_NS->ISR = (1U << 0);          // clear ADRDY
    ADC1_NS->CR |= (1U << 0);          // set ADEN
    for (volatile uint32_t t = 0; (ADC1_NS->ISR & (1U << 0)) == 0U; t++) {
        if (t > 1000000U) {
            Error_Handler();
        }
    }
    ADC1_NS->ISR = (1U << 0);          // ack ADRDY
}


void ADC_Init(void) {
    // 1. Enable bus clocks for GPIOA and ADC12; dummy read ensures propagation
    RCC_NS->AHB2ENR1 |= GPIOA_EN | ADC12_EN;
    (void)RCC_NS->AHB2ENR1;

    // 1b. Enable PWR clock on AHB3 and assert ASV ("VDDA analog supply valid").
    //     Without ASV, the chip leaves the analog isolation switch open and
    //     the ADC has no analog power — ADCAL never completes.
    RCC_NS->AHB3ENR |= RCC_AHB3ENR_PWREN;
    (void)RCC_NS->AHB3ENR;
    PWR_NS->SVMCR |= PWR_SVMCR_ASV;

    // 2. PA3 -> analog mode (MODER3 = 11)
    GPIOA_NS->MODER |= (3U << 6);

    // 3. ADC kernel clock = HSI16 (CCIPR3.ADCDACSEL = 100)
    RCC_NS->CR |= RCC_CR_HSION;
    while (!(RCC_NS->CR & RCC_CR_HSIRDY));
    RCC_NS->CCIPR3 = (RCC_NS->CCIPR3 & ~RCC_CCIPR3_ADCDACSEL) | RCC_CCIPR3_ADCDACSEL_2;

    // 4. ADC clock prescaler /8 -> 16 MHz / 8 = 2 MHz ADC clock
    ADC12_COMMON_NS->CCR = (ADC12_COMMON_NS->CCR & ~ADC_CCR_PRESC) | ADC_CCR_PRESC_2;

    // 5. Exit deep power down + enable ADVREGEN, then wait T_ADCVREG_SETUP (>=20us).
    //    500 iterations of a volatile loop at 16 MHz is ~94us — comfortable margin.
    ADC1_NS->CR &= ~(1U << 29); // Clear DEEPPWD
    ADC1_NS->CR |= (1U << 28);  // Set ADVREGEN
    for (volatile uint32_t i = 0; i < 500; i++);

    // 6. Run offset self-calibration. ADEN must be 0 here (true at reset).
    ADC1_NS->ISR = ADC_ISR_EOCAL;
    ADC1_NS->CR |= (1U << 31);                     // ADCAL = 1
    for (volatile uint32_t t = 0; (ADC1_NS->CR & (1U << 31)) != 0U; t++) {
        if (t > 1000000U) {
            Error_Handler();
        }
    }
    ADC1_NS->ISR = ADC_ISR_EOCAL;

    // 7. Channel / sequence / mode (must be done while ADEN = 0).
    ADC1_NS->PCSEL = (1U << ADC_CHANNEL_8);        // preselect IN8 (PA3)
    ADC1_NS->SMPR1 = (ADC1_NS->SMPR1 & ~ADC_SMPR1_SMP8) | ADC_CHANNEL_8_SAMPLE_TIME;
    ADC1_NS->SQR1  = (ADC_CHANNEL_8 << 6);         // L=0 (1 conv), SQ1=8
    ADC1_NS->CFGR1 |= (1U << 13) | (1U << 12);     // CONT + OVRMOD (latest sample wins)

    // 8. Enable ADC and wait for ADRDY.
    adc_enable();
}

void Start_Conversion(void) {
    ADC1_NS->CR |= (1U << 2); // Set ADSTART — begins continuous conversions
}

uint32_t ADC_Read(void) {
    for (volatile uint32_t timeout = 0; (ADC1_NS->ISR & (1U << 2)) == 0U; timeout++) {
        if (timeout > 1000000U) {
            Error_Handler();
        }
    }
    return ADC1_NS->DR;
}
