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
 * Runs on native platform — no ESP32 hardware required.
 *
 * NOTE: The screen_state_manager.c implementation is included inline here to
 * enable standalone testing without PlatformIO source file linking complexities.
 */

#include <unity.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/* ============================================================
 * INCLUDE SCREEN_STATE_MANAGER IMPLEMENTATION INLINE
 * ============================================================ */

/* Start of screen_state_manager.h (declarations) */
typedef enum {
    SCREEN_STATE_IDLE,
    SCREEN_STATE_MENU,
    SCREEN_STATE_APP,
    SCREEN_STATE_APP_NESTED,
    SCREEN_STATE_LOW_POWER,
    SCREEN_STATE_POWER_OFF,
    SCREEN_STATE_COUNT
} ScreenState_t;

const char* screen_state_to_string(ScreenState_t state);
ScreenState_t screen_state_from_string(const char* str);

#define NAV_HISTORY_MAX_SIZE  10

typedef struct {
    ScreenState_t state;
    uint32_t timestamp_ms;
    uint8_t app_id;
} NavHistoryRecord_t;

typedef struct {
    NavHistoryRecord_t records[NAV_HISTORY_MAX_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t size;
} NavigationHistory_t;

void nav_history_init(NavigationHistory_t* history);
bool nav_history_push(NavigationHistory_t* history, ScreenState_t state, uint32_t timestamp_ms, uint8_t app_id);
bool nav_history_pop(NavigationHistory_t* history, ScreenState_t* out_state, uint8_t* out_app_id);
bool nav_history_peek(const NavigationHistory_t* history, ScreenState_t* out_state, uint8_t* out_app_id);
void nav_history_clear(NavigationHistory_t* history);
uint8_t nav_history_get_size(const NavigationHistory_t* history);

typedef void (*ScreenStateTransitionCallback_t)(ScreenState_t old_state, ScreenState_t new_state, void* user_data);
typedef void (*PowerEventCallback_t)(bool low_power, void* user_data);

typedef struct {
    ScreenStateTransitionCallback_t on_state_transition;
    PowerEventCallback_t on_power_event;
    void* user_data;
    uint32_t idle_timeout_ms;
    uint32_t menu_timeout_ms;
    uint32_t app_timeout_ms;
} ScreenStateManagerConfig_t;

typedef struct {
    ScreenState_t current_state;
    NavigationHistory_t history;
    ScreenStateManagerConfig_t config;
    bool has_activity;
    uint32_t last_activity_ms;
    bool power_transition_pending;
} ScreenStateManager_t;

void screen_state_manager_init(ScreenStateManager_t* manager, const ScreenStateManagerConfig_t* config);
void screen_state_manager_set_config(ScreenStateManager_t* manager, const ScreenStateManagerConfig_t* config);
bool screen_state_manager_transition(ScreenStateManager_t* manager, ScreenState_t new_state, uint8_t app_id);
bool screen_state_manager_go_back(ScreenStateManager_t* manager);
bool screen_state_manager_is_in_state(const ScreenStateManager_t* manager, ScreenState_t state);
bool screen_state_manager_is_in_app(const ScreenStateManager_t* manager);
bool screen_state_manager_is_inactive(const ScreenStateManager_t* manager);
void screen_state_manager_update_activity(ScreenStateManager_t* manager, uint32_t timestamp_ms);
bool screen_state_manager_check_idle_timeout(const ScreenStateManager_t* manager, uint32_t current_ms);
ScreenState_t screen_state_manager_get_state(const ScreenStateManager_t* manager);
uint8_t screen_state_manager_get_history_size(const ScreenStateManager_t* manager);
bool screen_state_manager_enter_low_power(ScreenStateManager_t* manager);
bool screen_state_manager_wake_from_low_power(ScreenStateManager_t* manager, bool return_to_menu);
bool screen_state_manager_shutdown(ScreenStateManager_t* manager);
bool screen_state_manager_is_powered_down(const ScreenStateManager_t* manager);
/* End of declarations */

/* Start of screen_state_manager.c (implementation) */
#include <string.h>

const char* screen_state_to_string(ScreenState_t state)
{
    switch (state) {
        case SCREEN_STATE_IDLE:        return "IDLE";
        case SCREEN_STATE_MENU:        return "MENU";
        case SCREEN_STATE_APP:         return "APP";
        case SCREEN_STATE_APP_NESTED:  return "APP_NESTED";
        case SCREEN_STATE_LOW_POWER:   return "LOW_POWER";
        case SCREEN_STATE_POWER_OFF:   return "POWER_OFF";
        default:                       return "UNKNOWN";
    }
}

ScreenState_t screen_state_from_string(const char* str)
{
    if (!str) return SCREEN_STATE_IDLE;

    if (strcmp(str, "IDLE") == 0 || strcmp(str, "idle") == 0) return SCREEN_STATE_IDLE;
    if (strcmp(str, "MENU") == 0 || strcmp(str, "menu") == 0) return SCREEN_STATE_MENU;
    if (strcmp(str, "APP") == 0 || strcmp(str, "app") == 0) return SCREEN_STATE_APP;
    if (strcmp(str, "APP_NESTED") == 0 || strcmp(str, "app_nested") == 0) return SCREEN_STATE_APP_NESTED;
    if (strcmp(str, "LOW_POWER") == 0 || strcmp(str, "low_power") == 0) return SCREEN_STATE_LOW_POWER;
    if (strcmp(str, "POWER_OFF") == 0 || strcmp(str, "power_off") == 0) return SCREEN_STATE_POWER_OFF;

    return SCREEN_STATE_IDLE;
}

void nav_history_init(NavigationHistory_t* history)
{
    if (!history) return;
    memset(history, 0, sizeof(NavigationHistory_t));
    history->head = 0;
    history->tail = 0;
    history->size = 0;
}

static void history_inc_index(uint8_t* index, uint8_t max_size)
{
    *index = (*index + 1) % max_size;
}

bool nav_history_push(NavigationHistory_t* history, ScreenState_t state, uint32_t timestamp_ms, uint8_t app_id)
{
    if (!history) return false;

    if (history->size >= NAV_HISTORY_MAX_SIZE) {
        history_inc_index(&history->tail, NAV_HISTORY_MAX_SIZE);
        history->size--;
    }

    uint8_t write_pos = history->head;
    history->records[write_pos].state = state;
    history->records[write_pos].timestamp_ms = timestamp_ms;
    history->records[write_pos].app_id = app_id;
    history_inc_index(&history->head, NAV_HISTORY_MAX_SIZE);
    history->size++;

    return true;
}

bool nav_history_pop(NavigationHistory_t* history, ScreenState_t* out_state, uint8_t* out_app_id)
{
    if (!history || !out_state || !out_app_id) return false;
    if (history->size == 0) return false;

    uint8_t read_pos;
    if (history->head == 0) {
        read_pos = NAV_HISTORY_MAX_SIZE - 1;
    } else {
        read_pos = history->head - 1;
    }

    *out_state = history->records[read_pos].state;
    *out_app_id = history->records[read_pos].app_id;
    history->head = read_pos;
    history->size--;

    return true;
}

bool nav_history_peek(const NavigationHistory_t* history, ScreenState_t* out_state, uint8_t* out_app_id)
{
    if (!history || !out_state || !out_app_id) return false;
    if (history->size == 0) return false;

    uint8_t peek_pos;
    if (history->head == 0) {
        peek_pos = NAV_HISTORY_MAX_SIZE - 1;
    } else {
        peek_pos = history->head - 1;
    }

    *out_state = history->records[peek_pos].state;
    *out_app_id = history->records[peek_pos].app_id;
    return true;
}

void nav_history_clear(NavigationHistory_t* history)
{
    if (!history) return;
    memset(history, 0, sizeof(NavigationHistory_t));
    history->head = 0;
    history->tail = 0;
    history->size = 0;
}

uint8_t nav_history_get_size(const NavigationHistory_t* history)
{
    if (!history) return 0;
    return history->size;
}

void screen_state_manager_init(ScreenStateManager_t* manager, const ScreenStateManagerConfig_t* config)
{
    if (!manager) return;

    manager->current_state = SCREEN_STATE_IDLE;
    nav_history_init(&manager->history);

    if (config) {
        memcpy(&manager->config, config, sizeof(ScreenStateManagerConfig_t));
    } else {
        manager->config.idle_timeout_ms = 30000;
        manager->config.menu_timeout_ms = 60000;
        manager->config.app_timeout_ms = 120000;
        manager->config.on_state_transition = NULL;
        manager->config.on_power_event = NULL;
        manager->config.user_data = NULL;
    }

    manager->has_activity = true;
    manager->last_activity_ms = 0;
    manager->power_transition_pending = false;
    nav_history_push(&manager->history, SCREEN_STATE_IDLE, 0, 0);
}

void screen_state_manager_set_config(ScreenStateManager_t* manager, const ScreenStateManagerConfig_t* config)
{
    if (!manager || !config) return;
    memcpy(&manager->config, config, sizeof(ScreenStateManagerConfig_t));
}

bool screen_state_manager_transition(ScreenStateManager_t* manager, ScreenState_t new_state, uint8_t app_id)
{
    if (!manager) return false;

    ScreenState_t old_state = manager->current_state;

    if (new_state == old_state && old_state != SCREEN_STATE_APP_NESTED) {
        return true;
    }

    manager->current_state = new_state;
    manager->has_activity = true;

    if (new_state == SCREEN_STATE_APP_NESTED) {
        nav_history_push(&manager->history, old_state, manager->last_activity_ms, (old_state == SCREEN_STATE_APP) ? app_id : 0);
    }

    if (manager->config.on_state_transition) {
        manager->config.on_state_transition(old_state, new_state, manager->config.user_data);
    }

    return true;
}

bool screen_state_manager_go_back(ScreenStateManager_t* manager)
{
    if (!manager) return false;

    ScreenState_t current_state;
    uint8_t app_id;

    if (!nav_history_pop(&manager->history, &current_state, &app_id)) {
        manager->current_state = SCREEN_STATE_IDLE;
        return false;
    }

    if (current_state == manager->current_state) {
        nav_history_push(&manager->history, current_state, manager->last_activity_ms, app_id);
        return false;
    }

    return screen_state_manager_transition(manager, current_state, app_id);
}

bool screen_state_manager_is_in_state(const ScreenStateManager_t* manager, ScreenState_t state)
{
    if (!manager) return false;
    return manager->current_state == state;
}

bool screen_state_manager_is_in_app(const ScreenStateManager_t* manager)
{
    if (!manager) return false;
    ScreenState_t state = manager->current_state;
    return (state == SCREEN_STATE_APP || state == SCREEN_STATE_APP_NESTED);
}

bool screen_state_manager_is_inactive(const ScreenStateManager_t* manager)
{
    if (!manager) return false;
    ScreenState_t state = manager->current_state;
    return (state == SCREEN_STATE_IDLE || state == SCREEN_STATE_MENU);
}

void screen_state_manager_update_activity(ScreenStateManager_t* manager, uint32_t timestamp_ms)
{
    if (!manager) return;
    manager->has_activity = true;
    manager->last_activity_ms = timestamp_ms;
}

bool screen_state_manager_check_idle_timeout(const ScreenStateManager_t* manager, uint32_t current_ms)
{
    if (!manager) return false;

    if (!manager->has_activity) return true;

    uint32_t timeout_ms = manager->config.idle_timeout_ms;

    if (manager->current_state == SCREEN_STATE_MENU) {
        timeout_ms = manager->config.menu_timeout_ms;
    } else if (manager->current_state == SCREEN_STATE_APP) {
        timeout_ms = manager->config.app_timeout_ms;
    }

    if (current_ms >= manager->last_activity_ms) {
        return (current_ms - manager->last_activity_ms) > timeout_ms;
    } else {
        return ((0xFFFFFFFF - manager->last_activity_ms + 1) + current_ms) > timeout_ms;
    }
}

ScreenState_t screen_state_manager_get_state(const ScreenStateManager_t* manager)
{
    if (!manager) return SCREEN_STATE_IDLE;
    return manager->current_state;
}

uint8_t screen_state_manager_get_history_size(const ScreenStateManager_t* manager)
{
    if (!manager) return 0;
    return nav_history_get_size(&manager->history);
}

bool screen_state_manager_enter_low_power(ScreenStateManager_t* manager)
{
    if (!manager) return false;

    ScreenState_t old_state = manager->current_state;

    if (old_state != SCREEN_STATE_IDLE && old_state != SCREEN_STATE_LOW_POWER && old_state != SCREEN_STATE_POWER_OFF) {
        nav_history_push(&manager->history, old_state, manager->last_activity_ms, 0);
    }

    manager->current_state = SCREEN_STATE_LOW_POWER;
    manager->has_activity = false;

    if (manager->config.on_power_event) {
        manager->config.on_power_event(true, manager->config.user_data);
    }

    return true;
}

bool screen_state_manager_wake_from_low_power(ScreenStateManager_t* manager, bool return_to_menu)
{
    if (!manager) return false;

    ScreenState_t new_state;

    if (return_to_menu) {
        new_state = SCREEN_STATE_MENU;
    } else {
        ScreenState_t restored_state;
        uint8_t app_id;

        if (nav_history_pop(&manager->history, &restored_state, &app_id)) {
            new_state = restored_state;
        } else {
            new_state = SCREEN_STATE_IDLE;
        }
    }

    return screen_state_manager_transition(manager, new_state, 0);
}

bool screen_state_manager_shutdown(ScreenStateManager_t* manager)
{
    if (!manager) return false;

    ScreenState_t old_state = manager->current_state;

    if (old_state != SCREEN_STATE_IDLE && old_state != SCREEN_STATE_LOW_POWER && old_state != SCREEN_STATE_POWER_OFF) {
        nav_history_push(&manager->history, old_state, manager->last_activity_ms, 0);
    }

    manager->current_state = SCREEN_STATE_POWER_OFF;
    manager->has_activity = false;

    if (manager->config.on_power_event) {
        manager->config.on_power_event(true, manager->config.user_data);
    }

    return true;
}

bool screen_state_manager_is_powered_down(const ScreenStateManager_t* manager)
{
    if (!manager) return true;

    ScreenState_t state = manager->current_state;
    return (state == SCREEN_STATE_LOW_POWER || state == SCREEN_STATE_POWER_OFF);
}
/* End of screen_state_manager.c implementation */

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
    /* No cleanup needed for static tests */
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
    
    /* Push one entry */
    TEST_ASSERT_TRUE(nav_history_push(&history, SCREEN_STATE_MENU, 1000, 0));
    TEST_ASSERT_EQUAL_UINT8(1, history.size);
    
    /* Pop it */
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
    
    /* Push max size entries */
    for (uint8_t i = 0; i < NAV_HISTORY_MAX_SIZE; i++) {
        TEST_ASSERT_TRUE(nav_history_push(&history, (ScreenState_t)i,
                                          1000 * i, 0));
    }

    TEST_ASSERT_EQUAL_UINT8(NAV_HISTORY_MAX_SIZE, history.size);

    /* Next push succeeds but replaces oldest entry */
    /* (implementation removes oldest when full to maintain max size) */
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
    
    /* Pop should fail after clear */
    ScreenState_t dummy_state;
    uint8_t dummy_app;
    TEST_ASSERT_FALSE(nav_history_pop(&history, &dummy_state, &dummy_app));
}

