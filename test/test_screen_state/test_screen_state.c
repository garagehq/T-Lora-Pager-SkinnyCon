/**
 * @file      test_screen_state.c
 * @author    T-Lora-Pager-SkinnyCon Team
 * @license   MIT
 * @copyright Copyright (c) 2025  ShenZhen XinYuan Electronic Technology Co., Ltd
 * @date      2026-03-19
 *
 * @brief     Unity unit tests for screen state manager.
 *
 * Tests state transitions, navigation history, timeouts, and power management.
 * Runs on native platform -- no ESP32 hardware required.
 *
 * The library source is included directly because the native test environment
 * uses build_src_filter = -<*> (no source compilation).
 */

#include <unity.h>
#include <stdint.h>
#include <stdbool.h>

/* Include the library header and implementation directly */
#include "../../lib/screen_state/screen_state_manager.h"
#include "../../lib/screen_state/screen_state_manager.c"

/* ============================================================
 * MOCK CALLBACKS FOR TESTING
 * ============================================================ */

static int transition_callback_count = 0;
static ScreenState_t last_old_state = SCREEN_STATE_IDLE;
static ScreenState_t last_new_state = SCREEN_STATE_IDLE;

static void mock_transition_callback(ScreenState_t old_state,
                                     ScreenState_t new_state,
                                     void* user_data)
{
    (void)user_data;
    transition_callback_count++;
    last_old_state = old_state;
    last_new_state = new_state;
}

static int power_callback_count = 0;
static bool last_low_power_event = false;

static void mock_power_callback(bool low_power, void* user_data)
{
    (void)user_data;
    power_callback_count++;
    last_low_power_event = low_power;
}

/* ============================================================
 * TEST FIXTURE SETUP
 * ============================================================ */

void setUp(void)
{
    transition_callback_count = 0;
    power_callback_count = 0;
    last_old_state = SCREEN_STATE_IDLE;
    last_new_state = SCREEN_STATE_IDLE;
    last_low_power_event = false;
}

void tearDown(void)
{
}

/* ============================================================
 * STATE STRING CONVERSION TESTS
 * ============================================================ */

void test_state_to_string_idle(void)
{
    const char* str = screen_state_to_string(SCREEN_STATE_IDLE);
    TEST_ASSERT_NOT_NULL(str);
    TEST_ASSERT_EQUAL_STRING("IDLE", str);
}

void test_state_to_string_menu(void)
{
    TEST_ASSERT_EQUAL_STRING("MENU", screen_state_to_string(SCREEN_STATE_MENU));
}

void test_state_to_string_app(void)
{
    TEST_ASSERT_EQUAL_STRING("APP", screen_state_to_string(SCREEN_STATE_APP));
}

void test_state_to_string_app_nested(void)
{
    TEST_ASSERT_EQUAL_STRING("APP_NESTED",
                            screen_state_to_string(SCREEN_STATE_APP_NESTED));
}

void test_state_to_string_low_power(void)
{
    TEST_ASSERT_EQUAL_STRING("LOW_POWER",
                            screen_state_to_string(SCREEN_STATE_LOW_POWER));
}

void test_state_to_string_power_off(void)
{
    TEST_ASSERT_EQUAL_STRING("POWER_OFF",
                            screen_state_to_string(SCREEN_STATE_POWER_OFF));
}

void test_state_to_string_invalid(void)
{
    TEST_ASSERT_EQUAL_STRING("UNKNOWN",
                            screen_state_to_string((ScreenState_t)999));
}

void test_state_from_string_idle_uppercase(void)
{
    ScreenState_t state = screen_state_from_string("IDLE");
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_IDLE, state);
}

void test_state_from_string_idle_lowercase(void)
{
    ScreenState_t state = screen_state_from_string("idle");
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_IDLE, state);
}

void test_state_from_string_menu(void)
{
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_MENU,
                         screen_state_from_string("MENU"));
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_MENU,
                         screen_state_from_string("menu"));
}

void test_state_from_string_mixed_case(void)
{
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_MENU,
                         screen_state_from_string("Menu"));
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_APP,
                         screen_state_from_string("App"));
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_LOW_POWER,
                         screen_state_from_string("Low_Power"));
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_APP_NESTED,
                         screen_state_from_string("App_Nested"));
}

void test_state_from_string_null(void)
{
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_IDLE,
                         screen_state_from_string(NULL));
}

void test_state_from_string_invalid(void)
{
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_IDLE,
                         screen_state_from_string("INVALID_STATE"));
}

/* ============================================================
 * NAVIGATION HISTORY TESTS
 * ============================================================ */

