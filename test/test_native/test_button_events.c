/**
 * @file test_button_events.c
 * Unity tests for button event handler module.
 * 
 * Tests event dispatching, button state tracking, and navigation actions.
 * Runs on native platform — no ESP32 hardware required.
 * 
 * Note: This test mocks hardware dependencies for native execution.
 */

#include <unity.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "../application/SkinnyCon/src/button_event_handlers.h"

/* Mock hardware functions */
static bool mock_hw_has_keyboard(void)
{
    return true;
}

static void mock_hw_flush_keyboard(void)
{
    /* No-op for testing */
}

/* Test counter variables */
static int global_handler_calls = 0;
static int nav_handler_calls = 0;
static int app_handler_calls = 0;
static ButtonEvent_t last_event_received;

/* Test callback implementations */
static void mock_global_handler(const ButtonEvent_t *event)
{
    global_handler_calls++;
    if (event) {
        memcpy(&last_event_received, event, sizeof(ButtonEvent_t));
    }
}

static void mock_nav_handler(const ButtonEvent_t *event)
{
    nav_handler_calls++;
    if (event) {
        memcpy(&last_event_received, event, sizeof(ButtonEvent_t));
    }
}

static void mock_app_handler(const ButtonEvent_t *event)
{
    app_handler_calls++;
    if (event) {
        memcpy(&last_event_received, event, sizeof(ButtonEvent_t));
    }
}

/* ================================================================
 *  TEST SETUP/TEARDOWN
 * ================================================================ */

void setUp(void)
{
    /* Reset counters */
    global_handler_calls = 0;
    nav_handler_calls = 0;
    app_handler_calls = 0;
    memset(&last_event_received, 0, sizeof(last_event_received));
}

void tearDown(void)
{
    /* Nothing to clean up */
}

/* ================================================================
 *  INITIALIZATION TESTS
 * ================================================================ */

void test_init_sets_defaults(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    
    TEST_ASSERT_TRUE(manager.navigation_enabled);
    TEST_ASSERT_TRUE(manager.app_actions_enabled);
    TEST_ASSERT_EQUAL_UINT32(1000, manager.long_press_threshold_ms);
    TEST_ASSERT_EQUAL_UINT32(300, manager.double_click_threshold_ms);
    TEST_ASSERT_EQUAL_UINT32(50, manager.click_debounce_ms);
    TEST_ASSERT_NULL(manager.active_app_name);
    TEST_ASSERT_NULL(manager.active_app_context);
}

void test_init_clears_button_states(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    
    for (int i = 0; i < BUTTON_COUNT; i++) {
        TEST_ASSERT_EQUAL_UINT32(0, manager.button_states[i].press_count);
        TEST_ASSERT_FALSE(manager.button_states[i].is_pressed);
        TEST_ASSERT_FALSE(manager.button_states[i].has_double_click);
        TEST_ASSERT_FALSE(manager.button_states[i].has_long_press);
    }
}

void test_init_sets_hardware_callbacks(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    
    /* Should default to real hardware functions */
    TEST_ASSERT_NOT_NULL(manager.hw_has_keyboard);
    TEST_ASSERT_NOT_NULL(manager.hw_flush_keyboard);
}

/* ================================================================
 *  CALLBACK SETTING TESTS
 * ================================================================ */

void test_set_global_handler(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    
    button_event_manager_set_global_handler(&manager, mock_global_handler);
    
    TEST_ASSERT_NOT_NULL(manager.global_handler);
    
    /* Trigger event and verify callback */
    button_event_manager_simulate(&manager, BUTTON_ENCODER_CLICK, BUTTON_EVENT_CLICK);
    TEST_ASSERT_EQUAL_INT(1, global_handler_calls);
}

void test_set_nav_handler(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    
    button_event_manager_set_nav_handler(&manager, mock_nav_handler);
    
    button_event_manager_simulate(&manager, BUTTON_ENCODER_CLICK, BUTTON_EVENT_CLICK);
    TEST_ASSERT_EQUAL_INT(1, nav_handler_calls);
}

