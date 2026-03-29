/**
 * @file      screen_state_manager.h
 * @author    T-Lora-Pager-SkinnyCon Team
 * @license   MIT
 * @copyright Copyright (c) 2025  ShenZhen XinYuan Electronic Technology Co., Ltd
 * @date      2026-03-19
 *
 * @brief     Screen state management for UI navigation logic.
 *
 * This module provides state tracking, navigation history management,
 * and timeout control for the T-LoRa-Pager conference badge UI.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ============================================================
 * SCREEN STATE ENUMERATION
 * ============================================================ */

/**
 * @brief Enumeration of all possible UI screen states.
 * 
 * The screen state machine tracks the current UI context
 * to manage navigation, timeouts, and power states.
 */
typedef enum {
    SCREEN_STATE_IDLE,        /* Idle screen, waiting for input */
    SCREEN_STATE_MENU,        /* Main menu tileview open */
    SCREEN_STATE_APP,         /* Active application screen */
    SCREEN_STATE_APP_NESTED,  /* Nested screen within an app */
    SCREEN_STATE_LOW_POWER,   /* Low power standby mode */
    SCREEN_STATE_POWER_OFF,   /* Device is powered off */
    SCREEN_STATE_COUNT        /* Sentinel for array sizing */
} ScreenState_t;

/**
 * @brief Convert screen state to human-readable string.
 * @param state The screen state to convert
 * @return String representation of the state
 */
const char* screen_state_to_string(ScreenState_t state);

/**
 * @brief Convert string to screen state (case-insensitive).
 * @param str String representation of state (e.g. "IDLE", "idle", "Idle")
 * @return ScreenState_t or SCREEN_STATE_IDLE on invalid input
 */
ScreenState_t screen_state_from_string(const char* str);

/* ============================================================
 * NAVIGATION HISTORY
 * ============================================================ */

#define NAV_HISTORY_MAX_SIZE  10

/**
 * @brief Navigation history record.
 */
typedef struct {
    ScreenState_t state;
    uint32_t timestamp_ms;  /* Time in milliseconds */
    uint8_t app_id;         /* App identifier if in app state */
} NavHistoryRecord_t;

/**
 * @brief Navigation history stack.
 */
typedef struct {
    NavHistoryRecord_t records[NAV_HISTORY_MAX_SIZE];
    uint8_t head;           /* Index of newest record */
    uint8_t tail;           /* Index of oldest record */
    uint8_t size;           /* Current number of records */
} NavigationHistory_t;

/**
 * @brief Initialize navigation history stack.
 * @param history Pointer to navigation history structure
 */
void nav_history_init(NavigationHistory_t* history);

/**
 * @brief Push a new state onto the history stack.
 * @param history Pointer to navigation history
 * @param state New screen state
 * @param timestamp_ms Current timestamp in milliseconds
 * @param app_id App identifier (0 if not in app state)
 * @return true if push succeeded (evicts oldest entry when full), false on NULL input
 */
bool nav_history_push(NavigationHistory_t* history, 
                      ScreenState_t state, 
                      uint32_t timestamp_ms, 
                      uint8_t app_id);

/**
 * @brief Pop the most recent state from history.
 * @param history Pointer to navigation history
 * @param out_state Pointer to store popped state
 * @param out_app_id Pointer to store app ID
 * @return true if pop succeeded, false if stack is empty
 */
bool nav_history_pop(NavigationHistory_t* history,
                     ScreenState_t* out_state,
                     uint8_t* out_app_id);

/**
 * @brief Peek at the top of the history stack without removing.
 * @param history Pointer to navigation history
 * @param out_state Pointer to store current state
 * @param out_app_id Pointer to store app ID
 * @return true if stack has records, false if empty
 */
bool nav_history_peek(const NavigationHistory_t* history,
                      ScreenState_t* out_state,
                      uint8_t* out_app_id);

/**
 * @brief Clear the navigation history stack.
 * @param history Pointer to navigation history
 */
void nav_history_clear(NavigationHistory_t* history);

/**
 * @brief Get the number of records in the history stack.
 * @param history Pointer to navigation history
 * @return Number of records (0 if empty)
 */
uint8_t nav_history_get_size(const NavigationHistory_t* history);

/* ============================================================
 * STATE MANAGER
 * ============================================================ */

/**
 * @brief Callback function type for state transitions.
 */
typedef void (*ScreenStateTransitionCallback_t)(ScreenState_t old_state,
                                                 ScreenState_t new_state,
                                                 void* user_data);

/**
 * @brief Callback function type for power events.
 */
typedef void (*PowerEventCallback_t)(bool low_power, void* user_data);