void test_nav_history_wrap_around(void)
{
    NavigationHistory_t history;
    nav_history_init(&history);
    
    /* Fill and push many entries */
    for (uint16_t i = 0; i < 20; i++) {
        nav_history_push(&history, SCREEN_STATE_IDLE + (i % 3), i * 100, 0);
    }
    
    /* History should not exceed max size */
    TEST_ASSERT_LESS_OR_EQUAL(NAV_HISTORY_MAX_SIZE, history.size);
    
    /* Should be able to pop entries */
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
    
    /* Start in app with ID 5 */
    screen_state_manager_transition(&manager, SCREEN_STATE_APP, 5);
    
    /* Navigate to nested screen */
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
    
    /* Navigate to app first */
    screen_state_manager_transition(&manager, SCREEN_STATE_APP, 5);
    
    /* Enter low power */
    TEST_ASSERT_TRUE(screen_state_manager_enter_low_power(&manager));
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_LOW_POWER, manager.current_state);
    TEST_ASSERT_FALSE(manager.has_activity);
    
    /* Power callback should be called */
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
    
    /* Second transition */
    screen_state_manager_transition(&manager, SCREEN_STATE_APP, 5);
    
    TEST_ASSERT_EQUAL_UINT8(2, transition_callback_count);
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_MENU, last_old_state);
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_APP, last_new_state);
}