void test_nav_history_init(void)
{
    NavigationHistory_t history;
    nav_history_init(&history);

    TEST_ASSERT_EQUAL_UINT8(0, history.size);
    TEST_ASSERT_EQUAL_UINT8(0, history.head);
    TEST_ASSERT_EQUAL_UINT8(0, history.tail);
}

void test_nav_history_push_pop_single(void)
{
    NavigationHistory_t history;
    nav_history_init(&history);

    TEST_ASSERT_TRUE(nav_history_push(&history, SCREEN_STATE_MENU, 1000, 0));
    TEST_ASSERT_EQUAL_UINT8(1, history.size);

    ScreenState_t popped_state;
    uint8_t popped_app;
    TEST_ASSERT_TRUE(nav_history_pop(&history, &popped_state, &popped_app));
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_MENU, popped_state);
    TEST_ASSERT_EQUAL_UINT8(0, popped_app);
    TEST_ASSERT_EQUAL_UINT8(0, history.size);
}

void test_nav_history_push_max_size(void)
{
    NavigationHistory_t history;
    nav_history_init(&history);

    for (uint8_t i = 0; i < NAV_HISTORY_MAX_SIZE; i++) {
        TEST_ASSERT_TRUE(nav_history_push(&history, (ScreenState_t)i,
                                          1000 * i, 0));
    }

    TEST_ASSERT_EQUAL_UINT8(NAV_HISTORY_MAX_SIZE, history.size);

    /* Next push succeeds by evicting the oldest entry */
    TEST_ASSERT_TRUE(nav_history_push(&history, SCREEN_STATE_APP, 20000, 0));
    TEST_ASSERT_EQUAL_UINT8(NAV_HISTORY_MAX_SIZE, history.size);
}

void test_nav_history_peek(void)
{
    NavigationHistory_t history;
    nav_history_init(&history);

    nav_history_push(&history, SCREEN_STATE_MENU, 1000, 0);
    nav_history_push(&history, SCREEN_STATE_APP, 2000, 5);

    ScreenState_t peek_state;
    uint8_t peek_app;
    TEST_ASSERT_TRUE(nav_history_peek(&history, &peek_state, &peek_app));
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_APP, peek_state);
    TEST_ASSERT_EQUAL_UINT8(5, peek_app);

    /* Peek should not modify size */
    TEST_ASSERT_EQUAL_UINT8(2, history.size);
}

void test_nav_history_clear(void)
{
    NavigationHistory_t history;
    nav_history_init(&history);

    for (int i = 0; i < 5; i++) {
        nav_history_push(&history, (ScreenState_t)i, 1000 * i, 0);
    }

    nav_history_clear(&history);
    TEST_ASSERT_EQUAL_UINT8(0, history.size);

    ScreenState_t dummy_state;
    uint8_t dummy_app;
    TEST_ASSERT_FALSE(nav_history_pop(&history, &dummy_state, &dummy_app));
}

void test_nav_history_wrap_around(void)
{
    NavigationHistory_t history;
    nav_history_init(&history);

    for (uint16_t i = 0; i < 20; i++) {
        nav_history_push(&history, SCREEN_STATE_IDLE + (i % 3), i * 100, 0);
    }

    TEST_ASSERT_LESS_OR_EQUAL(NAV_HISTORY_MAX_SIZE, history.size);

    uint8_t pop_count = 0;
    ScreenState_t state;
    uint8_t app;
    while (nav_history_pop(&history, &state, &app)) {
        pop_count++;
    }

    TEST_ASSERT_EQUAL_UINT8(NAV_HISTORY_MAX_SIZE, pop_count);
}

/* ============================================================
 * STATE MANAGER TESTS
 * ============================================================ */

void test_state_manager_init_default(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager, NULL);

    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_IDLE, manager.current_state);
    TEST_ASSERT_TRUE(manager.has_activity);
    TEST_ASSERT_EQUAL_UINT8(1, manager.history.size);  /* Initial idle state */
}

void test_state_manager_init_with_config(void)
{
    ScreenStateManager_t manager;
    ScreenStateManagerConfig_t config = {
        .on_state_transition = mock_transition_callback,
        .on_power_event = mock_power_callback,
        .user_data = (void*)0x1234,
        .idle_timeout_ms = 5000,
        .menu_timeout_ms = 10000,
        .app_timeout_ms = 20000
    };

    screen_state_manager_init(&manager, &config);

    TEST_ASSERT_EQUAL_UINT32(5000, manager.config.idle_timeout_ms);
    TEST_ASSERT_EQUAL_UINT32(10000, manager.config.menu_timeout_ms);
    TEST_ASSERT_EQUAL_UINT32(20000, manager.config.app_timeout_ms);
    TEST_ASSERT_NOT_NULL(manager.config.on_state_transition);
}