/**
 * @brief Screen state manager configuration.
 */
typedef struct {
    ScreenStateTransitionCallback_t on_state_transition;
    PowerEventCallback_t on_power_event;
    void* user_data;
    uint32_t idle_timeout_ms;      /* Timeout before entering low power */
    uint32_t menu_timeout_ms;      /* Timeout when menu is open */
    uint32_t app_timeout_ms;       /* Timeout when app is active */
} ScreenStateManagerConfig_t;

/**
 * @brief Screen state manager instance.
 */
typedef struct {
    ScreenState_t current_state;
    NavigationHistory_t history;
    ScreenStateManagerConfig_t config;
    bool has_activity;           /* Has recent user activity */
    uint32_t last_activity_ms;   /* Timestamp of last activity */
} ScreenStateManager_t;

/**
 * @brief Initialize screen state manager with default config.
 * @param manager Pointer to state manager
 * @param config Configuration parameters (can be NULL for defaults)
 */
void screen_state_manager_init(ScreenStateManager_t* manager,
                               const ScreenStateManagerConfig_t* config);

/**
 * @brief Set custom configuration for state manager.
 * @param manager Pointer to state manager
 * @param config Configuration parameters
 */
void screen_state_manager_set_config(ScreenStateManager_t* manager,
                                     const ScreenStateManagerConfig_t* config);

/**
 * @brief Transition to a new screen state.
 * @param manager Pointer to state manager
 * @param new_state Target screen state
 * @param app_id App identifier (0 if not applicable)
 * @return true if transition succeeded
 */
bool screen_state_manager_transition(ScreenStateManager_t* manager,
                                     ScreenState_t new_state,
                                     uint8_t app_id);

/**
 * @brief Go back to previous state in history.
 * @param manager Pointer to state manager
 * @return true if transition succeeded, false if no history
 */
bool screen_state_manager_go_back(ScreenStateManager_t* manager);

/**
 * @brief Check if currently in a specific state.
 * @param manager Pointer to state manager
 * @param state State to check
 * @return true if in specified state
 */
bool screen_state_manager_is_in_state(const ScreenStateManager_t* manager,
                                      ScreenState_t state);

/**
 * @brief Check if manager is in app mode (any app).
 * @param manager Pointer to state manager
 * @return true if in app or app_nested state
 */
bool screen_state_manager_is_in_app(const ScreenStateManager_t* manager);

/**
 * @brief Check if manager is in menu or idle.
 * @param manager Pointer to state manager
 * @return true if in MENU or IDLE state
 */
bool screen_state_manager_is_inactive(const ScreenStateManager_t* manager);

/**
 * @brief Update activity timestamp (called on user input).
 * @param manager Pointer to state manager
 * @param timestamp_ms Current timestamp
 */
void screen_state_manager_update_activity(ScreenStateManager_t* manager,
                                          uint32_t timestamp_ms);

/**
 * @brief Check if idle timeout has expired.
 * @param manager Pointer to state manager
 * @param current_ms Current timestamp
 * @return true if timeout expired
 */
bool screen_state_manager_check_idle_timeout(const ScreenStateManager_t* manager,
                                             uint32_t current_ms);

/**
 * @brief Get the current screen state.
 * @param manager Pointer to state manager
 * @return Current ScreenState_t
 */
ScreenState_t screen_state_manager_get_state(const ScreenStateManager_t* manager);

/**
 * @brief Get the navigation history size.
 * @param manager Pointer to state manager
 * @return Number of records in history
 */
uint8_t screen_state_manager_get_history_size(const ScreenStateManager_t* manager);

/* ============================================================
 * POWER MANAGEMENT INTEGRATION
 * ============================================================ */

/**
 * @brief Enter low power mode.
 * @param manager Pointer to state manager
 * @return true if transition succeeded
 */
bool screen_state_manager_enter_low_power(ScreenStateManager_t* manager);

/**
 * @brief Wake from low power mode.
 * @param manager Pointer to state manager
 * @param return_to_menu If true, return to menu; if false, return to idle
 * @return true if transition succeeded
 */
bool screen_state_manager_wake_from_low_power(ScreenStateManager_t* manager,
                                              bool return_to_menu);

/**
 * @brief Shut down the device.
 * @param manager Pointer to state manager
 * @return true if transition succeeded
 */
bool screen_state_manager_shutdown(ScreenStateManager_t* manager);

/**
 * @brief Check if device is in low power or off state.
 * @param manager Pointer to state manager
 * @return true if in low power or power off
 */
bool screen_state_manager_is_powered_down(const ScreenStateManager_t* manager);

#ifdef __cplusplus
}
#endif