void test_multiple_transitions(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager, NULL);

    /* Navigate through multiple states */
    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, 0);
    screen_state_manager_transition(&manager, SCREEN_STATE_APP, 5);
    screen_state_manager_transition(&manager, SCREEN_STATE_APP_NESTED, 0);
    screen_state_manager_transition(&manager, SCREEN_STATE_MENU, 0);

    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_MENU, manager.current_state);
    /* History should contain MENU (pushed before APP_NESTED) plus initial IDLE */
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

    /* History contains the previous APP state plus initial IDLE */
    TEST_ASSERT_EQUAL_UINT8(2, screen_state_manager_get_history_size(&manager));
}

/* ============================================================
 * EDGE CASES AND ERROR HANDLING
 * ============================================================ */

void test_state_manager_null_pointer(void)
{
    /* All functions should handle NULL gracefully */
    TEST_ASSERT_FALSE(screen_state_manager_transition(NULL, SCREEN_STATE_MENU, 0));
    TEST_ASSERT_FALSE(screen_state_manager_go_back(NULL));
    TEST_ASSERT_FALSE(screen_state_manager_is_in_state(NULL, SCREEN_STATE_IDLE));
    TEST_ASSERT_FALSE(screen_state_manager_is_in_app(NULL));
    TEST_ASSERT_FALSE(screen_state_manager_is_inactive(NULL));
}

