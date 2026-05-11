#include "led.h"
#include "main.h"

typedef struct {
    GPIO_TypeDef *port;
    uint16_t      pin;
} LED_Descriptor;

static const LED_Descriptor leds[] = {
    [LED_RED]   = { USER_RED_GPIO_Port,   USER_RED_Pin   },
    [LED_GREEN] = { USER_GREEN_GPIO_Port, USER_GREEN_Pin },
    [LED_BLUE]  = { USER_BLUE_GPIO_Port,  USER_BLUE_Pin  },
};

void LED_On(LED_Color color)
{
    HAL_GPIO_WritePin(leds[color].port, leds[color].pin, GPIO_PIN_SET);
}

void LED_Off(LED_Color color)
{
    HAL_GPIO_WritePin(leds[color].port, leds[color].pin, GPIO_PIN_RESET);
}

void LED_Toggle(LED_Color color)
{
    HAL_GPIO_TogglePin(leds[color].port, leds[color].pin);
}

void LED_SetAll(uint8_t red, uint8_t green, uint8_t blue)
{
    HAL_GPIO_WritePin(USER_RED_GPIO_Port,   USER_RED_Pin,   red   ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(USER_GREEN_GPIO_Port, USER_GREEN_Pin, green ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(USER_BLUE_GPIO_Port,  USER_BLUE_Pin,  blue  ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
