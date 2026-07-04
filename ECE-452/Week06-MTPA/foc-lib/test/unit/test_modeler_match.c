/**
 * @file test_modeler_match.c
 * @brief Tests for motor profile matching (§5.13): foc_modeler_match against
 *        the default profile table, including the user_motor_01 acceptance
 *        case.
 */
#include "../../test/test_harness.h"
#include "foc/motor_modeler.h"
#include "foc/motor_types.h"
#include "foc/motor_math.h"

#include <string.h>
#include <stdio.h>

TEST_HARNESS_IMPL

static void test_exact_match_high_score(void)
{
    const foc_profile_table_t *table = foc_modeler_default_table();
    TEST_ASSERT_NOT_NULL(table);
    TEST_ASSERT_TRUE(table->count >= 4u);

    /* Exact match of MidDrone_24V's params should return that profile with
     * a high score. */
    const foc_motor_params_t *mid = &table->entries[1].params;
    TEST_ASSERT_EQUAL_INT(0, strcmp(table->entries[1].name, "MidDrone_24V"));

    q16_t score = 0;
    const foc_motor_profile_t *best = foc_modeler_match(table, mid, &score);
    TEST_ASSERT_NOT_NULL(best);
    TEST_ASSERT_EQUAL_INT(0, strcmp(best->name, "MidDrone_24V"));
    TEST_ASSERT_TRUE(score >= MODELER_MIN_SCORE);
}

static void test_ambiguous_low_score(void)
{
    const foc_profile_table_t *table = foc_modeler_default_table();

    /* A motor with parameters far from every profile should still return
     * a best match, but with a low score below the acceptance threshold. */
    foc_motor_params_t weird = {
        Q16(500.0), Q16(500.0), Q16(5000.0), 50u, Q16(0.001), Q16(1.0)
    };

    q16_t score = 0;
    const foc_motor_profile_t *best = foc_modeler_match(table, &weird, &score);
    TEST_ASSERT_NOT_NULL(best);
    TEST_ASSERT_TRUE(score < MODELER_MIN_SCORE);
}

static void test_user_motor_01_acceptance(void)
{
    const foc_profile_table_t *table = foc_modeler_default_table();
    TEST_ASSERT_NOT_NULL(table);

    /* Locate user_motor_01 in the default table. */
    const foc_motor_profile_t *user01 = NULL;
    for (uint32_t i = 0u; i < table->count; i++) {
        if (strcmp(table->entries[i].name, "user_motor_01") == 0) {
            user01 = &table->entries[i];
            break;
        }
    }
    TEST_ASSERT_NOT_NULL(user01);

    /* Synthetic ident-style measured params at user_motor_01's nominal
     * values (§5.13 acceptance case). */
    foc_motor_params_t measured = {
        Q16(0.149),   /* rs_ohm */
        Q16(96.0),    /* ls_mh */
        Q16(156.34),  /* lambda_m_mwb */
        1u,           /* pole_pairs */
        Q16(7.0),     /* rated_current_a */
        Q16(60.0)     /* max_speed_radps_m */
    };

    q16_t score = 0;
    const foc_motor_profile_t *best = foc_modeler_match(table, &measured, &score);
    TEST_ASSERT_NOT_NULL(best);
    TEST_ASSERT_EQUAL_INT(0, strcmp(best->name, "user_motor_01"));
    TEST_ASSERT_TRUE(score >= MODELER_MIN_SCORE);

    /* Sanity: the matched entry actually carries the expected plant values. */
    TEST_ASSERT_EQUAL_INT(1, user01->params.pole_pairs);
    TEST_ASSERT_EQUAL_INT(Q16(7.0), user01->params.rated_current_a);
}

static void test_user_motor_01_within_ident_tolerance(void)
{
    /* §5.12 ident tolerances: Rs +/-5%, Ls +/-10%, lambda_m +/-5%. Verify a
     * measured vector perturbed within those tolerances still matches. */
    const foc_profile_table_t *table = foc_modeler_default_table();

    foc_motor_params_t measured = {
        Q16(0.149 * 1.04),   /* Rs +4% */
        Q16(96.0 * 0.92),    /* Ls -8% */
        Q16(156.34 * 1.03),  /* lambda_m +3% */
        1u,
        Q16(7.0),
        Q16(60.0)
    };

    q16_t score = 0;
    const foc_motor_profile_t *best = foc_modeler_match(table, &measured, &score);
    TEST_ASSERT_NOT_NULL(best);
    TEST_ASSERT_EQUAL_INT(0, strcmp(best->name, "user_motor_01"));
    TEST_ASSERT_TRUE(score >= MODELER_MIN_SCORE);
}

int main(void)
{
    printf("=== test_modeler_match ===\n");
    RUN_TEST(test_exact_match_high_score);
    RUN_TEST(test_ambiguous_low_score);
    RUN_TEST(test_user_motor_01_acceptance);
    RUN_TEST(test_user_motor_01_within_ident_tolerance);
    TEST_REPORT();
}
