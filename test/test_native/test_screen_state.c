/**
 * @file test_screen_state.c
 * Unity tests for screen state management module.
 * 
 * Tests state transitions, history navigation, timeouts, and callbacks.
 * Runs on native platform — no ESP32 hardware required.
 */

#include <unity.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "../application/SkinnyCon/src/screen_state_manager.h"

/* Test callback counters */
static int state_change_count = 0;
static int low_power_enter_count = 0;
static int low_power_exit_count = 0;
static int app_enter_count = 0;
static int app_exit_count = 0;

/* Test callback implementations */
static void mock_state_change(ScreenState_t new_state, ScreenState_t old_state)
{
    state_change_count++;
    (void)new_state;
    (void)old_state;
}

static void mock_low_power_enter(void)
{
    low_power_enter_count++;
}

static void mock_low_power_exit(void)
{
    low_power_exit_count++;
}

static void mock_app_enter(const char *app_name)
{
    app_enter_count++;
    (void)app_name;
}

static void mock_app_exit(const char *app_name)
{
    app_exit_count++;
    (void)app_name;
}

/* ================================================================
 *  TEST SETUP/TEARDOWN
 * ================================================================ */

void setUp(void)
{
    /* Reset counters */
    state_change_count = 0;
    low_power_enter_count = 0;
    low_power_exit_count = 0;
    app_enter_count = 0;
    app_exit_count = 0;
}

void tearDown(void)
{
    /* Nothing to clean up */
}

/* ================================================================
 *  INITIALIZATION TESTS
 * ================================================================ */

void test_init_sets_default_state(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    TEST_ASSERT_EQUAL(SCREEN_STATE_IDLE, screen_state_manager_get_state(&manager));
    TEST_ASSERT_EQUAL(SCREEN_STATE_IDLE, screen_state_manager_get_previous_state(&manager));
}

void test_init_clears_history(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    TEST_ASSERT_FALSE(screen_state_manager_has_history(&manager));
    TEST_ASSERT_TRUE(manager.history_enabled);
    TEST_ASSERT_TRUE(manager.callbacks_enabled);
}

void test_init_default_timeout(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    TEST_ASSERT_EQUAL_UINT32(30000, screen_state_manager_get_timeout(&manager));
}

/* ================================================================
 *  STATE TRANSITION TESTS
 * ================================================================ */

void test_transition_idle_to_menu(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, TRANSITION_MANUAL, NULL);
    
    TEST_ASSERT_EQUAL(SCREEN_STATE_MENU, screen_state_manager_get_state(&manager));
    TEST_ASSERT_EQUAL(SCREEN_STATE_IDLE, screen_state_manager_get_previous_state(&manager));
    TEST_ASSERT_EQUAL_INT(1, state_change_count);
}

void test_transition_menu_to_app(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    /* Setup callbacks */
    screen_state_manager_set_state_callback(&manager, mock_state_change);
    screen_state_manager_set_app_enter_callback(&manager, mock_app_enter);
    
    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, TRANSITION_MANUAL, NULL);
    screen_state_manager_transition(&manager, SCREEN_STATE_APP, TRANSITION_MANUAL, "LoRa Chat");
    
    TEST_ASSERT_EQUAL(SCREEN_STATE_APP, screen_state_manager_get_state(&manager));
    TEST_ASSERT_EQUAL(SCREEN_STATE_MENU, screen_state_manager_get_previous_state(&manager));
    TEST_ASSERT_EQUAL_INT(2, state_change_count);
    TEST_ASSERT_EQUAL_INT(1, app_enter_count);
}

void test_transition_app_to_menu(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    screen_state_manager_set_state_callback(&manager, mock_state_change);
    screen_state_manager_set_app_exit_callback(&manager, mock_app_exit);
    
    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, TRANSITION_MANUAL, NULL);
    screen_state_manager_transition(&manager, SCREEN_STATE_APP, TRANSITION_MANUAL, "Settings");
    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, TRANSITION_MANUAL, NULL);
    
    TEST_ASSERT_EQUAL(SCREEN_STATE_MENU, screen_state_manager_get_state(&manager));
    TEST_ASSERT_EQUAL_INT(2, app_exit_count);
}

void test_transition_no_duplicate_state(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, TRANSITION_MANUAL, NULL);
    int count_before = state_change_count;
    
    /* Try to transition to same state */
    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, TRANSITION_MANUAL, NULL);
    
    /* Should not have changed state or called callbacks */
    TEST_ASSERT_EQUAL(SCREEN_STATE_MENU, screen_state_manager_get_state(&manager));
    TEST_ASSERT_EQUAL_INT(count_before, state_change_count);
}

