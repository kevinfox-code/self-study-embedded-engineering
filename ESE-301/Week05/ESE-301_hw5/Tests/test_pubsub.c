/*
 * Author:      Kevin Fox
 * Book:        Making Embedded Systems: Design Patterns for Great Software
 *              by Elecia White — O'Reilly Media
 * Description: Unit tests for the pub/sub module — verifies subscribe, unsubscribe, publish dispatch, subscriber count tracking, capacity limits, and error codes using assertion-based checks.
 */

/*
 * Sandbox test suite for the Publish/Subscribe pattern.
 *
 * Pure C (C11), no HAL dependencies.  Compile and run on the host:
 *   make run          (from this directory)
 *   gcc -Wall -Wextra -std=c11 -I../Core/Inc \
 *       test_pubsub.c ../Core/Src/pubsub.c ../Core/Src/scheduler.c \
 *       -o test_pubsub && ./test_pubsub
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "pubsub.h"
#include "scheduler.h"

/* ------------------------------------------------------------------ */
/* Minimal test framework                                              */
/* ------------------------------------------------------------------ */

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, name)                                               \
    do {                                                                \
        if (cond) {                                                     \
            printf("PASS  %s\n", (name));                              \
            g_pass++;                                                   \
        } else {                                                        \
            printf("FAIL  %s  (line %d)\n", (name), __LINE__);        \
            g_fail++;                                                   \
        }                                                               \
    } while (0)

/* ------------------------------------------------------------------ */
/* Shared handler state (reset before each test)                      */
/* ------------------------------------------------------------------ */

static int            g_calls_a   = 0;
static int            g_calls_b   = 0;
static int            g_calls_c   = 0;
static PUBSUB_Message g_last_a;
static PUBSUB_Message g_last_b;

static void HandlerA(const PUBSUB_Message *msg) { g_calls_a++; g_last_a = *msg; }
static void HandlerB(const PUBSUB_Message *msg) { g_calls_b++; g_last_b = *msg; }
static void HandlerC(const PUBSUB_Message *msg) { g_calls_c++; (void)msg; }

/* Extra handlers for subscriber-limit test */
static void HandlerD(const PUBSUB_Message *msg) { (void)msg; }
static void HandlerE(const PUBSUB_Message *msg) { (void)msg; }
static void HandlerF(const PUBSUB_Message *msg) { (void)msg; }
static void HandlerG(const PUBSUB_Message *msg) { (void)msg; }
static void HandlerH(const PUBSUB_Message *msg) { (void)msg; }

static void reset_state(void)
{
    g_calls_a = 0;
    g_calls_b = 0;
    g_calls_c = 0;
    memset(&g_last_a, 0, sizeof(g_last_a));
    memset(&g_last_b, 0, sizeof(g_last_b));
    PUBSUB_Init();
    SCHEDULER_Init();
}

/* ------------------------------------------------------------------ */
/* pubsub tests                                                        */
/* ------------------------------------------------------------------ */

static int test_subscribe_and_publish(void)
{
    reset_state();

    PUBSUB_Data data;
    data.tick_ms = 42u;

    PUBSUB_Status s = PUBSUB_Subscribe(TOPIC_TICK_1MS, HandlerA);
    CHECK(s == PUBSUB_OK, "subscribe returns OK");
    CHECK(PUBSUB_GetSubscriberCount(TOPIC_TICK_1MS) == 1u, "subscriber count is 1");

    PUBSUB_Publish(TOPIC_TICK_1MS, data);
    CHECK(g_calls_a == 1, "handler called once after one publish");
    CHECK(g_last_a.topic == TOPIC_TICK_1MS, "message carries correct topic");
    CHECK(g_last_a.data.tick_ms == 42u, "message carries correct data");

    PUBSUB_Publish(TOPIC_TICK_1MS, data);
    CHECK(g_calls_a == 2, "handler called again on second publish");

    return 1;
}

