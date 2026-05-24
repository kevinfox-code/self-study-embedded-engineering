/*
 * Author:      Kevin Fox
 * Book:        Bare-Metal Embedded C Programming
 *              by Israel Gbati — Packt, 2024
 * Description: LED blink application used to validate the custom Makefile build system. Identical register-level PB7 toggle loop to Week 4, with a slower delay count to confirm the Makefile correctly compiles and links a multi-file project.
 */
#include <stdint.h>

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
#define PB7_OUTPUT_MASK			(3U << 14)
#define PIN7_ON 				(1U << 7)

void SystemInit(void) {}

int main(void)
{
	RCC_AHB2EN_R |= GPIOB_EN;
	GPIOB_MODE_R &= ~PB7_OUTPUT_MASK;
	GPIOB_MODE_R |= PB7_OUTPUT_MODE;
	GPIOB_OD_R |= PIN7_ON;

	while (1) {
    GPIOB_OD_R ^= PIN7_ON;
    for (volatile int i = 0; i < 1000000; i++) {}
  }
}

void Error_Handler(void) {}