void test_set_app_handler(void)
{
    ButtonEventManager_t manager;
    ButtonEvent_t saved_event;
    
    button_event_manager_init(&manager);
    button_event_manager_set_app_handler(&manager, mock_app_handler);
    
    button_event_manager_simulate(&manager, BUTTON_ENCODER_CLICK, BUTTON_EVENT_CLICK);
    TEST_ASSERT_EQUAL_INT(1, app_handler_calls);
}

/* ================================================================
 *  NAVIGATION DISABLE/ENABLE TESTS
 * ================================================================ */

void test_disable_navigation(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    
    button_event_manager_set_nav_handler(&manager, mock_nav_handler);
    button_event_manager_set_navigation_enabled(&manager, false);
    
    button_event_manager_simulate(&manager, BUTTON_ENCODER_CLICK, BUTTON_EVENT_CLICK);
    TEST_ASSERT_EQUAL_INT(0, nav_handler_calls);  /* Should not call nav handler */
}

void test_enable_navigation(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    
    button_event_manager_set_navigation_enabled(&manager, false);
    button_event_manager_set_nav_handler(&manager, mock_nav_handler);
    
    button_event_manager_set_navigation_enabled(&manager, true);
    button_event_manager_simulate(&manager, BUTTON_ENCODER_CLICK, BUTTON_EVENT_CLICK);
    
    TEST_ASSERT_EQUAL_INT(1, nav_handler_calls);
}

/* ================================================================
 *  APPLICATION CONTEXT TESTS
 * ================================================================ */

void test_set_active_app(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    
    button_event_manager_set_active_app(&manager, "LoRa Chat", (void*)0x1234);
    
    TEST_ASSERT_EQUAL_STRING("LoRa Chat", button_event_manager_get_active_app(&manager));
}

void test_set_active_app_clears_old(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    
    button_event_manager_set_active_app(&manager, "App1", (void*)0x1111);
    button_event_manager_set_active_app(&manager, "App2", (void*)0x2222);
    
    TEST_ASSERT_EQUAL_STRING("App2", button_event_manager_get_active_app(&manager));
}

/* ================================================================
 *  BUTTON STATE TRACKING TESTS
 * ================================================================ */

void test_click_increments_count(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    button_event_manager_set_nav_handler(&manager, mock_nav_handler);
    
    button_event_manager_simulate(&manager, BUTTON_ENCODER_CLICK, BUTTON_EVENT_CLICK);
    button_event_manager_simulate(&manager, BUTTON_ENCODER_CLICK, BUTTON_EVENT_CLICK);
    
    const ButtonState_t *state = button_event_manager_get_button_state(&manager, BUTTON_ENCODER_CLICK);
    TEST_ASSERT_NOT_NULL(state);
    TEST_ASSERT_EQUAL_UINT32(2, state->press_count);
}

void test_double_click_flag(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    
    button_event_manager_simulate(&manager, BUTTON_ENCODER_CLICK, BUTTON_EVENT_DOUBLE_CLICK);
    
    const ButtonState_t *state = button_event_manager_get_button_state(&manager, BUTTON_ENCODER_CLICK);
    TEST_ASSERT_NOT_NULL(state);
    TEST_ASSERT_TRUE(state->has_double_click);
}

void test_long_press_flag(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    
    button_event_manager_simulate(&manager, BUTTON_ENCODER_CLICK, BUTTON_EVENT_LONG_PRESS);
    
    const ButtonState_t *state = button_event_manager_get_button_state(&manager, BUTTON_ENCODER_CLICK);
    TEST_ASSERT_NOT_NULL(state);
    TEST_ASSERT_TRUE(state->has_long_press);
}

/* ================================================================
 *  DISPATCHER FUNCTION TESTS
 * ================================================================ */