static int test_multiple_subscribers_same_topic(void)
{
    reset_state();

    PUBSUB_Subscribe(TOPIC_TICK_1S, HandlerA);
    PUBSUB_Subscribe(TOPIC_TICK_1S, HandlerB);
    PUBSUB_Subscribe(TOPIC_TICK_1S, HandlerC);
    CHECK(PUBSUB_GetSubscriberCount(TOPIC_TICK_1S) == 3u, "three subscribers registered");

    PUBSUB_Data data;
    data.tick_ms = 1000u;
    PUBSUB_Publish(TOPIC_TICK_1S, data);

    CHECK(g_calls_a == 1, "HandlerA called");
    CHECK(g_calls_b == 1, "HandlerB called");
    CHECK(g_calls_c == 1, "HandlerC called");

    return 1;
}

static int test_topic_isolation(void)
{
    reset_state();

    /* HandlerA only subscribes to TOPIC_TICK_1S. */
    PUBSUB_Subscribe(TOPIC_TICK_1S, HandlerA);

    PUBSUB_Data data;
    data.tick_ms = 1u;

    /* Publishing to a different topic must not invoke HandlerA. */
    PUBSUB_Publish(TOPIC_TICK_1MS,   data);
    PUBSUB_Publish(TOPIC_TICK_100MS, data);
    PUBSUB_Publish(TOPIC_BUTTON,     data);
    PUBSUB_Publish(TOPIC_TEMPERATURE, data);
    CHECK(g_calls_a == 0, "handler not triggered by other topics");

    /* Only a matching publish fires it. */
    PUBSUB_Publish(TOPIC_TICK_1S, data);
    CHECK(g_calls_a == 1, "handler triggered by subscribed topic");

    return 1;
}

static int test_subscriber_limit(void)
{
    reset_state();

    /* Fill the table to PUBSUB_MAX_SUBSCRIBERS. */
    CHECK(PUBSUB_Subscribe(TOPIC_BUTTON, HandlerA) == PUBSUB_OK, "slot 1 OK");
    CHECK(PUBSUB_Subscribe(TOPIC_BUTTON, HandlerB) == PUBSUB_OK, "slot 2 OK");
    CHECK(PUBSUB_Subscribe(TOPIC_BUTTON, HandlerC) == PUBSUB_OK, "slot 3 OK");
    CHECK(PUBSUB_Subscribe(TOPIC_BUTTON, HandlerD) == PUBSUB_OK, "slot 4 OK");
    CHECK(PUBSUB_Subscribe(TOPIC_BUTTON, HandlerE) == PUBSUB_OK, "slot 5 OK");
    CHECK(PUBSUB_Subscribe(TOPIC_BUTTON, HandlerF) == PUBSUB_OK, "slot 6 OK");
    CHECK(PUBSUB_Subscribe(TOPIC_BUTTON, HandlerG) == PUBSUB_OK, "slot 7 OK");
    CHECK(PUBSUB_Subscribe(TOPIC_BUTTON, HandlerH) == PUBSUB_OK, "slot 8 OK (max)");
    CHECK(PUBSUB_GetSubscriberCount(TOPIC_BUTTON) == PUBSUB_MAX_SUBSCRIBERS,
          "count equals max");

    /* One more must be rejected. */
    PUBSUB_Status overflow = PUBSUB_Subscribe(TOPIC_BUTTON, HandlerA);
    CHECK(overflow == PUBSUB_ERR_FULL, "subscribe beyond max returns ERR_FULL");
    CHECK(PUBSUB_GetSubscriberCount(TOPIC_BUTTON) == PUBSUB_MAX_SUBSCRIBERS,
          "count unchanged after overflow attempt");

    return 1;
}

