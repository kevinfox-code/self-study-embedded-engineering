#include "gpio.h"

// PC7 - Green LED
#define GPIOC_EN 	(1U << 2)
#define GREEN_LED_SET 	(1U << 7)
#define GREEN_LED_RESET (1U << (23))

// PB7 - Blue LED
#define GPIOB_EN 	(1U << 1)
#define BLUE_LED_SET 	(1U << 7)
#define BLUE_LED_RESET 	(1U << (23))

// PG2 - Red LED
#define GPIOG_EN 	(1U << 6)
#define RED_LED_SET 	(1U << 2)
#define RED_LED_RESET 	(1U << (18))

// PC13 - User Button
#define USER_BUTTON_PIN (1U << (13))

void led_init(void) {
    /*Enable clock to GPIOB */
    RCC_NS->AHB2ENR1 |= GPIOB_EN;
    /*Set PB7 as output */
	GPIOB_NS->MODER &= ~(1U << 15);	//	Clear bit 15 to set PB7 as output 
	GPIOB_NS->MODER |= (1U << 14);	//	Set bit 14 to set PB7 as output
	GPIOB_NS->OTYPER &= ~(1U << 7);	//	Push-pull output
	GPIOB_NS->OSPEEDR |= (1U << 14);	//	Medium speed
	GPIOB_NS->PUPDR &= ~(3U << 14);	//	No pull-up/pull-down

    /*Enable clock to GPIOC */
    RCC_NS->AHB2ENR1 |= GPIOC_EN;
    /*Set PC7 as output */
    GPIOC_NS->MODER &= ~(1U << 15);	//	Clear bit 15 to set PC7 as output
    GPIOC_NS->MODER |= (1U << 14);	//	Set bit 14 to set PC7 as output
    GPIOC_NS->OTYPER &= ~(1U << 7);	//	Push-pull output
    GPIOC_NS->OSPEEDR |= (1U << 14);	//	Medium speed
    GPIOC_NS->PUPDR &= ~(3U << 14);	//	No pull-up/pull-down

    /*Enable clock to GPIOG */
    RCC_NS->AHB2ENR1 |= GPIOG_EN;
    /*Set PG2 as output */
    GPIOG_NS->MODER &= ~(1U << 5);	//	Clear bit 5 to set PG2 as output
    GPIOG_NS->MODER |= (1U << 4);	//	Set bit 4 to set PG2 as output
    GPIOG_NS->OTYPER &= ~(1U << 2);	//	Push-pull output
    GPIOG_NS->OSPEEDR &= ~(3U << 4);	//	Clear bits [5:4]
    GPIOG_NS->OSPEEDR |= (1U << 4);	//	Set to medium speed (01)
    GPIOG_NS->PUPDR &= ~(3U << 4);	//	No pull-up/pull-down
}

//  PC13 - User Button ACTIVE HIGH
void button_init(void) {
    /*Enable clock to GPIOC */
    RCC_NS->AHB2ENR1 |= GPIOC_EN;
    /*Set PC13 as input */
    GPIOC_NS->MODER &= ~(3U << 26);	//	Clear bits [27:26] to set PC13 as input
    GPIOC_NS->PUPDR |= (1U << 27);	//	Pull-down resistor
    GPIOC_NS->PUPDR &= ~(1U << 26);	//	Clear bit 26 to set pull-down resistor
}

void blue_led_on(void) {
    GPIOB_NS->BSRR = BLUE_LED_SET;
}

void blue_led_off(void) {
    GPIOB_NS->BSRR = BLUE_LED_RESET;
}

void green_led_on(void) {
    GPIOC_NS->BSRR = GREEN_LED_SET;
}

void green_led_off(void) {
    GPIOC_NS->BSRR = GREEN_LED_RESET;
}

void red_led_on(void) {
    GPIOG_NS->BSRR = RED_LED_SET;
}

void red_led_off(void) {
    GPIOG_NS->BSRR = RED_LED_RESET;
}

bool get_button_state(void) {
    return (GPIOC_NS->IDR & USER_BUTTON_PIN) != 0;
}