/* ================================================================
 *  HISTORY TESTS
 * ================================================================ */

void test_history_records_transitions(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    /* Perform several transitions */
    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, TRANSITION_MANUAL, NULL);
    screen_state_manager_transition(&manager, SCREEN_STATE_APP, TRANSITION_MANUAL, "GPS");
    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, TRANSITION_MANUAL, NULL);
    screen_state_manager_transition(&manager, SCREEN_STATE_IDLE, TRANSITION_AUTO, NULL);
    
    TEST_ASSERT_TRUE(screen_state_manager_has_history(&manager));
    TEST_ASSERT_EQUAL_UINT32(4, manager.history.depth);
}

void test_navigate_back_success(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, TRANSITION_MANUAL, NULL);
    screen_state_manager_transition(&manager, SCREEN_STATE_APP, TRANSITION_MANUAL, "Radio");
    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, TRANSITION_MANUAL, NULL);
    
    /* Navigate back */
    bool success = screen_state_manager_navigate_back(&manager);
    
    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_EQUAL(SCREEN_STATE_APP, screen_state_manager_get_state(&manager));
}

void test_navigate_back_no_history(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    /* No history yet */
    bool success = screen_state_manager_navigate_back(&manager);
    
    /* Should default to MENU */
    TEST_ASSERT_FALSE(success);
    TEST_ASSERT_EQUAL(SCREEN_STATE_MENU, screen_state_manager_get_state(&manager));
}

void test_clear_history(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, TRANSITION_MANUAL, NULL);
    screen_state_manager_transition(&manager, SCREEN_STATE_APP, TRANSITION_MANUAL, "Test");
    
    screen_state_manager_clear_history(&manager);
    
    TEST_ASSERT_FALSE(screen_state_manager_has_history(&manager));
    TEST_ASSERT_EQUAL_UINT32(0, manager.history.depth);
}

void test_history_disable_prevents_recording(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    screen_state_manager_set_history_enabled(&manager, false);
    
    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, TRANSITION_MANUAL, NULL);
    screen_state_manager_transition(&manager, SCREEN_STATE_APP, TRANSITION_MANUAL, "Test");
    
    TEST_ASSERT_FALSE(screen_state_manager_has_history(&manager));
}

/* ================================================================
 *  LOW POWER TESTS
 * ================================================================ */

void test_low_power_transition(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    screen_state_manager_set_state_callback(&manager, mock_state_change);
    screen_state_manager_set_low_power_enter_callback(&manager, mock_low_power_enter);
    screen_state_manager_set_low_power_exit_callback(&manager, mock_low_power_exit);
    
    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, TRANSITION_MANUAL, NULL);
    screen_state_manager_transition(&manager, SCREEN_STATE_LOW_POWER, TRANSITION_AUTO, NULL);
    
    TEST_ASSERT_EQUAL(SCREEN_STATE_LOW_POWER, screen_state_manager_get_state(&manager));
    TEST_ASSERT_EQUAL_INT(1, low_power_enter_count);
}

void test_exit_low_power(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    screen_state_manager_set_state_callback(&manager, mock_state_change);
    screen_state_manager_set_low_power_exit_callback(&manager, mock_low_power_exit);
    
    screen_state_manager_transition(&manager, SCREEN_STATE_IDLE, TRANSITION_MANUAL, NULL);
    screen_state_manager_transition(&manager, SCREEN_STATE_LOW_POWER, TRANSITION_AUTO, NULL);
    screen_state_manager_transition(&manager, SCREEN_STATE_IDLE, TRANSITION_MANUAL, NULL);
    
    TEST_ASSERT_EQUAL_INT(1, low_power_exit_count);
}

void test_is_low_power_true(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    screen_state_manager_transition(&manager, SCREEN_STATE_LOW_POWER, TRANSITION_AUTO, NULL);
    
    TEST_ASSERT_TRUE(screen_state_manager_is_low_power(&manager));
}

void test_is_low_power_false_for_idle(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    screen_state_manager_transition(&manager, SCREEN_STATE_IDLE, TRANSITION_MANUAL, NULL);
    
    TEST_ASSERT_FALSE(screen_state_manager_is_low_power(&manager));
}

/* ================================================================
 *  ACTIVITY/TIMEOUT TESTS
 * ================================================================ */

void test_update_activity_timestamp(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    screen_state_manager_update_activity(&manager, 1000);
    screen_state_manager_update_activity(&manager, 2000);
    
    TEST_ASSERT_EQUAL_UINT32(2000, manager.last_activity_tick);
}

