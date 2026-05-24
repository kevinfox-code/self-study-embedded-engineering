/*
 * Author:      Kevin Fox
 * Book:        Making Embedded Systems: Design Patterns for Great Software
 *              by Elecia White — O'Reilly Media
 * Description: Public interface for the user button driver — defines the BUTTON_State enum and declares init, state query, and interrupt-callback registration functions.
 */

#ifndef __BUTTON_H__
#define __BUTTON_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BUTTON_RELEASED = 0,
    BUTTON_PRESSED  = 1,
} BUTTON_State;

void         BUTTON_Init(void);
BUTTON_State BUTTON_GetState(void);
void         BUTTON_RegisterCallback(void (*cb)(void));

#ifdef __cplusplus
}
#endif

#endif /* __BUTTON_H__ */
