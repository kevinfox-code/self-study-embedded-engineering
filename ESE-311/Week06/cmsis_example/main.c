#include <stdint.h>
#include "stm32u575xx.h"

#define GPIOB_EN 	(1U << 1)
#define PIN7 		(1U << 7)
#define LED_PIN 	(PIN7)

void SystemInit(void) {}

int main(void)
{
	RCC_NS->AHB2ENR1 |= GPIOB_EN;	//	Enable clock for GPIOB

	GPIOB_NS->MODER &= ~(1U << 15);	//	Clear bit 15 to set PB7 as output 
	GPIOB_NS->MODER |= (1U << 14);	//	Set bit 14 to set PB7 as output
	GPIOB_NS->ODR |= LED_PIN;

	while (1) {
    GPIOB_NS->ODR ^= LED_PIN;
    for (volatile int i = 0; i < 1000000; i++) {}
  }
}

void Error_Handler(void) {}