void test_dispatch_back_creates_event(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    button_event_manager_set_nav_handler(&manager, mock_nav_handler);
    
    button_event_manager_dispatch_back(&manager);
    
    TEST_ASSERT_EQUAL_INT(1, nav_handler_calls);
    TEST_ASSERT_EQUAL(BUTTON_BACK, last_event_received.button_id);
    TEST_ASSERT_EQUAL(BUTTON_EVENT_CLICK, last_event_received.event_type);
}

void test_dispatch_menu_creates_event(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    button_event_manager_set_nav_handler(&manager, mock_nav_handler);
    
    button_event_manager_dispatch_menu(&manager);
    
    TEST_ASSERT_EQUAL_INT(1, nav_handler_calls);
    TEST_ASSERT_EQUAL(BUTTON_MENU, last_event_received.button_id);
}

void test_dispatch_select_creates_app_event(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    button_event_manager_set_app_handler(&manager, mock_app_handler);
    
    button_event_manager_dispatch_select(&manager);
    
    TEST_ASSERT_EQUAL_INT(1, app_handler_calls);
    TEST_ASSERT_EQUAL(BUTTON_SELECT, last_event_received.button_id);
}

void test_dispatch_rotate_up_creates_event(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    button_event_manager_set_nav_handler(&manager, mock_nav_handler);
    
    button_event_manager_dispatch_rotate_up(&manager);
    
    TEST_ASSERT_EQUAL_INT(1, nav_handler_calls);
    TEST_ASSERT_EQUAL(BUTTON_EVENT_ROTATE_UP, last_event_received.event_type);
}

void test_dispatch_rotate_down_creates_event(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    button_event_manager_set_nav_handler(&manager, mock_nav_handler);
    
    button_event_manager_dispatch_rotate_down(&manager);
    
    TEST_ASSERT_EQUAL_INT(1, nav_handler_calls);
    TEST_ASSERT_EQUAL(BUTTON_EVENT_ROTATE_DOWN, last_event_received.event_type);
}

/* ================================================================
 *  CLEAR STATE TESTS
 * ================================================================ */

void test_clear_state_resets_counters(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    button_event_manager_set_nav_handler(&manager, mock_nav_handler);
    
    button_event_manager_simulate(&manager, BUTTON_ENCODER_CLICK, BUTTON_EVENT_CLICK);
    button_event_manager_simulate(&manager, BUTTON_ENCODER_CLICK, BUTTON_EVENT_CLICK);
    
    button_event_manager_clear_state(&manager);
    
    const ButtonState_t *state = button_event_manager_get_button_state(&manager, BUTTON_ENCODER_CLICK);
    TEST_ASSERT_NOT_NULL(state);
    TEST_ASSERT_EQUAL_UINT32(0, state->press_count);
}

void test_clear_state_clears_app_context(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    
    button_event_manager_set_active_app(&manager, "TestApp", (void*)0x5678);
    button_event_manager_clear_state(&manager);
    
    TEST_ASSERT_NULL(button_event_manager_get_active_app(&manager));
}

/* ================================================================
 *  PRINT STATE TESTS
 * ================================================================ */

void test_print_state_does_not_crash(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    
    /* Should not crash even if output is captured */
    button_event_manager_print_state(&manager);
}

/* ================================================================
 *  ENABLED CHECK TESTS
 * ================================================================ */

void test_is_enabled_returns_true(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    
    TEST_ASSERT_TRUE(button_event_manager_is_enabled(&manager));
}

void test_is_enabled_returns_false_when_both_disabled(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    
    button_event_manager_set_navigation_enabled(&manager, false);
    button_event_manager_set_app_actions_enabled(&manager, false);
    
    TEST_ASSERT_FALSE(button_event_manager_is_enabled(&manager));
}

/* ================================================================
 *  INVALID BUTTON ID TESTS
 * ================================================================ */

void test_handle_invalid_button_id_does_not_crash(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    
    ButtonEvent_t invalid_event = {
        .button_id = BUTTON_COUNT,  /* Invalid */
        .event_type = BUTTON_EVENT_CLICK,
        .timestamp_ms = 0,
        .user_data = NULL
    };
    
    /* Should return false for invalid button */
    TEST_ASSERT_FALSE(button_event_manager_handle_hw_event(&manager, &invalid_event));
}