void test_check_timeout_not_expired(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    screen_state_manager_set_timeout(&manager, 5000);
    screen_state_manager_update_activity(&manager, 1000);
    
    bool expired = screen_state_manager_check_timeout(&manager, 3000);
    TEST_ASSERT_FALSE(expired);
}

void test_check_timeout_expired(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    screen_state_manager_set_timeout(&manager, 2000);
    screen_state_manager_update_activity(&manager, 1000);
    
    bool expired = screen_state_manager_check_timeout(&manager, 5000);
    TEST_ASSERT_TRUE(expired);
}

void test_check_timeout_no_activity(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    /* Never called update_activity */
    bool expired = screen_state_manager_check_timeout(&manager, 1000);
    TEST_ASSERT_FALSE(expired);
}

void test_activity_wakes_from_low_power(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    screen_state_manager_transition(&manager, SCREEN_STATE_IDLE, TRANSITION_MANUAL, NULL);
    screen_state_manager_update_activity(&manager, 1000);
    screen_state_manager_transition(&manager, SCREEN_STATE_LOW_POWER, TRANSITION_AUTO, NULL);
    
    /* Activity should wake up */
    screen_state_manager_update_activity(&manager, 2000);
    
    TEST_ASSERT_FALSE(screen_state_manager_is_low_power(&manager));
}

/* ================================================================
 *  IS IN STATE TESTS
 * ================================================================ */

void test_is_in_app_true_for_app_state(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    screen_state_manager_transition(&manager, SCREEN_STATE_APP, TRANSITION_MANUAL, "Test");
    
    TEST_ASSERT_TRUE(screen_state_manager_is_in_app(&manager));
}

void test_is_in_app_true_for_app_nested(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    screen_state_manager_transition(&manager, SCREEN_STATE_APP_NESTED, TRANSITION_MANUAL, "Test");
    
    TEST_ASSERT_TRUE(screen_state_manager_is_in_app(&manager));
}

void test_is_in_app_false_for_menu(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, TRANSITION_MANUAL, NULL);
    
    TEST_ASSERT_FALSE(screen_state_manager_is_in_app(&manager));
}

void test_is_in_state_menu(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, TRANSITION_MANUAL, NULL);
    
    TEST_ASSERT_TRUE(screen_state_manager_is_in_state(&manager, SCREEN_STATE_MENU));
    TEST_ASSERT_FALSE(screen_state_manager_is_in_state(&manager, SCREEN_STATE_APP));
}

/* ================================================================
 *  NESTED APP STATE TESTS
 * ================================================================ */

void test_transition_app_to_app_nested(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    screen_state_manager_transition(&manager, SCREEN_STATE_APP, TRANSITION_MANUAL, "Settings");
    screen_state_manager_transition(&manager, SCREEN_STATE_APP_NESTED, TRANSITION_MANUAL, NULL);
    
    TEST_ASSERT_EQUAL(SCREEN_STATE_APP_NESTED, screen_state_manager_get_state(&manager));
    TEST_ASSERT_TRUE(screen_state_manager_is_in_app(&manager));
}

/* ================================================================
 *  LAST TRANSITION TYPE TESTS
 * ================================================================ */

void test_get_last_transition_manual(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, TRANSITION_MANUAL, NULL);
    
    TEST_ASSERT_EQUAL(TRANSITION_MANUAL, screen_state_manager_get_last_transition(&manager));
}

void test_get_last_transition_auto(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    screen_state_manager_transition(&manager, SCREEN_STATE_LOW_POWER, TRANSITION_AUTO, NULL);
    
    TEST_ASSERT_EQUAL(TRANSITION_AUTO, screen_state_manager_get_last_transition(&manager));
}

void test_get_last_transition_count_on_empty_history(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    TEST_ASSERT_EQUAL(TRANSITION_COUNT, screen_state_manager_get_last_transition(&manager));
}

/* ================================================================
 *  CALLBACK ENABLEMENT TESTS
 * ================================================================ */

void test_disable_callbacks_prevents_calls(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    screen_state_manager_set_state_callback(&manager, mock_state_change);
    screen_state_manager_set_callbacks_enabled(&manager, false);
    
    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, TRANSITION_MANUAL, NULL);
    
    TEST_ASSERT_EQUAL_INT(0, state_change_count);
}

void test_enable_callbacks_resumes_calls(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager);
    
    screen_state_manager_set_state_callback(&manager, mock_state_change);
    screen_state_manager_set_callbacks_enabled(&manager, false);
    
    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, TRANSITION_MANUAL, NULL);
    
    /* Enable callbacks */
    screen_state_manager_set_callbacks_enabled(&manager, true);
    screen_state_manager_transition(&manager, SCREEN_STATE_APP, TRANSITION_MANUAL, "Test");
    
    TEST_ASSERT_EQUAL_INT(1, state_change_count);
}

