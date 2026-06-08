/*
 * Author:      Kevin Fox
 * Description: Thread-safe circular buffer for UART DMA/interrupt operations.
 *
 * Non-blocking, atomic-based ring buffer for concurrent producer/consumer.
 * Supports bounded, deterministic memory usage (fixed size at init).
 */
#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdint.h>
#include <stddef.h>

/* Ring buffer handle (opaque). */
typedef struct
{
    uint8_t          *buffer;
    uint16_t          capacity;
    volatile uint16_t writeIdx;
    volatile uint16_t readIdx;
} ringbuffer_t;

/* Initialize ring buffer with fixed capacity. Returns 0 on success. */
int ringbuffer_init(ringbuffer_t *rb, uint8_t *data, uint16_t capacity);

/* Write single byte (non-blocking). Returns 0 on success, -1 if full. */
int ringbuffer_write(ringbuffer_t *rb, uint8_t byte);

/* Read single byte (non-blocking). Returns byte on success, -1 if empty. */
int ringbuffer_read(ringbuffer_t *rb);

/* Get number of bytes available to read. */
uint16_t ringbuffer_available(const ringbuffer_t *rb);

/* Get number of bytes available to write. */
uint16_t ringbuffer_free(const ringbuffer_t *rb);

/* Clear all data (reset indices). */
void ringbuffer_clear(ringbuffer_t *rb);

#endif /* RINGBUFFER_H */
