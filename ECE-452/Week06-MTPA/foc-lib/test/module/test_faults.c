/**
 * @file test_faults.c
 * @brief Module tests for motor_faults (Phase 6, §11.2 row 4).
 */
#include "../../test/test_harness.h"
#include "foc/motor_faults.h"
#include <stdio.h>

TEST_HARNESS_IMPL

static void test_raise_immediate(void)
{
    foc_faults_t f;
    foc_faults_init(&f, g_foc_fault_map_default, FOC_FAULT_NUM_ENTRIES);
    foc_faults_raise(&f, FOC_FAULT_OC_SW);
    TEST_ASSERT_TRUE(foc_faults_active(&f) & FOC_FAULT_OC_SW);
}

static void test_debounce_exactness(void)
{
    foc_faults_t f;
    foc_faults_init(&f, g_foc_fault_map_default, FOC_FAULT_NUM_ENTRIES);
    /* OC_SW debounce count in default map = 3. */
    foc_faults_raise_debounced(&f, FOC_FAULT_OC_SW, 3u);
    TEST_ASSERT_EQUAL_INT(0u, foc_faults_active(&f) & FOC_FAULT_OC_SW); /* not yet */
    foc_faults_raise_debounced(&f, FOC_FAULT_OC_SW, 3u);
    TEST_ASSERT_EQUAL_INT(0u, foc_faults_active(&f) & FOC_FAULT_OC_SW); /* still not */
    foc_faults_raise_debounced(&f, FOC_FAULT_OC_SW, 3u);
    TEST_ASSERT_TRUE(foc_faults_active(&f) & FOC_FAULT_OC_SW); /* now latched */
}

static void test_clear(void)
{
    foc_faults_t f;
    foc_faults_init(&f, g_foc_fault_map_default, FOC_FAULT_NUM_ENTRIES);
    foc_faults_raise(&f, FOC_FAULT_OC_SW);
    foc_faults_clear(&f, FOC_FAULT_OC_SW);
    TEST_ASSERT_EQUAL_INT(0u, foc_faults_active(&f) & FOC_FAULT_OC_SW);
}

static void test_fatal_blocks_retry(void)
{
    foc_faults_t f;
    foc_faults_init(&f, g_foc_fault_map_default, FOC_FAULT_NUM_ENTRIES);
    foc_faults_raise(&f, FOC_FAULT_OC_HW); /* FATAL */
    TEST_ASSERT_EQUAL_INT(FOC_FALSE, foc_faults_retry_allowed(&f));
}

static void test_retry_budget_exhaustion(void)
{
    foc_faults_t f;
    foc_faults_init(&f, g_foc_fault_map_default, FOC_FAULT_NUM_ENTRIES);
    foc_faults_raise(&f, FOC_FAULT_OC_SW); /* AUTO_RETRY max=2 */
    /* Clear cooldown first. */
    foc_faults_tick(&f, 1000u);
    foc_bool_t r1 = foc_faults_retry_allowed(&f);
    TEST_ASSERT_EQUAL_INT(FOC_TRUE, r1);
    foc_faults_tick(&f, 600u);
    /* Second retry. */
    foc_bool_t r2 = foc_faults_retry_allowed(&f);
    TEST_ASSERT_EQUAL_INT(FOC_TRUE, r2);
    /* Third call should fail (budget exhausted → FATAL). */
    foc_bool_t r3 = foc_faults_retry_allowed(&f);
    TEST_ASSERT_EQUAL_INT(FOC_FALSE, r3);
}

static void test_healthy_tick_resets_counters(void)
{
    foc_faults_t f;
    foc_faults_init(&f, g_foc_fault_map_default, FOC_FAULT_NUM_ENTRIES);
    /* Exhaust retry counter for OC_SW. */
    foc_faults_raise(&f, FOC_FAULT_OC_SW);
    foc_faults_tick(&f, 1000u);
    foc_faults_retry_allowed(&f); /* consume one */
    foc_faults_retry_allowed(&f); /* consume two */
    /* Simulate 30 s of healthy run → reset. */
    foc_faults_healthy_tick(&f, 30000u);
    /* After clearing fault, retry should be available again. */
    foc_faults_clear(&f, FOC_FAULT_OC_SW);
    foc_faults_raise(&f, FOC_FAULT_OC_SW);
    foc_faults_tick(&f, 1000u);
    TEST_ASSERT_EQUAL_INT(FOC_TRUE, foc_faults_retry_allowed(&f));
}

int main(void)
{
    printf("=== test_faults ===\n");
    RUN_TEST(test_raise_immediate);
    RUN_TEST(test_debounce_exactness);
    RUN_TEST(test_clear);
    RUN_TEST(test_fatal_blocks_retry);
    RUN_TEST(test_retry_budget_exhaustion);
    RUN_TEST(test_healthy_tick_resets_counters);
    TEST_REPORT();
}
