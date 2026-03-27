/**
 * @file      screen_state_manager.c
 * @author    T-Lora-Pager-SkinnyCon Team
 * @license   MIT
 * @copyright Copyright (c) 2025  ShenZhen XinYuan Electronic Technology Co., Ltd
 * @date      2026-03-19
 *
 * @brief     Screen state management implementation.
 *
 * This module provides state tracking, navigation history management,
 * and timeout control for the T-LoRa-Pager conference badge UI.
 */

#include "screen_state_manager.h"
#include <string.h>

/* ============================================================
 * STATE STRING CONVERSION
 * ============================================================ */

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

/* ============================================================
 * NAVIGATION HISTORY IMPLEMENTATION
 * ============================================================ */

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

bool nav_history_push(NavigationHistory_t* history,
                      ScreenState_t state,
                      uint32_t timestamp_ms,
                      uint8_t app_id)
{
    if (!history) return false;
    
    /* If stack is full, remove oldest entry to make room */
    if (history->size >= NAV_HISTORY_MAX_SIZE) {
        history_inc_index(&history->tail, NAV_HISTORY_MAX_SIZE);
        history->size--;
    }
    
    /* Calculate write position */
    uint8_t write_pos = history->head;
    
    /* Store record */
    history->records[write_pos].state = state;
    history->records[write_pos].timestamp_ms = timestamp_ms;
    history->records[write_pos].app_id = app_id;
    
    /* Update head */
    history_inc_index(&history->head, NAV_HISTORY_MAX_SIZE);
    history->size++;
    
    return true;
}

bool nav_history_pop(NavigationHistory_t* history,
                     ScreenState_t* out_state,
                     uint8_t* out_app_id)
{
    if (!history || !out_state || !out_app_id) return false;
    if (history->size == 0) return false;
    
    /* Calculate read position (before head) */
    uint8_t read_pos;
    if (history->head == 0) {
        read_pos = NAV_HISTORY_MAX_SIZE - 1;
    } else {
        read_pos = history->head - 1;
    }
    
    *out_state = history->records[read_pos].state;
    *out_app_id = history->records[read_pos].app_id;
    
    /* Update head and size */
    history->head = read_pos;
    history->size--;
    
    return true;
}

