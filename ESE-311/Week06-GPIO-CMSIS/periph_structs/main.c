/*
 * Author:      Kevin Fox
 * Book:        Bare-Metal Embedded C Programming
 *              by Israel Gbati — Packt, 2024
 * Description: Demonstrates writing custom GPIO and RCC peripheral structs from scratch (matching STM32U575 reference-manual offsets) as an alternative to CMSIS headers, then uses them to blink PB7 via struct member access.
 */
#include <stdint.h>

// #define PERIPH_BASE_NS 			(0X40000000UL)
// #define AHB2PERIPH_OFFSET_NS 	(0x02020000UL)
// #define AHB3PERIPH_OFFSET_NS 	(0x06020000UL)
// #define AHB2PERIPH_BASE_NS 		(PERIPH_BASE_NS + AHB2PERIPH_OFFSET_NS)
// #define AHB3PERIPH_BASE_NS 		(PERIPH_BASE_NS + AHB3PERIPH_OFFSET_NS)
// #define GPIOB_OFFSET 			(0x00000400UL)
// #define GPIOB_BASE 				(AHB2PERIPH_BASE_NS + GPIOB_OFFSET)
// #define RCC_OFFSET 				(0x00000C00UL)
// #define RCC_BASE 				(AHB3PERIPH_BASE_NS + RCC_OFFSET)
// #define AHB2EN1_R_OFFSET 		(0x0000008CUL)
// #define RCC_AHB2EN_R			(*(volatile unsigned int *)(RCC_BASE + AHB2EN1_R_OFFSET))
// #define MODE_R_OFFSET 			(0x00000000UL)
// #define GPIOB_MODE_R 			(*(volatile unsigned int *)(GPIOB_BASE + MODE_R_OFFSET))
// #define OD_R_OFFSET 			(0x00000014UL)
// #define GPIOB_OD_R 				(*(volatile unsigned int *)(GPIOB_BASE + OD_R_OFFSET))


// #define PB7_OUTPUT_MODE			(1U << 14)
// #define PB7_OUTPUT_MASK			(3U << 14)


typedef struct {
	volatile uint32_t MODE;			// Offset 0x00
	volatile uint32_t OTYPER;		// Offset 0x04
	volatile uint32_t OSPEEDR;		// Offset 0x08
	volatile uint32_t PUPDR;		// Offset 0x0C
	volatile uint32_t IDR;			// Offset 0x10
	volatile uint32_t ODR;			// Offset 0x14
	volatile uint32_t BSRR;			// Offset 0x18
	volatile uint32_t LCKR;			// Offset 0x1C
	volatile uint32_t AFR[2];	// Offset 0x20 (AFR[0]) and 0x24 (AFR[1])
	volatile uint32_t BRR;			// Offset 0x28
} GPIO_TypeDef;

typedef struct {
	volatile uint32_t DUMMY[35];		// Offsets 0x00 to 0x8C
	volatile uint32_t AHB2ENR1;			// Offset 0x8C
} RCC_TypeDef;

#define RCC_BASE 	(0x46020C00UL)
#define GPIOB_BASE 	(0x42020400UL)

#define GPIOB_EN 	(1U << 1)
#define PIN7 		(1U << 7)
#define LED_PIN 	(PIN7)


#define GPIOB 		((GPIO_TypeDef *) GPIOB_BASE)
#define RCC 		((RCC_TypeDef *) RCC_BASE)


void SystemInit(void) {}

int main(void)
{
	RCC->AHB2ENR1 |= GPIOB_EN;
	GPIOB->MODE &= ~(1U << 15);	//	Clear bit 15 to set PB7 as output 
	GPIOB->MODE |= (1U << 14);	//	Set bit 14 to set PB7 as output
	GPIOB->ODR |= LED_PIN;

	while (1) {
    GPIOB->ODR ^= LED_PIN;
    for (volatile int i = 0; i < 1000000; i++) {}
  }
}

void Error_Handler(void) {}