void test_state_manager_transition_idle_to_menu(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager, NULL);

    TEST_ASSERT_TRUE(screen_state_manager_transition(&manager,
                                                     SCREEN_STATE_MENU, 0));
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_MENU, manager.current_state);
    TEST_ASSERT_TRUE(manager.has_activity);
}

void test_state_manager_transition_menu_to_app(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager, NULL);
    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, 0);

    TEST_ASSERT_TRUE(screen_state_manager_transition(&manager,
                                                     SCREEN_STATE_APP, 5));
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_APP, manager.current_state);
}

void test_state_manager_transition_app_to_nested(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager, NULL);

    screen_state_manager_transition(&manager, SCREEN_STATE_APP, 5);

    TEST_ASSERT_TRUE(screen_state_manager_transition(&manager,
                                                     SCREEN_STATE_APP_NESTED, 0));
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_APP_NESTED, manager.current_state);

    /* History should contain previous states */
    TEST_ASSERT_EQUAL_UINT8(2, manager.history.size);
}

void test_state_manager_go_back_from_nested(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager, NULL);

    /* Navigate: IDLE -> APP -> APP_NESTED */
    screen_state_manager_transition(&manager, SCREEN_STATE_APP, 5);
    screen_state_manager_transition(&manager, SCREEN_STATE_APP_NESTED, 0);

    /* Go back should return to APP */
    TEST_ASSERT_TRUE(screen_state_manager_go_back(&manager));
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_APP, manager.current_state);

    /* Go back again should return to IDLE */
    TEST_ASSERT_TRUE(screen_state_manager_go_back(&manager));
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_IDLE, manager.current_state);
}

void test_state_manager_go_back_empty_history(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager, NULL);

    /* Already at initial state, go back should handle gracefully */
    TEST_ASSERT_FALSE(screen_state_manager_go_back(&manager));
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_IDLE, manager.current_state);
}

void test_state_manager_is_in_state(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager, NULL);

    TEST_ASSERT_TRUE(screen_state_manager_is_in_state(&manager, SCREEN_STATE_IDLE));
    TEST_ASSERT_FALSE(screen_state_manager_is_in_state(&manager, SCREEN_STATE_MENU));

    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, 0);
    TEST_ASSERT_TRUE(screen_state_manager_is_in_state(&manager, SCREEN_STATE_MENU));
}

void test_state_manager_is_in_app(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager, NULL);

    TEST_ASSERT_FALSE(screen_state_manager_is_in_app(&manager));

    screen_state_manager_transition(&manager, SCREEN_STATE_APP, 5);
    TEST_ASSERT_TRUE(screen_state_manager_is_in_app(&manager));

    screen_state_manager_transition(&manager, SCREEN_STATE_APP_NESTED, 0);
    TEST_ASSERT_TRUE(screen_state_manager_is_in_app(&manager));

    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, 0);
    TEST_ASSERT_FALSE(screen_state_manager_is_in_app(&manager));
}

void test_state_manager_is_inactive(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager, NULL);

    TEST_ASSERT_TRUE(screen_state_manager_is_inactive(&manager));

    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, 0);
    TEST_ASSERT_TRUE(screen_state_manager_is_inactive(&manager));

    screen_state_manager_transition(&manager, SCREEN_STATE_APP, 5);
    TEST_ASSERT_FALSE(screen_state_manager_is_inactive(&manager));
}

void test_state_manager_update_activity(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager, NULL);

    manager.has_activity = false;
    manager.last_activity_ms = 0;

    screen_state_manager_update_activity(&manager, 50000);

    TEST_ASSERT_TRUE(manager.has_activity);
    TEST_ASSERT_EQUAL_UINT32(50000, manager.last_activity_ms);
}

void test_state_manager_check_timeout_no_activity(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager, NULL);

    manager.has_activity = false;

    TEST_ASSERT_TRUE(screen_state_manager_check_idle_timeout(&manager, 1000));
}

void test_state_manager_check_timeout_within_limit(void)
{
    ScreenStateManager_t manager;
    ScreenStateManagerConfig_t config = {
        .idle_timeout_ms = 10000
    };
    screen_state_manager_init(&manager, &config);

    screen_state_manager_update_activity(&manager, 50000);

    /* Within timeout */
    TEST_ASSERT_FALSE(screen_state_manager_check_idle_timeout(&manager, 55000));

    /* At timeout boundary */
    TEST_ASSERT_FALSE(screen_state_manager_check_idle_timeout(&manager, 60000));

    /* Past timeout */
    TEST_ASSERT_TRUE(screen_state_manager_check_idle_timeout(&manager, 60001));
}