void test_nav_history_null_pointer(void)
{
    /* History functions should handle NULL */
    nav_history_init(NULL);  /* Should not crash */
    TEST_ASSERT_FALSE(nav_history_push(NULL, SCREEN_STATE_IDLE, 0, 0));
    TEST_ASSERT_FALSE(nav_history_pop(NULL, NULL, NULL));
    TEST_ASSERT_FALSE(nav_history_peek(NULL, NULL, NULL));
}

void test_state_change_to_same_state(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager, NULL);
    
    /* Try to transition to same state */
    TEST_ASSERT_TRUE(screen_state_manager_transition(&manager, 
                                                     SCREEN_STATE_IDLE, 0));
    
    /* Should still be in IDLE */
    TEST_ASSERT_EQUAL_INT(SCREEN_STATE_IDLE, manager.current_state);
}

void test_nested_app_navigation(void)
{
    ScreenStateManager_t manager;
    screen_state_manager_init(&manager, NULL);

    /* Go to app 5, then app 10 (nested) */
    screen_state_manager_transition(&manager, SCREEN_STATE_APP, 5);
    screen_state_manager_transition(&manager, SCREEN_STATE_APP_NESTED, 0);

    /* History should contain the previous APP state plus initial IDLE */
    TEST_ASSERT_EQUAL_UINT8(2, manager.history.size);

    /* Go back should return to APP */
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
