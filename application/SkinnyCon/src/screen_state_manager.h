/**
 * @file      screen_state_manager.h
 * @brief     Screen state management module for T-LoRa-Pager-SkinnyCon
 * 
 * This module provides state management for screen navigation, including:
 * - Screen state tracking (menu, app, idle)
 * - Navigation history stack
 * - Screen transition callbacks
 * - Power state integration
 * 
 * Part of the UI Navigation Logic improvements.
 */

#ifndef SCREEN_STATE_MANAGER_H
#define SCREEN_STATE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

/* Maximum navigation history depth */
#define MAX_NAVIGATION_HISTORY  10

/* Screen state enumeration */
typedef enum {
    SCREEN_STATE_IDLE,          /* Clock screen (low power) */
    SCREEN_STATE_MENU,          /* Main app menu tileview */
    SCREEN_STATE_APP,           /* Active application screen */
    SCREEN_STATE_APP_NESTED,    /* Nested menu within app (e.g., settings) */
    SCREEN_STATE_LOW_POWER,     /* Screen off, device in sleep */
    SCREEN_STATE_POWER_OFF,     /* Device powered down (low battery) */
    SCREEN_STATE_COUNT
} ScreenState_t;

/* Screen transition event types */
typedef enum {
    TRANSITION_MANUAL,          /* User-initiated (button/encoder) */
    TRANSITION_AUTO,            /* Auto (timeout, sleep) */
    TRANSITION_ERROR,           /* Error-induced */
    TRANSITION_COUNT
} TransitionType_t;

/* Screen transition record for history */
typedef struct {
    ScreenState_t from_state;
    ScreenState_t to_state;
    TransitionType_t type;
    uint32_t timestamp_ms;      /* System tick at transition */
} TransitionRecord_t;

/* Navigation history stack */
typedef struct {
    TransitionRecord_t history[MAX_NAVIGATION_HISTORY];
    uint32_t head;              /* Index of most recent */
    uint32_t depth;             /* Number of records in history */
} NavigationHistory_t;

/* Screen state manager context */
typedef struct {
    ScreenState_t current_state;
    ScreenState_t previous_state;
    NavigationHistory_t history;
    
    /* Callbacks */
    void (*on_state_change)(ScreenState_t new_state, ScreenState_t old_state);
    void (*on_low_power_enter)(void);
    void (*on_low_power_exit)(void);
    void (*on_app_enter)(const char *app_name);
    void (*on_app_exit)(const char *app_name);
    
    /* Configuration */
    bool history_enabled;
    bool callbacks_enabled;
    uint32_t low_power_timeout_ms;
    
    /* Timing */
    uint32_t last_activity_tick;
    uint32_t screen_timeout_ticks;
} ScreenStateManager_t;

/* ================================================================
 *  PUBLIC API
 * ================================================================ */

/**
 * @brief Initialize screen state manager to default state
 * 
 * @param manager Pointer to manager instance
 */
void screen_state_manager_init(ScreenStateManager_t *manager);

/**
 * @brief Set state change callback
 * 
 * @param manager Pointer to manager instance
 * @param callback Function to call on state change
 */
void screen_state_manager_set_state_callback(ScreenStateManager_t *manager,
                                              void (*callback)(ScreenState_t, ScreenState_t));

/**
 * @brief Set low power enter callback
 * 
 * @param manager Pointer to manager instance
 * @param callback Function to call before entering low power
 */
void screen_state_manager_set_low_power_enter_callback(ScreenStateManager_t *manager,
                                                        void (*callback)(void));

/**
 * @brief Set low power exit callback
 * 
 * @param manager Pointer to manager instance
 * @param callback Function to call after exiting low power
 */
void screen_state_manager_set_low_power_exit_callback(ScreenStateManager_t *manager,
                                                       void (*callback)(void));

/**
 * @brief Set app enter callback
 * 
 * @param manager Pointer to manager instance
 * @param callback Function to call when entering an app
 */
void screen_state_manager_set_app_enter_callback(ScreenStateManager_t *manager,
                                                  void (*callback)(const char *));

/**
 * @brief Set app exit callback
 * 
 * @param manager Pointer to manager instance
 * @param callback Function to call when exiting an app
 */
void screen_state_manager_set_app_exit_callback(ScreenStateManager_t *manager,
                                                 void (*callback)(const char *));

/**
 * @brief Transition to a new screen state
 * 
 * Handles state change logic, navigation history, and callbacks.
 * 
 * @param manager Pointer to manager instance
 * @param new_state Target state
 * @param type Transition type (manual/auto/error)
 * @param app_name Optional app name for APP state transitions
 */