void test_state_manager_check_timeout_with_wraparound(void)
{
    ScreenStateManager_t manager;
    ScreenStateManagerConfig_t config = {
        .idle_timeout_ms = 1000
    };
    screen_state_manager_init(&manager, &config);

    screen_state_manager_update_activity(&manager, 0xFFFFFFFF - 500);

    /* After wraparound, timeout should be detected */
    TEST_ASSERT_TRUE(screen_state_manager_check_idle_timeout(&manager, 500));
}

/* ============================================================
 * POWER MANAGEMENT TESTS
 * ============================================================ */

void test_enter_low_power_mode(void)
{
    ScreenStateManager_t manager;
    ScreenStateManagerConfig_t config = {
        .on_power_event = mock_power_callback
    };
    screen_state_manager_init(&manager, &config);

    screen_state_manager_transition(&manager, SCREEN_STATE_APP, 5);

    TEST_ASSERT_TRUE(screen_state_manager_enter_low_power(&manager));
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_LOW_POWER, manager.current_state);
    TEST_ASSERT_FALSE(manager.has_activity);

    TEST_ASSERT_EQUAL_UINT8(1, power_callback_count);
    TEST_ASSERT_TRUE(last_low_power_event);
}

void test_wake_from_low_power_to_idle(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager, NULL);

    screen_state_manager_enter_low_power(&manager);
    TEST_ASSERT_TRUE(screen_state_manager_wake_from_low_power(&manager, false));

    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_IDLE, manager.current_state);
}

void test_wake_from_low_power_to_menu(void)
{
    ScreenStateManager_t manager;
    ScreenStateManagerConfig_t config = {
        .on_power_event = mock_power_callback
    };
    screen_state_manager_init(&manager, &config);

    screen_state_manager_enter_low_power(&manager);
    TEST_ASSERT_TRUE(screen_state_manager_wake_from_low_power(&manager, true));

    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_MENU, manager.current_state);
}

void test_shutdown(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager, NULL);

    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, 0);

    TEST_ASSERT_TRUE(screen_state_manager_shutdown(&manager));
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_POWER_OFF, manager.current_state);
    TEST_ASSERT_FALSE(manager.has_activity);
}

void test_is_powered_down(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager, NULL);

    TEST_ASSERT_FALSE(screen_state_manager_is_powered_down(&manager));

    screen_state_manager_enter_low_power(&manager);
    TEST_ASSERT_TRUE(screen_state_manager_is_powered_down(&manager));

    screen_state_manager_shutdown(&manager);
    TEST_ASSERT_TRUE(screen_state_manager_is_powered_down(&manager));
}

void test_transition_callback(void)
{
    ScreenStateManager_t manager;
    ScreenStateManagerConfig_t config = {
        .on_state_transition = mock_transition_callback
    };
    screen_state_manager_init(&manager, &config);

    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, 0);

    TEST_ASSERT_EQUAL_UINT8(1, transition_callback_count);
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_IDLE, last_old_state);
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_MENU, last_new_state);

    screen_state_manager_transition(&manager, SCREEN_STATE_APP, 5);

    TEST_ASSERT_EQUAL_UINT8(2, transition_callback_count);
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_MENU, last_old_state);
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_APP, last_new_state);
}

void test_multiple_transitions(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager, NULL);

    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, 0);
    screen_state_manager_transition(&manager, SCREEN_STATE_APP, 5);
    screen_state_manager_transition(&manager, SCREEN_STATE_APP_NESTED, 0);
    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, 0);

    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_MENU, manager.current_state);
    TEST_ASSERT_EQUAL_UINT8(2, manager.history.size);
}

void test_state_manager_get_state(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager, NULL);

    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_IDLE,
                         screen_state_manager_get_state(&manager));

    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, 0);
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_MENU,
                         screen_state_manager_get_state(&manager));
}

void test_state_manager_get_history_size(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager, NULL);

    TEST_ASSERT_EQUAL_UINT8(1, screen_state_manager_get_history_size(&manager));

    screen_state_manager_transition(&manager, SCREEN_STATE_APP, 5);
    screen_state_manager_transition(&manager, SCREEN_STATE_APP_NESTED, 0);

    TEST_ASSERT_EQUAL_UINT8(2, screen_state_manager_get_history_size(&manager));
}

/* ============================================================
 * EDGE CASES AND ERROR HANDLING
 * ============================================================ */

