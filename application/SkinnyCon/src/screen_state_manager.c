/**
 * @file      screen_state_manager.c
 * @brief     Screen state management implementation for T-LoRa-Pager-SkinnyCon
 * 
 * See screen_state_manager.h for API documentation.
 * 
 * This module provides robust screen state tracking for the UI navigation system,
 * supporting:
 * - State machine transitions (IDLE -> MENU -> APP -> MENU -> IDLE)
 * - Navigation history stack
 * - Power state management
 * - Callback hooks for integration with hardware
 */

#include "screen_state_manager.h"
#include <string.h>
#include <stdio.h>

/* ================================================================
 *  INTERNAL STATE
 * ================================================================ */

static const char *state_names[] = {
    "IDLE",
    "MENU",
    "APP",
    "APP_NESTED",
    "LOW_POWER",
    "POWER_OFF"
};

static const char *transition_names[] = {
    "MANUAL",
    "AUTO",
    "ERROR"
};

/* ================================================================
 *  CORE FUNCTIONS
 * ================================================================ */

void screen_state_manager_init(ScreenStateManager_t *manager)
{
    if (!manager) {
        return;
    }
    
    /* Initialize basic state */
    manager->current_state = SCREEN_STATE_IDLE;
    manager->previous_state = SCREEN_STATE_IDLE;
    
    /* Initialize history */
    manager->history.head = 0;
    manager->history.depth = 0;
    memset(manager->history.history, 0, sizeof(manager->history.history));
    
    /* Initialize callbacks (null by default) */
    manager->on_state_change = NULL;
    manager->on_low_power_enter = NULL;
    manager->on_low_power_exit = NULL;
    manager->on_app_enter = NULL;
    manager->on_app_exit = NULL;
    
    /* Initialize configuration */
    manager->history_enabled = true;
    manager->callbacks_enabled = true;
    manager->low_power_timeout_ms = 30000;  /* 30 seconds default */
    
    /* Initialize timing */
    manager->last_activity_tick = 0;
    manager->screen_timeout_ticks = 0;
}

void screen_state_manager_set_state_callback(ScreenStateManager_t *manager,
                                              void (*callback)(ScreenState_t, ScreenState_t))
{
    if (manager) {
        manager->on_state_change = callback;
    }
}

void screen_state_manager_set_low_power_enter_callback(ScreenStateManager_t *manager,
                                                        void (*callback)(void))
{
    if (manager) {
        manager->on_low_power_enter = callback;
    }
}

void screen_state_manager_set_low_power_exit_callback(ScreenStateManager_t *manager,
                                                       void (*callback)(void))
{
    if (manager) {
        manager->on_low_power_exit = callback;
    }
}

void screen_state_manager_set_app_enter_callback(ScreenStateManager_t *manager,
                                                  void (*callback)(const char *))
{
    if (manager) {
        manager->on_app_enter = callback;
    }
}

void screen_state_manager_set_app_exit_callback(ScreenStateManager_t *manager,
                                                 void (*callback)(const char *))
{
    if (manager) {
        manager->on_app_exit = callback;
    }
}

