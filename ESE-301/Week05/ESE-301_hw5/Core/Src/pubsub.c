#include "pubsub.h"
#include <string.h>

static PUBSUB_Handler s_handlers[PUBSUB_TOPIC_COUNT][PUBSUB_MAX_SUBSCRIBERS];
static uint8_t        s_counts[PUBSUB_TOPIC_COUNT];

void PUBSUB_Init(void)
{
    memset(s_handlers, 0, sizeof(s_handlers));
    memset(s_counts,   0, sizeof(s_counts));
}

PUBSUB_Status PUBSUB_Subscribe(PUBSUB_Topic topic, PUBSUB_Handler handler)
{
    if (topic >= PUBSUB_TOPIC_COUNT || handler == NULL) {
        return PUBSUB_ERR_INVALID;
    }
    if (s_counts[topic] >= PUBSUB_MAX_SUBSCRIBERS) {
        return PUBSUB_ERR_FULL;
    }
    s_handlers[topic][s_counts[topic]++] = handler;
    return PUBSUB_OK;
}

PUBSUB_Status PUBSUB_Unsubscribe(PUBSUB_Topic topic, PUBSUB_Handler handler)
{
    if (topic >= PUBSUB_TOPIC_COUNT || handler == NULL) {
        return PUBSUB_ERR_INVALID;
    }
    uint8_t n = s_counts[topic];
    for (uint8_t i = 0; i < n; i++) {
        if (s_handlers[topic][i] == handler) {
            /* Shift remaining entries down to close the gap. */
            for (uint8_t j = i; j < (uint8_t)(n - 1u); j++) {
                s_handlers[topic][j] = s_handlers[topic][j + 1u];
            }
            s_handlers[topic][n - 1u] = NULL;
            s_counts[topic]--;
            return PUBSUB_OK;
        }
    }
    return PUBSUB_ERR_INVALID;
}

void PUBSUB_Publish(PUBSUB_Topic topic, PUBSUB_Data data)
{
    if (topic >= PUBSUB_TOPIC_COUNT) {
        return;
    }
    PUBSUB_Message msg;
    msg.topic = topic;
    msg.data  = data;

    uint8_t n = s_counts[topic];
    for (uint8_t i = 0; i < n; i++) {
        s_handlers[topic][i](&msg);
    }
}

uint8_t PUBSUB_GetSubscriberCount(PUBSUB_Topic topic)
{
    if (topic >= PUBSUB_TOPIC_COUNT) {
        return 0u;
    }
    return s_counts[topic];
}