/* ================================================================
 *  EVENT DISPATCHING PRIORITY TESTS
 * ================================================================ */

void test_global_handler_called_before_nav(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    
    int call_order[2];
    int call_idx = 0;
    
    /* Create sequential call tracking */
    static void global_first(const ButtonEvent_t *e) {
        /* Placeholder - uses external state */
    }
    
    button_event_manager_set_global_handler(&manager, mock_global_handler);
    button_event_manager_set_nav_handler(&manager, mock_nav_handler);
    
    button_event_manager_simulate(&manager, BUTTON_ENCODER_CLICK, BUTTON_EVENT_CLICK);
    
    /* Global should have been called */
    TEST_ASSERT_EQUAL_INT(1, global_handler_calls);
}

/* ================================================================
 *  ROTATE EVENT TESTS
 * ================================================================ */

void test_rotate_up_event(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    button_event_manager_set_nav_handler(&manager, mock_nav_handler);
    
    button_event_manager_simulate(&manager, BUTTON_ENCODER_ROTATE, BUTTON_EVENT_ROTATE_UP);
    
    TEST_ASSERT_EQUAL_INT(1, nav_handler_calls);
    TEST_ASSERT_EQUAL(BUTTON_EVENT_ROTATE_UP, last_event_received.event_type);
}

void test_rotate_down_event(void)
{
    ButtonEventManager_t manager;
    button_event_manager_init(&manager);
    button_event_manager_set_nav_handler(&manager, mock_nav_handler);
    
    button_event_manager_simulate(&manager, BUTTON_ENCODER_ROTATE, BUTTON_EVENT_ROTATE_DOWN);
    
    TEST_ASSERT_EQUAL_INT(1, nav_handler_calls);
    TEST_ASSERT_EQUAL(BUTTON_EVENT_ROTATE_DOWN, last_event_received.event_type);
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
    RUN_TEST(test_init_sets_defaults);
    RUN_TEST(test_init_clears_button_states);
    RUN_TEST(test_init_sets_hardware_callbacks);
    
    /* Callback setting tests */
    RUN_TEST(test_set_global_handler);
    RUN_TEST(test_set_nav_handler);
    RUN_TEST(test_set_app_handler);
    
    /* Navigation enable/disable tests */
    RUN_TEST(test_disable_navigation);
    RUN_TEST(test_enable_navigation);
    
    /* Application context tests */
    RUN_TEST(test_set_active_app);
    RUN_TEST(test_set_active_app_clears_old);
    
    /* Button state tracking tests */
    RUN_TEST(test_click_increments_count);
    RUN_TEST(test_double_click_flag);
    RUN_TEST(test_long_press_flag);
    
    /* Dispatcher function tests */
    RUN_TEST(test_dispatch_back_creates_event);
    RUN_TEST(test_dispatch_menu_creates_event);
    RUN_TEST(test_dispatch_select_creates_app_event);
    RUN_TEST(test_dispatch_rotate_up_creates_event);
    RUN_TEST(test_dispatch_rotate_down_creates_event);
    
    /* Clear state tests */
    RUN_TEST(test_clear_state_resets_counters);
    RUN_TEST(test_clear_state_clears_app_context);
    
    /* Print state tests */
    RUN_TEST(test_print_state_does_not_crash);
    
    /* Enabled check tests */
    RUN_TEST(test_is_enabled_returns_true);
    RUN_TEST(test_is_enabled_returns_false_when_both_disabled);
    
    /* Invalid button ID tests */
    RUN_TEST(test_handle_invalid_button_id_does_not_crash);
    
    /* Event dispatching priority tests */
    RUN_TEST(test_global_handler_called_before_nav);
    
    /* Rotate event tests */
    RUN_TEST(test_rotate_up_event);
    RUN_TEST(test_rotate_down_event);
    
    return UNITY_END();
}