void screen_state_manager_transition(ScreenStateManager_t *manager,
                                      ScreenState_t new_state,
                                      TransitionType_t type,
                                      const char *app_name)
{
    if (!manager) {
        return;
    }
    
    ScreenState_t old_state = manager->current_state;
    
    /* Skip if no change */
    if (new_state == old_state) {
        return;
    }
    
    /* Handle app-specific transitions */
    if (old_state == SCREEN_STATE_APP && new_state != SCREEN_STATE_APP && 
        new_state != SCREEN_STATE_APP_NESTED && app_name) {
        /* Exit current app */
        if (manager->callbacks_enabled && manager->on_app_exit) {
            manager->on_app_exit(app_name);
        }
    }
    
    if (new_state == SCREEN_STATE_APP || new_state == SCREEN_STATE_APP_NESTED) {
        /* Entering app */
        if (manager->callbacks_enabled && manager->on_app_enter && app_name) {
            manager->on_app_enter(app_name);
        }
    }
    
    /* Update previous state */
    manager->previous_state = old_state;
    
    /* Update current state */
    manager->current_state = new_state;
    
    /* Record transition in history (if enabled) */
    if (manager->history_enabled && manager->history.depth < MAX_NAVIGATION_HISTORY) {
        TransitionRecord_t *record = &manager->history.history[manager->history.head];
        record->from_state = old_state;
        record->to_state = new_state;
        record->type = type;
        record->timestamp_ms = manager->last_activity_tick;
        
        manager->history.head = (manager->history.head + 1) % MAX_NAVIGATION_HISTORY;
        if (manager->history.depth < MAX_NAVIGATION_HISTORY) {
            manager->history.depth++;
        }
    }
    
    /* Call state change callback */
    if (manager->callbacks_enabled && manager->on_state_change) {
        manager->on_state_change(new_state, old_state);
    }
    
    /* Handle low power transitions */
    if (old_state != SCREEN_STATE_LOW_POWER && new_state == SCREEN_STATE_LOW_POWER) {
        if (manager->callbacks_enabled && manager->on_low_power_enter) {
            manager->on_low_power_enter();
        }
    } else if (old_state == SCREEN_STATE_LOW_POWER && new_state != SCREEN_STATE_LOW_POWER) {
        if (manager->callbacks_enabled && manager->on_low_power_exit) {
            manager->on_low_power_exit();
        }
    }
}

ScreenState_t screen_state_manager_get_state(const ScreenStateManager_t *manager)
{
    if (!manager) {
        return SCREEN_STATE_COUNT;  /* Invalid state */
    }
    return manager->current_state;
}

ScreenState_t screen_state_manager_get_previous_state(const ScreenStateManager_t *manager)
{
    if (!manager) {
        return SCREEN_STATE_IDLE;
    }
    return manager->previous_state;
}

TransitionType_t screen_state_manager_get_last_transition(const ScreenStateManager_t *manager)
{
    if (!manager || manager->history.depth == 0) {
        return TRANSITION_COUNT;
    }
    
    /* Get most recent record */
    uint32_t index = manager->history.head;
    if (index == 0) {
        index = manager->history.depth - 1;
    } else {
        index = (index - 1) % MAX_NAVIGATION_HISTORY;
    }
    
    return manager->history.history[index].type;
}

bool screen_state_manager_is_in_state(const ScreenStateManager_t *manager, ScreenState_t state)
{
    if (!manager) {
        return false;
    }
    return manager->current_state == state;
}

bool screen_state_manager_is_in_app(const ScreenStateManager_t *manager)
{
    if (!manager) {
        return false;
    }
    return manager->current_state == SCREEN_STATE_APP || 
           manager->current_state == SCREEN_STATE_APP_NESTED;
}

bool screen_state_manager_is_low_power(const ScreenStateManager_t *manager)
{
    if (!manager) {
        return false;
    }
    return manager->current_state == SCREEN_STATE_LOW_POWER || 
           manager->current_state == SCREEN_STATE_POWER_OFF;
}

bool screen_state_manager_has_history(const ScreenStateManager_t *manager)
{
    if (!manager) {
        return false;
    }
    return manager->history_enabled && manager->history.depth > 0;
}

bool screen_state_manager_navigate_back(ScreenStateManager_t *manager)
{
    if (!manager || !manager->history_enabled || manager->history.depth == 0) {
        /* No history, default to MENU */
        screen_state_manager_transition(manager, SCREEN_STATE_MENU, TRANSITION_MANUAL, NULL);
        return false;
    }
    
    /* Get previous record from history */
    uint32_t index = manager->history.head;
    if (index == 0) {
        index = manager->history.depth - 1;
    } else {
        index = (index - 1) % MAX_NAVIGATION_HISTORY;
    }
    
    TransitionRecord_t *record = &manager->history.history[index];
    ScreenState_t prev_state = record->from_state;
    
    /* Remove this record (simplified: just decrement depth) */
    if (manager->history.depth > 0) {
        manager->history.depth--;
    }
    
    /* Transition back */
    screen_state_manager_transition(manager, prev_state, TRANSITION_MANUAL, NULL);
    return true;
}

