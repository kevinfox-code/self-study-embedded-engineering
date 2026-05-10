#ifndef __PUBSUB_H__
#define __PUBSUB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Maximum number of handlers that can subscribe to a single topic. */
#define PUBSUB_MAX_SUBSCRIBERS  8U

/*
 * Topics the system can publish.  Add new topics before PUBSUB_TOPIC_COUNT;
 * that sentinel is used to size the internal subscriber table.
 */
typedef enum {
    TOPIC_TICK_1MS    = 0,
    TOPIC_TICK_100MS  = 1,
    TOPIC_TICK_1S     = 2,
    TOPIC_BUTTON      = 3,
    TOPIC_TEMPERATURE = 4,
    PUBSUB_TOPIC_COUNT
} PUBSUB_Topic;

/* Per-topic payload.  Only the field matching the topic is meaningful. */
typedef union {
    uint32_t tick_ms;               /* TOPIC_TICK_* : milliseconds since init  */
    uint8_t  button_state;          /* TOPIC_BUTTON : 0 = released, 1 = pressed */
    int16_t  temperature_decidegc;  /* TOPIC_TEMPERATURE : tenths of °C         */
} PUBSUB_Data;

/* The message delivered to every subscriber when a topic fires. */
typedef struct {
    PUBSUB_Topic topic;
    PUBSUB_Data  data;
} PUBSUB_Message;

typedef void (*PUBSUB_Handler)(const PUBSUB_Message *msg);

typedef enum {
    PUBSUB_OK          =  0,
    PUBSUB_ERR_FULL    = -1,  /* Topic's subscriber table is full          */
    PUBSUB_ERR_INVALID = -2,  /* NULL handler, out-of-range topic, or      */
                              /* Unsubscribe called for a non-subscriber    */
} PUBSUB_Status;

void          PUBSUB_Init(void);
PUBSUB_Status PUBSUB_Subscribe(PUBSUB_Topic topic, PUBSUB_Handler handler);
PUBSUB_Status PUBSUB_Unsubscribe(PUBSUB_Topic topic, PUBSUB_Handler handler);
void          PUBSUB_Publish(PUBSUB_Topic topic, PUBSUB_Data data);
uint8_t       PUBSUB_GetSubscriberCount(PUBSUB_Topic topic);

#ifdef __cplusplus
}
#endif

#endif /* __PUBSUB_H__ */
