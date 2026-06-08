/*
 * Author:      Kevin Fox
 * Description: Thread-safe circular buffer implementation.
 *
 * Non-blocking ring buffer using index-based access.
 * Supports safe concurrent access from ISR and main thread.
 */
#include "ringbuffer.h"

/* Critical section macros: disable/enable interrupts on MCU targets,
 * no-ops for host unit tests. Use compiler predefined macros to detect ARM.
 */
#if defined(__arm__) || defined(__ARM_ARCH) || defined(__thumb__) || defined(__aarch64__)
#include "stm32u575xx.h"
#define RB_CRITICAL_DISABLE() __disable_irq()
#define RB_CRITICAL_ENABLE()  __enable_irq()
#else
#define RB_CRITICAL_DISABLE() ((void)0)
#define RB_CRITICAL_ENABLE()  ((void)0)
#endif

int ringbuffer_init(ringbuffer_t *rb, uint8_t *data, uint16_t capacity)
{
    if (!rb || !data || capacity == 0)
    {
        return -1;
    }
    rb->buffer   = data;
    rb->capacity = capacity;
    rb->writeIdx = 0;
    rb->readIdx  = 0;
    return 0;
}

int ringbuffer_write(ringbuffer_t *rb, uint8_t byte)
{
    if (!rb)
        return -1;
    uint16_t nextWriteIdx;

    RB_CRITICAL_DISABLE();
    nextWriteIdx = (rb->writeIdx + 1) % rb->capacity;
    if (nextWriteIdx == rb->readIdx)
    {
        RB_CRITICAL_ENABLE();
        return -1; /* Buffer full */
    }

    rb->buffer[rb->writeIdx] = byte;
    rb->writeIdx             = nextWriteIdx;
    RB_CRITICAL_ENABLE();
    return 0;
}

int ringbuffer_read(ringbuffer_t *rb)
{
    if (!rb)
        return -1;
    uint8_t byte;

    RB_CRITICAL_DISABLE();
    if (rb->readIdx == rb->writeIdx)
    {
        RB_CRITICAL_ENABLE();
        return -1; /* Buffer empty */
    }

    byte        = rb->buffer[rb->readIdx];
    rb->readIdx = (rb->readIdx + 1) % rb->capacity;
    RB_CRITICAL_ENABLE();
    return (int)byte;
}

uint16_t ringbuffer_available(const ringbuffer_t *rb)
{
    if (!rb)
        return 0;
    uint16_t w, r;
    RB_CRITICAL_DISABLE();
    w = rb->writeIdx;
    r = rb->readIdx;
    RB_CRITICAL_ENABLE();

    if (w >= r)
    {
        return w - r;
    }
    else
    {
        return rb->capacity - (r - w);
    }
}

uint16_t ringbuffer_free(const ringbuffer_t *rb)
{
    if (!rb)
        return 0;
    return rb->capacity - 1 - ringbuffer_available(rb);
}

void ringbuffer_clear(ringbuffer_t *rb)
{
    if (!rb)
        return;
    RB_CRITICAL_DISABLE();
    rb->writeIdx = 0;
    rb->readIdx  = 0;
    RB_CRITICAL_ENABLE();
}