void screen_state_manager_transition(ScreenStateManager_t *manager,
                                      ScreenState_t new_state,
                                      TransitionType_t type,
                                      const char *app_name);

/**
 * @brief Get current screen state
 * 
 * @param manager Pointer to manager instance
 * @return Current state
 */
ScreenState_t screen_state_manager_get_state(const ScreenStateManager_t *manager);

/**
 * @brief Get previous screen state
 * 
 * @param manager Pointer to manager instance
 * @return Previous state, or IDLE if none
 */
ScreenState_t screen_state_manager_get_previous_state(const ScreenStateManager_t *manager);

/**
 * @brief Get last transition type
 * 
 * @param manager Pointer to manager instance
 * @return Last transition type, or TRANSITION_COUNT on error
 */
TransitionType_t screen_state_manager_get_last_transition(const ScreenStateManager_t *manager);

/**
 * @brief Check if currently in a screen state
 * 
 * @param manager Pointer to manager instance
 * @param state State to check
 * @return true if in state, false otherwise
 */
bool screen_state_manager_is_in_state(const ScreenStateManager_t *manager, ScreenState_t state);

/**
 * @brief Check if currently in an app (APP or APP_NESTED)
 * 
 * @param manager Pointer to manager instance
 * @return true if in an app, false otherwise
 */
bool screen_state_manager_is_in_app(const ScreenStateManager_t *manager);

/**
 * @brief Check if in low power or idle state
 * 
 * @param manager Pointer to manager instance
 * @return true if low power/idle, false otherwise
 */
bool screen_state_manager_is_low_power(const ScreenStateManager_t *manager);

/**
 * @brief Check if navigation history is available
 * 
 * @param manager Pointer to manager instance
 * @return true if history available, false otherwise
 */
bool screen_state_manager_has_history(const ScreenStateManager_t *manager);

/**
 * @brief Navigate back using history (if available)
 * 
 * If history is enabled and has records, will transition to previous state.
 * Otherwise, transitions to MENU state.
 * 
 * @param manager Pointer to manager instance
 * @return true if navigation performed, false if no history available
 */
bool screen_state_manager_navigate_back(ScreenStateManager_t *manager);

/**
 * @brief Clear navigation history
 * 
 * @param manager Pointer to manager instance
 */
void screen_state_manager_clear_history(ScreenStateManager_t *manager);

/**
 * @brief Update activity timer (for timeout detection)
 * 
 * Call this on user activity (button press, encoder rotation, touch).
 * 
 * @param manager Pointer to manager instance
 * @param current_tick Current system tick
 */
void screen_state_manager_update_activity(ScreenStateManager_t *manager, uint32_t current_tick);

/**
 * @brief Check if timeout has occurred
 * 
 * @param manager Pointer to manager instance
 * @param current_tick Current system tick
 * @return true if timeout occurred, false otherwise
 */
bool screen_state_manager_check_timeout(const ScreenStateManager_t *manager, uint32_t current_tick);

/**
 * @brief Get timeout in milliseconds
 * 
 * @param manager Pointer to manager instance
 * @return Timeout value in ms
 */
uint32_t screen_state_manager_get_timeout(const ScreenStateManager_t *manager);

/**
 * @brief Set timeout in milliseconds
 * 
 * @param manager Pointer to manager instance
 * @param timeout_ms New timeout value
 */
void screen_state_manager_set_timeout(ScreenStateManager_t *manager, uint32_t timeout_ms);

/**
 * @brief Enable/disable navigation history
 * 
 * @param manager Pointer to manager instance
 * @param enabled true to enable, false to disable
 */
void screen_state_manager_set_history_enabled(ScreenStateManager_t *manager, bool enabled);

/**
 * @brief Enable/disable callbacks
 * 
 * @param manager Pointer to manager instance
 * @param enabled true to enable, false to disable
 */
void screen_state_manager_set_callbacks_enabled(ScreenStateManager_t *manager, bool enabled);

/* ================================================================
 *  UTILITY FUNCTIONS
 * ================================================================ */

/**
 * @brief Get string name of screen state (for debugging/logging)
 * 
 * @param state State to convert
 * @return Static string with state name
 */
const char *screen_state_manager_get_state_name(ScreenState_t state);

/**
 * @brief Get string name of transition type (for debugging/logging)
 * 
 * @param type Transition type
 * @return Static string with type name
 */
const char *screen_state_manager_get_transition_name(TransitionType_t type);

/**
 * @brief Print current state and history to stdout (debugging)
 * 
 * @param manager Pointer to manager instance
 */
void screen_state_manager_print_state(ScreenStateManager_t *manager);

#endif /* SCREEN_STATE_MANAGER_H */
