#include "scheduler.h"
#include "pubsub.h"

/*
 * Marked volatile because SCHEDULER_Tick() is designed to be called from
 * an ISR while SCHEDULER_GetTickMs() may be called from the main loop.
 */
static volatile uint32_t s_tick_ms = 0u;

void SCHEDULER_Init(void)
{
    s_tick_ms = 0u;
}

/*
 * Call from a 1 ms hardware timer ISR.
 * Publishes the current tick count to all relevant topics.
 */
void SCHEDULER_Tick(void)
{
    s_tick_ms++;

    PUBSUB_Data data;
    data.tick_ms = s_tick_ms;

    PUBSUB_Publish(TOPIC_TICK_1MS, data);

    if (s_tick_ms % 100u == 0u) {
        PUBSUB_Publish(TOPIC_TICK_100MS, data);
    }
    if (s_tick_ms % 1000u == 0u) {
        PUBSUB_Publish(TOPIC_TICK_1S, data);
    }
}

uint32_t SCHEDULER_GetTickMs(void)
{
    return s_tick_ms;
}
