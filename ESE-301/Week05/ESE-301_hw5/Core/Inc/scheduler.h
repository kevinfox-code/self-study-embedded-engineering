/*
 * Author:      Kevin Fox
 * Book:        Making Embedded Systems: Design Patterns for Great Software
 *              by Elecia White — O'Reilly Media
 * Description: Declares the time-driven event publisher — SCHEDULER_Tick() is called from a 1 ms ISR and fires TOPIC_TICK_1MS, TOPIC_TICK_100MS, and TOPIC_TICK_1S pub/sub events at the appropriate intervals.
 */

#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
 * Time-driven publisher for the pub/sub pattern.
 *
 * Call SCHEDULER_Tick() from a 1 ms hardware timer ISR.  Each call
 * publishes TOPIC_TICK_1MS unconditionally, TOPIC_TICK_100MS every
 * 100 calls, and TOPIC_TICK_1S every 1000 calls.
 */
void     SCHEDULER_Init(void);
void     SCHEDULER_Tick(void);
uint32_t SCHEDULER_GetTickMs(void);

#ifdef __cplusplus
}
#endif

#endif /* __SCHEDULER_H__ */
