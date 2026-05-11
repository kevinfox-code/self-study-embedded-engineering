#ifndef __LED_H__
#define __LED_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
    LED_RED   = 0,
    LED_GREEN = 1,
    LED_BLUE  = 2,
} LED_Color;

void LED_On(LED_Color color);
void LED_Off(LED_Color color);
void LED_Toggle(LED_Color color);
void LED_SetAll(uint8_t red, uint8_t green, uint8_t blue);

#ifdef __cplusplus
}
#endif

#endif /* __LED_H__ */