static int test_invalid_inputs(void)
{
    reset_state();

    /* NULL handler */
    CHECK(PUBSUB_Subscribe(TOPIC_TICK_1MS, NULL) == PUBSUB_ERR_INVALID,
          "NULL handler returns ERR_INVALID");

    /* Out-of-range topic for subscribe */
    CHECK(PUBSUB_Subscribe(PUBSUB_TOPIC_COUNT, HandlerA) == PUBSUB_ERR_INVALID,
          "out-of-range topic returns ERR_INVALID on subscribe");

    /* Out-of-range topic for unsubscribe */
    CHECK(PUBSUB_Unsubscribe(PUBSUB_TOPIC_COUNT, HandlerA) == PUBSUB_ERR_INVALID,
          "out-of-range topic returns ERR_INVALID on unsubscribe");

    /* NULL handler for unsubscribe */
    CHECK(PUBSUB_Unsubscribe(TOPIC_TICK_1MS, NULL) == PUBSUB_ERR_INVALID,
          "NULL handler returns ERR_INVALID on unsubscribe");

    /* Out-of-range topic for GetSubscriberCount */
    CHECK(PUBSUB_GetSubscriberCount(PUBSUB_TOPIC_COUNT) == 0u,
          "out-of-range topic returns 0 from GetSubscriberCount");

    /* Publishing to an out-of-range topic must not crash. */
    PUBSUB_Data data;
    data.tick_ms = 0u;
    PUBSUB_Publish(PUBSUB_TOPIC_COUNT, data);
    CHECK(1, "publish to out-of-range topic does not crash");

    return 1;
}

static int test_unsubscribe(void)
{
    reset_state();

    PUBSUB_Subscribe(TOPIC_TICK_100MS, HandlerA);
    PUBSUB_Subscribe(TOPIC_TICK_100MS, HandlerB);
    CHECK(PUBSUB_GetSubscriberCount(TOPIC_TICK_100MS) == 2u, "two subscribers before unsubscribe");

    PUBSUB_Status s = PUBSUB_Unsubscribe(TOPIC_TICK_100MS, HandlerA);
    CHECK(s == PUBSUB_OK, "unsubscribe returns OK");
    CHECK(PUBSUB_GetSubscriberCount(TOPIC_TICK_100MS) == 1u, "count decrements after unsubscribe");

    /* HandlerA must not be called; HandlerB must still be called. */
    PUBSUB_Data data;
    data.tick_ms = 100u;
    PUBSUB_Publish(TOPIC_TICK_100MS, data);
    CHECK(g_calls_a == 0, "unsubscribed handler not called");
    CHECK(g_calls_b == 1, "remaining handler still called");

    return 1;
}

static int test_unsubscribe_nonexistent(void)
{
    reset_state();

    /* HandlerA never subscribed — must return ERR_INVALID, not crash. */
    PUBSUB_Status s = PUBSUB_Unsubscribe(TOPIC_TICK_1MS, HandlerA);
    CHECK(s == PUBSUB_ERR_INVALID, "unsubscribe of non-subscriber returns ERR_INVALID");

    return 1;
}

static int test_resubscribe_after_unsubscribe(void)
{
    reset_state();

    PUBSUB_Subscribe(TOPIC_TEMPERATURE, HandlerA);

    PUBSUB_Data data;
    data.temperature_decidegc = 250;
    PUBSUB_Publish(TOPIC_TEMPERATURE, data);
    CHECK(g_calls_a == 1, "handler called before unsubscribe");

    PUBSUB_Unsubscribe(TOPIC_TEMPERATURE, HandlerA);

    PUBSUB_Publish(TOPIC_TEMPERATURE, data);
    CHECK(g_calls_a == 1, "handler not called after unsubscribe");

    /* Re-subscribe and verify it fires again. */
    PUBSUB_Subscribe(TOPIC_TEMPERATURE, HandlerA);
    PUBSUB_Publish(TOPIC_TEMPERATURE, data);
    CHECK(g_calls_a == 2, "handler called again after re-subscribe");

    return 1;
}