bool nav_history_peek(const NavigationHistory_t* history,
                      ScreenState_t* out_state,
                      uint8_t* out_app_id)
{
    if (!history || !out_state || !out_app_id) return false;
    if (history->size == 0) return false;
    
    /* Peek at the position before head */
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

/* ============================================================
 * STATE MANAGER IMPLEMENTATION
 * ============================================================ */

void screen_state_manager_init(ScreenStateManager_t* manager,
                               const ScreenStateManagerConfig_t* config)
{
    if (!manager) return;
    
    /* Initialize manager struct */
    manager->current_state = SCREEN_STATE_IDLE;
    nav_history_init(&manager->history);
    
    /* Initialize with default config if no custom config provided */
    if (config) {
        screen_state_manager_set_config(manager, config);
    } else {
        manager->config.idle_timeout_ms = 30000;  /* 30 seconds default */
        manager->config.menu_timeout_ms = 60000;  /* 60 seconds default */
        manager->config.app_timeout_ms = 120000;  /* 120 seconds default */
        manager->config.on_state_transition = NULL;
        manager->config.on_power_event = NULL;
        manager->config.user_data = NULL;
    }
    
    manager->has_activity = true;
    manager->last_activity_ms = 0;  /* Will be set by first activity */
    manager->power_transition_pending = false;
    
    /* Push initial idle state to history */
    nav_history_push(&manager->history, SCREEN_STATE_IDLE, 0, 0);
}

void screen_state_manager_set_config(ScreenStateManager_t* manager,
                                     const ScreenStateManagerConfig_t* config)
{
    if (!manager || !config) return;
    
    memcpy(&manager->config, config, sizeof(ScreenStateManagerConfig_t));
}

bool screen_state_manager_transition(ScreenStateManager_t* manager,
                                     ScreenState_t new_state,
                                     uint8_t app_id)
{
    if (!manager) return false;
    
    ScreenState_t old_state = manager->current_state;
    
    /* Skip if already in this state (except for navigation history updates) */
    if (new_state == old_state && old_state != SCREEN_STATE_APP_NESTED) {
        return true;
    }
    
    manager->current_state = new_state;
    manager->has_activity = true;
    /* Timestamp will be updated by caller via update_activity() */
    
    /* Push to history for APP_NESTED state */
    if (new_state == SCREEN_STATE_APP_NESTED) {
        nav_history_push(&manager->history, old_state, 
                        manager->last_activity_ms, 
                        (old_state == SCREEN_STATE_APP) ? app_id : 0);
    }
    
    /* Call transition callback if registered */
    if (manager->config.on_state_transition) {
        manager->config.on_state_transition(old_state, new_state, 
                                           manager->config.user_data);
    }
    
    return true;
}

bool screen_state_manager_go_back(ScreenStateManager_t* manager)
{
    if (!manager) return false;

    ScreenState_t current_state;
    uint8_t app_id;

    /* Try to pop from history */
    if (!nav_history_pop(&manager->history, &current_state, &app_id)) {
        /* No history, stay at idle */
        manager->current_state = SCREEN_STATE_IDLE;
        return false;
    }

    /* If popped state is same as current, we have nothing to go back to */
    if (current_state == manager->current_state) {
        /* Push the state back since we couldn't go anywhere */
        nav_history_push(&manager->history, current_state, manager->last_activity_ms, app_id);
        return false;
    }

    return screen_state_manager_transition(manager, current_state, app_id);
}

bool screen_state_manager_is_in_state(const ScreenStateManager_t* manager,
                                      ScreenState_t state)
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

void screen_state_manager_update_activity(ScreenStateManager_t* manager,
                                          uint32_t timestamp_ms)
{
    if (!manager) return;
    
    manager->has_activity = true;
    manager->last_activity_ms = timestamp_ms;
}

bool screen_state_manager_check_idle_timeout(const ScreenStateManager_t* manager,
                                             uint32_t current_ms)
{
    if (!manager) return false;

    if (!manager->has_activity) {
        /* No activity ever - timeout immediately */
        return true;
    }

    uint32_t timeout_ms = manager->config.idle_timeout_ms;

    /* Adjust timeout based on current state */
    if (manager->current_state == SCREEN_STATE_MENU) {
        timeout_ms = manager->config.menu_timeout_ms;
    } else if (manager->current_state == SCREEN_STATE_APP) {
        timeout_ms = manager->config.app_timeout_ms;
    }

    /* Calculate elapsed time (handle wraparound) */
    if (current_ms >= manager->last_activity_ms) {
        return (current_ms - manager->last_activity_ms) > timeout_ms;
    } else {
        /* Timestamp wrapped around */
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

/* ============================================================
 * POWER MANAGEMENT
 * ============================================================ */

bool screen_state_manager_enter_low_power(ScreenStateManager_t* manager)
{
    if (!manager) return false;
    
    ScreenState_t old_state = manager->current_state;
    
    /* Store state before transition for wake-up */
    if (old_state != SCREEN_STATE_IDLE && 
        old_state != SCREEN_STATE_LOW_POWER &&
        old_state != SCREEN_STATE_POWER_OFF) {
        nav_history_push(&manager->history, old_state, 
                        manager->last_activity_ms, 0);
    }
    
    manager->current_state = SCREEN_STATE_LOW_POWER;
    manager->has_activity = false;
    
    /* Call power event callback */
    if (manager->config.on_power_event) {
        manager->config.on_power_event(true, manager->config.user_data);
    }
    
    return true;
}

bool screen_state_manager_wake_from_low_power(ScreenStateManager_t* manager,
                                              bool return_to_menu)
{
    if (!manager) return false;
    
    ScreenState_t new_state;
    
    if (return_to_menu) {
        new_state = SCREEN_STATE_MENU;
    } else {
        /* Try to restore previous state from history */
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
    
    /* Store state before transition */
    if (old_state != SCREEN_STATE_IDLE && 
        old_state != SCREEN_STATE_LOW_POWER &&
        old_state != SCREEN_STATE_POWER_OFF) {
        nav_history_push(&manager->history, old_state, 
                        manager->last_activity_ms, 0);
    }
    
    manager->current_state = SCREEN_STATE_POWER_OFF;
    manager->has_activity = false;
    
    /* Call power event callback */
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
