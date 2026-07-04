/**
 * @file test_harness.h
 * @brief Lightweight assert-based test harness (Decision D-001).
 *        Provides Unity-compatible macro names backed by assert() + printf.
 *        Include this header in every test file instead of Unity.
 */
#ifndef FOC_TEST_HARNESS_H
#define FOC_TEST_HARNESS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Global test counters (defined once per translation unit via the macro
 * TEST_HARNESS_IMPL at the bottom of the file that has main()).
 * ---------------------------------------------------------------------- */
extern int th_tests_run;
extern int th_tests_failed;
extern const char *th_current_test;

/* -------------------------------------------------------------------------
 * Saturation event counter (Decision D-002).
 * Tests can check this to assert zero unexpected saturations.
 * ---------------------------------------------------------------------- */
extern int g_sat_count;

/* When FOC_TESTING is set, foc_sat_event() is declared in motor_math.h and
 * defined in TEST_HARNESS_IMPL.  When building without FOC_TESTING we also
 * keep a local macro override for any code included directly in the test TU. */
#ifndef FOC_TESTING
#undef  FOC_SAT_HOOK
#define FOC_SAT_HOOK(tag) do { g_sat_count++; } while(0)
#endif

/* -------------------------------------------------------------------------
 * RUN_TEST macro — call before each test function.
 * ---------------------------------------------------------------------- */
#define RUN_TEST(fn) \
    do { \
        th_current_test = #fn; \
        th_tests_run++; \
        int _before = th_tests_failed; \
        fn(); \
        if (th_tests_failed == _before) { \
            printf("  PASS  %s\n", #fn); \
        } \
    } while(0)

/* -------------------------------------------------------------------------
 * Assertion macros (print file+line on failure, do not abort immediately).
 * ---------------------------------------------------------------------- */

/* Internal helper — avoids GNU ##__VA_ARGS__ extension. */
static inline void th_fail_msg(const char *test, const char *file, int line,
                                const char *msg)
{
    printf("  FAIL  %s [%s:%d]  %s\n", test, file, line, msg);
}

/* For formatted failure messages, tests call th_fail_fmt directly. */
#define _TH_FAIL(msg) \
    do { \
        th_fail_msg(th_current_test, __FILE__, __LINE__, msg); \
        th_tests_failed++; \
    } while(0)

#define _TH_FAIL_FMT(fmt, ...) \
    do { \
        char _th_buf[256]; \
        snprintf(_th_buf, sizeof(_th_buf), fmt, __VA_ARGS__); \
        th_fail_msg(th_current_test, __FILE__, __LINE__, _th_buf); \
        th_tests_failed++; \
    } while(0)

#define TEST_ASSERT_TRUE(cond) \
    do { if (!(cond)) { _TH_FAIL("expected TRUE: " #cond); } } while(0)

#define TEST_ASSERT_FALSE(cond) \
    do { if ((cond)) { _TH_FAIL("expected FALSE: " #cond); } } while(0)

#define TEST_ASSERT_EQUAL_INT(expected, actual) \
    do { \
        long long _e = (long long)(expected); \
        long long _a = (long long)(actual); \
        if (_e != _a) { _TH_FAIL_FMT("expected %lld got %lld", _e, _a); } \
    } while(0)

#define TEST_ASSERT_EQUAL_UINT(expected, actual) \
    do { \
        unsigned long long _e = (unsigned long long)(expected); \
        unsigned long long _a = (unsigned long long)(actual); \
        if (_e != _a) { _TH_FAIL_FMT("expected 0x%llx got 0x%llx", _e, _a); } \
    } while(0)

#define TEST_ASSERT_INT_WITHIN(delta, expected, actual) \
    do { \
        long long _e = (long long)(expected); \
        long long _a = (long long)(actual); \
        long long _d = (long long)(delta); \
        long long _diff = _a - _e; \
        if (_diff < 0) _diff = -_diff; \
        if (_diff > _d) { _TH_FAIL_FMT("|%lld - %lld| = %lld > delta %lld", _e, _a, _diff, _d); } \
    } while(0)

#define TEST_ASSERT_EQUAL_PTR(expected, actual) \
    do { \
        if ((const void *)(expected) != (const void *)(actual)) { \
            _TH_FAIL("pointer mismatch"); } \
    } while(0)

#define TEST_ASSERT_NOT_NULL(ptr) \
    do { if ((ptr) == NULL) { _TH_FAIL(#ptr " is NULL"); } } while(0)

/* -------------------------------------------------------------------------
 * Test result reporting (call from main after all RUN_TEST).
 * ---------------------------------------------------------------------- */
#define TEST_REPORT() \
    do { \
        printf("\n--- Results: %d/%d passed ---\n", \
               th_tests_run - th_tests_failed, th_tests_run); \
        if (th_tests_failed > 0) { \
            printf("FAILURES: %d\n", th_tests_failed); \
            return 1; \
        } \
        printf("ALL TESTS PASSED\n"); \
        return 0; \
    } while(0)

/* -------------------------------------------------------------------------
 * Define this macro exactly ONCE, in the file that contains main().
 * ---------------------------------------------------------------------- */
#define TEST_HARNESS_IMPL \
    int th_tests_run    = 0; \
    int th_tests_failed = 0; \
    const char *th_current_test = ""; \
    int g_sat_count = 0; \
    void foc_sat_event(const char *tag) { (void)tag; g_sat_count++; }

#endif /* FOC_TEST_HARNESS_H */