static int test_unsubscribe_middle_preserves_order(void)
{
    reset_state();

    /* Subscribe A, B, C then remove the middle one (B). */
    PUBSUB_Subscribe(TOPIC_TICK_1S, HandlerA);
    PUBSUB_Subscribe(TOPIC_TICK_1S, HandlerB);
    PUBSUB_Subscribe(TOPIC_TICK_1S, HandlerC);

    PUBSUB_Unsubscribe(TOPIC_TICK_1S, HandlerB);
    CHECK(PUBSUB_GetSubscriberCount(TOPIC_TICK_1S) == 2u, "count is 2 after removing middle");

    PUBSUB_Data data;
    data.tick_ms = 1000u;
    PUBSUB_Publish(TOPIC_TICK_1S, data);
    CHECK(g_calls_a == 1, "first subscriber still fires");
    CHECK(g_calls_b == 0, "removed subscriber does not fire");
    CHECK(g_calls_c == 1, "last subscriber still fires");

    return 1;
}

static int test_message_data_per_topic(void)
{
    reset_state();

    PUBSUB_Subscribe(TOPIC_BUTTON,      HandlerA);
    PUBSUB_Subscribe(TOPIC_TEMPERATURE, HandlerB);

    PUBSUB_Data btn_data;
    btn_data.button_state = 1u;
    PUBSUB_Publish(TOPIC_BUTTON, btn_data);

    PUBSUB_Data temp_data;
    temp_data.temperature_decidegc = -50;
    PUBSUB_Publish(TOPIC_TEMPERATURE, temp_data);

    CHECK(g_last_a.data.button_state == 1u, "button payload correct");
    CHECK(g_last_b.data.temperature_decidegc == -50, "temperature payload correct");
    CHECK(g_last_a.topic == TOPIC_BUTTON, "button message topic field correct");
    CHECK(g_last_b.topic == TOPIC_TEMPERATURE, "temperature message topic field correct");

    return 1;
}

/* ------------------------------------------------------------------ */
/* Scheduler tests                                                     */
/* ------------------------------------------------------------------ */

/* Counters tracking how many times each scheduler topic fires. */
static int g_sched_1ms   = 0;
static int g_sched_100ms = 0;
static int g_sched_1s    = 0;
static uint32_t g_last_tick_1ms   = 0u;
static uint32_t g_last_tick_100ms = 0u;
static uint32_t g_last_tick_1s    = 0u;

static void OnSched1ms(const PUBSUB_Message *m)
{
    g_sched_1ms++;
    g_last_tick_1ms = m->data.tick_ms;
}
static void OnSched100ms(const PUBSUB_Message *m)
{
    g_sched_100ms++;
    g_last_tick_100ms = m->data.tick_ms;
}
static void OnSched1s(const PUBSUB_Message *m)
{
    g_sched_1s++;
    g_last_tick_1s = m->data.tick_ms;
}

static void reset_sched_state(void)
{
    g_sched_1ms   = 0;
    g_sched_100ms = 0;
    g_sched_1s    = 0;
    g_last_tick_1ms   = 0u;
    g_last_tick_100ms = 0u;
    g_last_tick_1s    = 0u;
    PUBSUB_Init();
    SCHEDULER_Init();

    PUBSUB_Subscribe(TOPIC_TICK_1MS,   OnSched1ms);
    PUBSUB_Subscribe(TOPIC_TICK_100MS, OnSched100ms);
    PUBSUB_Subscribe(TOPIC_TICK_1S,    OnSched1s);
}

static int test_scheduler_init(void)
{
    reset_sched_state();
    CHECK(SCHEDULER_GetTickMs() == 0u, "tick starts at 0 after init");
    return 1;
}

static int test_scheduler_1ms_fires_every_tick(void)
{
    reset_sched_state();

    for (int i = 0; i < 10; i++) {
        SCHEDULER_Tick();
    }
    CHECK(g_sched_1ms == 10, "TOPIC_TICK_1MS fires once per tick");
    CHECK(SCHEDULER_GetTickMs() == 10u, "GetTickMs returns correct count");

    return 1;
}