void test_state_manager_null_pointer(void)
{
    TEST_ASSERT_FALSE(screen_state_manager_transition(NULL, SCREEN_STATE_MENU, 0));
    TEST_ASSERT_FALSE(screen_state_manager_go_back(NULL));
    TEST_ASSERT_FALSE(screen_state_manager_is_in_state(NULL, SCREEN_STATE_IDLE));
    TEST_ASSERT_FALSE(screen_state_manager_is_in_app(NULL));
    TEST_ASSERT_FALSE(screen_state_manager_is_inactive(NULL));
}

void test_nav_history_null_pointer(void)
{
    nav_history_init(NULL);  /* Should not crash */
    TEST_ASSERT_FALSE(nav_history_push(NULL, SCREEN_STATE_IDLE, 0, 0));
    TEST_ASSERT_FALSE(nav_history_pop(NULL, NULL, NULL));
    TEST_ASSERT_FALSE(nav_history_peek(NULL, NULL, NULL));
}

void test_state_change_to_same_state(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager, NULL);

    TEST_ASSERT_TRUE(screen_state_manager_transition(&manager,
                                                     SCREEN_STATE_IDLE, 0));
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_IDLE, manager.current_state);
}

void test_nested_app_navigation(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager, NULL);

    screen_state_manager_transition(&manager, SCREEN_STATE_APP, 5);
    screen_state_manager_transition(&manager, SCREEN_STATE_APP_NESTED, 0);

    TEST_ASSERT_EQUAL_UINT8(2, manager.history.size);

    screen_state_manager_go_back(&manager);
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_APP, manager.current_state);
}

/* ============================================================
 * MAIN
 * ============================================================ */

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    UNITY_BEGIN();

    /* State string conversion tests */
    RUN_TEST(test_state_to_string_idle);
    RUN_TEST(test_state_to_string_menu);
    RUN_TEST(test_state_to_string_app);
    RUN_TEST(test_state_to_string_app_nested);
    RUN_TEST(test_state_to_string_low_power);
    RUN_TEST(test_state_to_string_power_off);
    RUN_TEST(test_state_to_string_invalid);
    RUN_TEST(test_state_from_string_idle_uppercase);
    RUN_TEST(test_state_from_string_idle_lowercase);
    RUN_TEST(test_state_from_string_menu);
    RUN_TEST(test_state_from_string_mixed_case);
    RUN_TEST(test_state_from_string_null);
    RUN_TEST(test_state_from_string_invalid);

    /* Navigation history tests */
    RUN_TEST(test_nav_history_init);
    RUN_TEST(test_nav_history_push_pop_single);
    RUN_TEST(test_nav_history_push_max_size);
    RUN_TEST(test_nav_history_peek);
    RUN_TEST(test_nav_history_clear);
    RUN_TEST(test_nav_history_wrap_around);

    /* State manager initialization tests */
    RUN_TEST(test_state_manager_init_default);
    RUN_TEST(test_state_manager_init_with_config);
    RUN_TEST(test_transition_callback);

    /* State transition tests */
    RUN_TEST(test_state_manager_transition_idle_to_menu);
    RUN_TEST(test_state_manager_transition_menu_to_app);
    RUN_TEST(test_state_manager_transition_app_to_nested);
    RUN_TEST(test_state_manager_go_back_from_nested);
    RUN_TEST(test_state_manager_go_back_empty_history);
    RUN_TEST(test_state_manager_is_in_state);
    RUN_TEST(test_state_manager_is_in_app);
    RUN_TEST(test_state_manager_is_inactive);
    RUN_TEST(test_state_manager_update_activity);
    RUN_TEST(test_state_manager_check_timeout_no_activity);
    RUN_TEST(test_state_manager_check_timeout_within_limit);
    RUN_TEST(test_state_manager_check_timeout_with_wraparound);
    RUN_TEST(test_multiple_transitions);
    RUN_TEST(test_nested_app_navigation);
    RUN_TEST(test_state_change_to_same_state);

    /* Power management tests */
    RUN_TEST(test_enter_low_power_mode);
    RUN_TEST(test_wake_from_low_power_to_idle);
    RUN_TEST(test_wake_from_low_power_to_menu);
    RUN_TEST(test_shutdown);
    RUN_TEST(test_is_powered_down);

    /* Utility tests */
    RUN_TEST(test_state_manager_get_state);
    RUN_TEST(test_state_manager_get_history_size);

    /* Edge case tests */
    RUN_TEST(test_state_manager_null_pointer);
    RUN_TEST(test_nav_history_null_pointer);

    return UNITY_END();
}
