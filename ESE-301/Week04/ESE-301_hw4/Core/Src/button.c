/*
 * Author:      Kevin Fox
 * Book:        Making Embedded Systems: Design Patterns for Great Software
 *              by Elecia White — O'Reilly Media
 * Description: Implements GPIO input reading and interrupt-driven debounce logic for the user button, with support for registering a callback invoked on button press events.
 */

#include "button.h"
#include "main.h"

static void (*s_callback)(void) = NULL;
static GPIO_PinState s_pressed_level = GPIO_PIN_SET;

void BUTTON_Init(void)
{
    GPIO_InitTypeDef gpio_init = {0};
    GPIO_PinState idle_level = HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin);

    /* Detect polarity from the idle state and arm interrupt for the press edge. */
    s_pressed_level = (idle_level == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET;

    gpio_init.Pin = USER_BUTTON_Pin;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Mode = (s_pressed_level == GPIO_PIN_SET) ? GPIO_MODE_IT_RISING : GPIO_MODE_IT_FALLING;
    HAL_GPIO_Init(USER_BUTTON_GPIO_Port, &gpio_init);

    HAL_NVIC_SetPriority(EXTI13_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI13_IRQn);
}

BUTTON_State BUTTON_GetState(void)
{
    return (HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == s_pressed_level)
           ? BUTTON_PRESSED
           : BUTTON_RELEASED;
}

void BUTTON_RegisterCallback(void (*cb)(void))
{
    s_callback = cb;
}

static void BUTTON_OnExtiEvent(uint16_t GPIO_Pin)
{
    if ((GPIO_Pin == USER_BUTTON_Pin) && (s_callback != NULL) && (BUTTON_GetState() == BUTTON_PRESSED))
    {
        s_callback();
    }
}

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
    BUTTON_OnExtiEvent(GPIO_Pin);
}

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
    BUTTON_OnExtiEvent(GPIO_Pin);
}