static int test_scheduler_100ms_interval(void)
{
    reset_sched_state();

    /* 99 ticks: 100ms topic should NOT have fired yet. */
    for (int i = 0; i < 99; i++) {
        SCHEDULER_Tick();
    }
    CHECK(g_sched_100ms == 0, "100ms topic silent before 100 ticks");

    /* Tick 100: fires for the first time. */
    SCHEDULER_Tick();
    CHECK(g_sched_100ms == 1, "100ms topic fires at tick 100");
    CHECK(g_last_tick_100ms == 100u, "100ms message carries tick 100");

    /* Another 100 ticks: fires a second time. */
    for (int i = 0; i < 100; i++) {
        SCHEDULER_Tick();
    }
    CHECK(g_sched_100ms == 2, "100ms topic fires again at tick 200");
    CHECK(g_last_tick_100ms == 200u, "100ms message carries tick 200");

    return 1;
}

static int test_scheduler_1s_interval(void)
{
    reset_sched_state();

    /* 999 ticks: 1s topic must not fire. */
    for (int i = 0; i < 999; i++) {
        SCHEDULER_Tick();
    }
    CHECK(g_sched_1s == 0, "1s topic silent before 1000 ticks");

    /* Tick 1000: fires. */
    SCHEDULER_Tick();
    CHECK(g_sched_1s == 1, "1s topic fires at tick 1000");
    CHECK(g_last_tick_1s == 1000u, "1s message carries tick 1000");

    /* Another 1000 ticks. */
    for (int i = 0; i < 1000; i++) {
        SCHEDULER_Tick();
    }
    CHECK(g_sched_1s == 2, "1s topic fires again at tick 2000");

    return 1;
}

static int test_scheduler_1s_includes_100ms(void)
{
    reset_sched_state();

    /* At tick 1000, both the 100ms and 1s topics must fire. */
    for (int i = 0; i < 1000; i++) {
        SCHEDULER_Tick();
    }
    /* 100ms topic: 1000 / 100 = 10 times */
    CHECK(g_sched_100ms == 10, "100ms topic fired 10 times in 1000 ticks");
    /* 1s topic: once */
    CHECK(g_sched_1s == 1, "1s topic fired once in 1000 ticks");
    /* 1ms topic: every tick */
    CHECK(g_sched_1ms == 1000, "1ms topic fired 1000 times");

    return 1;
}

static int test_scheduler_tick_data_accuracy(void)
{
    reset_sched_state();

    for (int i = 0; i < 500; i++) {
        SCHEDULER_Tick();
    }
    CHECK(g_last_tick_1ms == 500u, "last 1ms message tick equals 500");

    return 1;
}

/* ------------------------------------------------------------------ */
/* Main                                                                */
/* ------------------------------------------------------------------ */

int main(void)
{
    printf("=== Publish/Subscribe Pattern — Test Suite ===\n\n");

    printf("--- Core pub/sub ---\n");
    test_subscribe_and_publish();
    test_multiple_subscribers_same_topic();
    test_topic_isolation();
    test_subscriber_limit();
    test_invalid_inputs();
    test_unsubscribe();
    test_unsubscribe_nonexistent();
    test_resubscribe_after_unsubscribe();
    test_unsubscribe_middle_preserves_order();
    test_message_data_per_topic();

    printf("\n--- Scheduler (time-driven publisher) ---\n");
    test_scheduler_init();
    test_scheduler_1ms_fires_every_tick();
    test_scheduler_100ms_interval();
    test_scheduler_1s_interval();
    test_scheduler_1s_includes_100ms();
    test_scheduler_tick_data_accuracy();

    int total = g_pass + g_fail;
    printf("\n==============================================\n");
    printf("Results: %d/%d passed", g_pass, total);
    if (g_fail > 0) {
        printf("   *** %d FAILED ***", g_fail);
    }
    printf("\n==============================================\n");

    return (g_fail == 0) ? 0 : 1;
}
