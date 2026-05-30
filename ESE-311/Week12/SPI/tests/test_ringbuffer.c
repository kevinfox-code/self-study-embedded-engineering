/*
 * Host-runnable unit tests for the ring buffer module.
 *
 * Build with:
 *   cc -std=c11 -Wall -Wextra -IInc \
 *      tests/test_ringbuffer.c Src/ringbuffer.c \
 *      -o test_ringbuffer
 * Run with:
 *   ./test_ringbuffer
 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "ringbuffer.h"

#define ANSI_GREEN "\x1b[32m"
#define ANSI_RED "\x1b[31m"
#define ANSI_RESET "\x1b[0m"

static int tests_passed = 0;

#define RUN_TEST(fn, description) \
    do { \
        printf("Running: %s ... ", description); \
        fflush(stdout); \
        fn(); \
        printf(ANSI_GREEN "PASS" ANSI_RESET "\n"); \
        tests_passed++; \
    } while (0)

static void test_ringbuffer_init_configures_state(void)
{
    ringbuffer_t rb;
    uint8_t storage[4] = { 0U, 0U, 0U, 0U };

    assert(ringbuffer_init(&rb, storage, (uint16_t)sizeof(storage)) == 0);
    assert(rb.buffer == storage);
    assert(rb.capacity == (uint16_t)sizeof(storage));
    assert(rb.writeIdx == 0U);
    assert(rb.readIdx == 0U);
    assert(ringbuffer_available(&rb) == 0U);
    assert(ringbuffer_free(&rb) == 3U);
}

static void test_ringbuffer_init_rejects_invalid_params(void)
{
    ringbuffer_t rb;
    uint8_t storage[4] = { 0U, 0U, 0U, 0U };

    assert(ringbuffer_init(NULL, storage, (uint16_t)sizeof(storage)) == -1);
    assert(ringbuffer_init(&rb, NULL, (uint16_t)sizeof(storage)) == -1);
    assert(ringbuffer_init(&rb, storage, 0U) == -1);
}

static void test_ringbuffer_write_and_read_single_byte(void)
{
    ringbuffer_t rb;
    uint8_t storage[4] = { 0U, 0U, 0U, 0U };

    assert(ringbuffer_init(&rb, storage, (uint16_t)sizeof(storage)) == 0);
    assert(ringbuffer_write(&rb, 0xABU) == 0);
    assert(ringbuffer_available(&rb) == 1U);
    assert(ringbuffer_free(&rb) == 2U);
    assert(ringbuffer_read(&rb) == 0xAB);
    assert(ringbuffer_available(&rb) == 0U);
    assert(ringbuffer_free(&rb) == 3U);
}

static void test_ringbuffer_write_rejects_when_full(void)
{
    ringbuffer_t rb;
    uint8_t storage[4] = { 0U, 0U, 0U, 0U };

    assert(ringbuffer_init(&rb, storage, (uint16_t)sizeof(storage)) == 0);
    assert(ringbuffer_write(&rb, 0x10U) == 0);
    assert(ringbuffer_write(&rb, 0x20U) == 0);
    assert(ringbuffer_write(&rb, 0x30U) == 0);
    assert(ringbuffer_write(&rb, 0x40U) == -1);
    assert(ringbuffer_available(&rb) == 3U);
    assert(ringbuffer_free(&rb) == 0U);
}

static void test_ringbuffer_read_rejects_when_empty(void)
{
    ringbuffer_t rb;
    uint8_t storage[4] = { 0U, 0U, 0U, 0U };

    assert(ringbuffer_init(&rb, storage, (uint16_t)sizeof(storage)) == 0);
    assert(ringbuffer_read(&rb) == -1);
    assert(ringbuffer_available(&rb) == 0U);
    assert(ringbuffer_free(&rb) == 3U);
}

static void test_ringbuffer_wraparound_preserves_fifo_order(void)
{
    ringbuffer_t rb;
    uint8_t storage[4] = { 0U, 0U, 0U, 0U };

    assert(ringbuffer_init(&rb, storage, (uint16_t)sizeof(storage)) == 0);
    assert(ringbuffer_write(&rb, 0x01U) == 0);
    assert(ringbuffer_write(&rb, 0x02U) == 0);
    assert(ringbuffer_write(&rb, 0x03U) == 0);

    assert(ringbuffer_read(&rb) == 0x01);
    assert(ringbuffer_write(&rb, 0x04U) == 0);

    assert(ringbuffer_available(&rb) == 3U);
    assert(ringbuffer_read(&rb) == 0x02);
    assert(ringbuffer_read(&rb) == 0x03);
    assert(ringbuffer_read(&rb) == 0x04);
    assert(ringbuffer_read(&rb) == -1);
}

static void test_ringbuffer_clear_resets_state(void)
{
    ringbuffer_t rb;
    uint8_t storage[4] = { 0U, 0U, 0U, 0U };

    assert(ringbuffer_init(&rb, storage, (uint16_t)sizeof(storage)) == 0);
    assert(ringbuffer_write(&rb, 0x55U) == 0);
    assert(ringbuffer_write(&rb, 0x66U) == 0);

    ringbuffer_clear(&rb);

    assert(rb.writeIdx == 0U);
    assert(rb.readIdx == 0U);
    assert(ringbuffer_available(&rb) == 0U);
    assert(ringbuffer_free(&rb) == 3U);
    assert(ringbuffer_read(&rb) == -1);
    assert(ringbuffer_write(&rb, 0x77U) == 0);
    assert(ringbuffer_read(&rb) == 0x77);
}

static void test_ringbuffer_capacity_one_stays_unwritable(void)
{
    ringbuffer_t rb;
    uint8_t storage[1] = { 0U };

    assert(ringbuffer_init(&rb, storage, (uint16_t)sizeof(storage)) == 0);
    assert(ringbuffer_available(&rb) == 0U);
    assert(ringbuffer_free(&rb) == 0U);
    assert(ringbuffer_write(&rb, 0xAAU) == -1);
    assert(ringbuffer_read(&rb) == -1);
}

int main(void)
{
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║            Ring Buffer Host Unit Test Suite               ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    RUN_TEST(test_ringbuffer_init_configures_state, "ringbuffer_init sets the storage, capacity, and indices");
    RUN_TEST(test_ringbuffer_init_rejects_invalid_params, "ringbuffer_init rejects NULL pointers and zero capacity");
    RUN_TEST(test_ringbuffer_write_and_read_single_byte, "ringbuffer_write and ringbuffer_read work for one byte");
    RUN_TEST(test_ringbuffer_write_rejects_when_full, "ringbuffer_write rejects writes once the buffer is full");
    RUN_TEST(test_ringbuffer_read_rejects_when_empty, "ringbuffer_read rejects reads from an empty buffer");
    RUN_TEST(test_ringbuffer_wraparound_preserves_fifo_order, "ringbuffer preserves FIFO order across wraparound");
    RUN_TEST(test_ringbuffer_clear_resets_state, "ringbuffer_clear resets indices and restores empty state");
    RUN_TEST(test_ringbuffer_capacity_one_stays_unwritable, "capacity-one buffers remain empty by design");

    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║ Test Summary                                               ║\n");
    printf("║ Passed: %-3d   Total: %-3d                                  ║\n", tests_passed, tests_passed);
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    printf(ANSI_GREEN "SUCCESS: All %d tests passed!\n" ANSI_RESET, tests_passed);
    return 0;
}