/* ================================================================
 *  NULL POINTER TESTS
 * ================================================================ */

void test_null_manager_does_not_crash(void)
{
    ScreenStateManager_t *manager = NULL;
    
    /* These should not crash even with NULL manager */
    ScreenStateManager_t dummy;
    memset(&dummy, 0, sizeof(dummy));
    
    /* Test functions that accept NULL */
    TEST_ASSERT_EQUAL(SCREEN_STATE_COUNT, screen_state_manager_get_state(manager));
    TEST_ASSERT_EQUAL(SCREEN_STATE_IDLE, screen_state_manager_get_previous_state(manager));
    TEST_ASSERT_FALSE(screen_state_manager_is_low_power(manager));
    
    /* Init with NULL should be safe */
    screen_state_manager_init(NULL);
}

/* ================================================================
 *  UTILITY FUNCTION TESTS
 * ================================================================ */

void test_get_state_name_valid_states(void)
{
    TEST_ASSERT_EQUAL_STRING("IDLE", screen_state_manager_get_state_name(SCREEN_STATE_IDLE));
    TEST_ASSERT_EQUAL_STRING("MENU", screen_state_manager_get_state_name(SCREEN_STATE_MENU));
    TEST_ASSERT_EQUAL_STRING("APP", screen_state_manager_get_state_name(SCREEN_STATE_APP));
    TEST_ASSERT_EQUAL_STRING("UNKNOWN", screen_state_manager_get_state_name(SCREEN_STATE_COUNT));
}

void test_get_transition_name_valid_types(void)
{
    TEST_ASSERT_EQUAL_STRING("MANUAL", screen_state_manager_get_transition_name(TRANSITION_MANUAL));
    TEST_ASSERT_EQUAL_STRING("AUTO", screen_state_manager_get_transition_name(TRANSITION_AUTO));
    TEST_ASSERT_EQUAL_STRING("UNKNOWN", screen_state_manager_get_transition_name(TRANSITION_COUNT));
}

/* ================================================================
 *  MAIN
 * ================================================================ */

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    
    /* Initialization tests */
    RUN_TEST(test_init_sets_default_state);
    RUN_TEST(test_init_clears_history);
    RUN_TEST(test_init_default_timeout);
    
    /* State transition tests */
    RUN_TEST(test_transition_idle_to_menu);
    RUN_TEST(test_transition_menu_to_app);
    RUN_TEST(test_transition_app_to_menu);
    RUN_TEST(test_transition_no_duplicate_state);
    
    /* History tests */
    RUN_TEST(test_history_records_transitions);
    RUN_TEST(test_navigate_back_success);
    RUN_TEST(test_navigate_back_no_history);
    RUN_TEST(test_clear_history);
    RUN_TEST(test_history_disable_prevents_recording);
    
    /* Low power tests */
    RUN_TEST(test_low_power_transition);
    RUN_TEST(test_exit_low_power);
    RUN_TEST(test_is_low_power_true);
    RUN_TEST(test_is_low_power_false_for_idle);
    
    /* Activity/timeout tests */
    RUN_TEST(test_update_activity_timestamp);
    RUN_TEST(test_check_timeout_not_expired);
    RUN_TEST(test_check_timeout_expired);
    RUN_TEST(test_check_timeout_no_activity);
    RUN_TEST(test_activity_wakes_from_low_power);
    
    /* Is in state tests */
    RUN_TEST(test_is_in_app_true_for_app_state);
    RUN_TEST(test_is_in_app_true_for_app_nested);
    RUN_TEST(test_is_in_app_false_for_menu);
    RUN_TEST(test_is_in_state_menu);
    
    /* Nested app state tests */
    RUN_TEST(test_transition_app_to_app_nested);
    
    /* Last transition type tests */
    RUN_TEST(test_get_last_transition_manual);
    RUN_TEST(test_get_last_transition_auto);
    RUN_TEST(test_get_last_transition_count_on_empty_history);
    
    /* Callback enablement tests */
    RUN_TEST(test_disable_callbacks_prevents_calls);
    RUN_TEST(test_enable_callbacks_resumes_calls);
    
    /* Null pointer tests */
    RUN_TEST(test_null_manager_does_not_crash);
    
    /* Utility function tests */
    RUN_TEST(test_get_state_name_valid_states);
    RUN_TEST(test_get_transition_name_valid_types);
    
    return UNITY_END();
}