void screen_state_manager_clear_history(ScreenStateManager_t *manager)
{
    if (!manager) {
        return;
    }
    
    manager->history.head = 0;
    manager->history.depth = 0;
    memset(manager->history.history, 0, sizeof(manager->history.history));
}

void screen_state_manager_update_activity(ScreenStateManager_t *manager, uint32_t current_tick)
{
    if (!manager) {
        return;
    }
    
    manager->last_activity_tick = current_tick;
    
    /* If in low power state, wake up */
    if (manager->current_state == SCREEN_STATE_LOW_POWER) {
        screen_state_manager_transition(manager, 
                                         manager->previous_state, 
                                         TRANSITION_AUTO, 
                                         NULL);
    }
}

bool screen_state_manager_check_timeout(const ScreenStateManager_t *manager, uint32_t current_tick)
{
    if (!manager) {
        return false;
    }
    
    if (manager->last_activity_tick == 0) {
        return false;
    }
    
    uint32_t elapsed = current_tick - manager->last_activity_tick;
    
    /* Handle wraparound */
    if (elapsed > 0xFFFFFFFF / 2) {
        elapsed = 0xFFFFFFFF - manager->last_activity_tick + current_tick;
    }
    
    return elapsed > manager->low_power_timeout_ms;
}

uint32_t screen_state_manager_get_timeout(const ScreenStateManager_t *manager)
{
    if (!manager) {
        return 30000;  /* Default 30 seconds */
    }
    return manager->low_power_timeout_ms;
}

void screen_state_manager_set_timeout(ScreenStateManager_t *manager, uint32_t timeout_ms)
{
    if (manager) {
        manager->low_power_timeout_ms = timeout_ms;
    }
}

void screen_state_manager_set_history_enabled(ScreenStateManager_t *manager, bool enabled)
{
    if (manager) {
        manager->history_enabled = enabled;
    }
}

void screen_state_manager_set_callbacks_enabled(ScreenStateManager_t *manager, bool enabled)
{
    if (manager) {
        manager->callbacks_enabled = enabled;
    }
}

/* ================================================================
 *  UTILITY FUNCTIONS
 * ================================================================ */

const char *screen_state_manager_get_state_name(ScreenState_t state)
{
    if (state < SCREEN_STATE_COUNT) {
        return state_names[state];
    }
    return "UNKNOWN";
}

const char *screen_state_manager_get_transition_name(TransitionType_t type)
{
    if (type < TRANSITION_COUNT) {
        return transition_names[type];
    }
    return "UNKNOWN";
}

void screen_state_manager_print_state(ScreenStateManager_t *manager)
{
    if (!manager) {
        printf("ScreenStateManager: NULL\n");
        return;
    }
    
    printf("ScreenState: %s\n", screen_state_manager_get_state_name(manager->current_state));
    printf("PreviousState: %s\n", screen_state_manager_get_state_name(manager->previous_state));
    printf("HistoryDepth: %u\n", manager->history.depth);
    printf("HistoryEnabled: %s\n", manager->history_enabled ? "true" : "false");
    printf("CallbacksEnabled: %s\n", manager->callbacks_enabled ? "true" : "false");
    printf("Timeout: %lu ms\n", manager->low_power_timeout_ms);
    printf("LastActivity: %lu ms\n", manager->last_activity_tick);
    
    if (manager->history.depth > 0) {
        printf("Navigation History:\n");
        for (uint32_t i = 0; i < manager->history.depth; i++) {
            uint32_t idx = (manager->history.head - manager->history.depth + i + 1) % MAX_NAVIGATION_HISTORY;
            printf("  [%u] %s -> %s (%s)\n",
                   i,
                   screen_state_manager_get_state_name(manager->history.history[idx].from_state),
                   screen_state_manager_get_state_name(manager->history.history[idx].to_state),
                   screen_state_manager_get_transition_name(manager->history.history[idx].type));
        }
    }
}
