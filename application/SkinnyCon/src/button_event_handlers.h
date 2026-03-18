/**
 * @file      button_event_handlers.h
 * @brief     Button event handler module for T-LoRa-Pager-SkinnyCon
 * 
 * This module provides centralized button event handling for the LVGL-based UI.
 * It abstracts hardware button inputs (TCA8418 keyboard encoder) and provides
 * unified event dispatching for navigation and application actions.
 * 
 * Key features:
 * - Encoder button press detection
 * - Navigation key handling (back, menu, select)
 * - Application-specific action dispatching
 * - Double-click and long-press detection
 * - Hardware abstraction layer integration
 */

#ifndef BUTTON_EVENT_HANDLERS_H
#define BUTTON_EVENT_HANDLERS_H

#include <stdint.h>
#include <stdbool.h>

/* Button identifiers */
typedef enum {
    BUTTON_ENCODER_ROTATE = 0,    /* Encoder wheel rotation */
    BUTTON_ENCODER_CLICK,         /* Encoder wheel click */
    BUTTON_ENCODER_DOUBLE_CLICK,  /* Encoder double click */
    BUTTON_LONG_PRESS,            /* Long press (>1s) */
    BUTTON_BACK,                  /* Dedicated back button */
    BUTTON_MENU,                  /* Dedicated menu button (if present) */
    BUTTON_SELECT,                /* Dedicated select button (if present) */
    BUTTON_COUNT                  /* Sentinel */
} ButtonId_t;

/* Button state */
typedef struct {
    uint32_t press_time_ms;       /* Duration of current press */
    uint32_t last_press_time;     /* Timestamp of last press event */
    uint32_t press_count;         /* Number of clicks in current burst */
    bool is_pressed;              /* Current press state */
    bool has_double_click;        /* Double-click detected */
    bool has_long_press;          /* Long press detected */
} ButtonState_t;

/* Button event type */
typedef enum {
    BUTTON_EVENT_PRESS_START,
    BUTTON_EVENT_PRESS_END,
    BUTTON_EVENT_CLICK,
    BUTTON_EVENT_DOUBLE_CLICK,
    BUTTON_EVENT_LONG_PRESS,
    BUTTON_EVENT_ROTATE_UP,
    BUTTON_EVENT_ROTATE_DOWN,
    BUTTON_EVENT_COUNT
} ButtonEventType_t;

/* Button event structure */
typedef struct {
    ButtonId_t button_id;
    ButtonEventType_t event_type;
    uint32_t timestamp_ms;
    void *user_data;              /* Optional event context */
} ButtonEvent_t;

/* Button event callback type */
typedef void (*ButtonEventHandler_t)(const ButtonEvent_t *event);

/* Button manager context */
typedef struct {
    ButtonState_t button_states[BUTTON_COUNT];
    
    /* Event dispatch callbacks */
    ButtonEventHandler_t global_handler;
    ButtonEventHandler_t nav_handler;
    ButtonEventHandler_t app_handler;
    
    /* Configuration */
    uint32_t long_press_threshold_ms;
    uint32_t double_click_threshold_ms;
    uint32_t click_debounce_ms;
    
    /* Navigation state */
    bool navigation_enabled;
    bool app_actions_enabled;
    
    /* Hardware abstraction */
    bool (*hw_has_keyboard)(void);
    void (*hw_flush_keyboard)(void);
    ButtonEvent_t *(*hw_read_button_event)(void);
    
    /* Application context */
    void *active_app_context;
    const char *active_app_name;
} ButtonEventManager_t;

/* ================================================================
 *  PUBLIC API
 * ================================================================ */

/**
 * @brief Initialize button event manager to defaults
 * 
 * @param manager Pointer to manager instance
 */
void button_event_manager_init(ButtonEventManager_t *manager);

/**
 * @brief Set global button event handler
 * 
 * This handler is called for ALL button events before specific handlers.
 * 
 * @param manager Pointer to manager instance
 * @param handler Callback function
 */
void button_event_manager_set_global_handler(ButtonEventManager_t *manager,
                                              ButtonEventHandler_t handler);

/**
 * @brief Set navigation handler (back, menu, select buttons)
 * 
 * @param manager Pointer to manager instance
 * @param handler Callback function
 */
void button_event_manager_set_nav_handler(ButtonEventManager_t *manager,
                                           ButtonEventHandler_t handler);

/**
 * @brief Set application handler (app-specific actions)
 * 
 * @param manager Pointer to manager instance
 * @param handler Callback function
 */
void button_event_manager_set_app_handler(ButtonEventManager_t *manager,
                                           ButtonEventHandler_t handler);

/**
 * @brief Enable/disable navigation button events
 * 
 * @param manager Pointer to manager instance
 * @param enabled true to enable, false to disable
 */
void button_event_manager_set_navigation_enabled(ButtonEventManager_t *manager,
                                                  bool enabled);

/**
 * @brief Enable/disable application action events
 * 
 * @param manager Pointer to manager instance
 * @param enabled true to enable, false to disable
 */
void button_event_manager_set_app_actions_enabled(ButtonEventManager_t *manager,
                                                   bool enabled);

/**
 * @brief Set long press timeout (milliseconds)
 * 
 * @param manager Pointer to manager instance
 * @param timeout_ms New timeout value
 */
void button_event_manager_set_long_press_threshold(ButtonEventManager_t *manager,
                                                    uint32_t timeout_ms);

/**
 * @brief Set double click timeout (milliseconds)
 * 
 * @param manager Pointer to manager instance
 * @param timeout_ms New timeout value
 */
void button_event_manager_set_double_click_threshold(ButtonEventManager_t *manager,
                                                      uint32_t timeout_ms);

/**
 * @brief Set click debounce timeout (milliseconds)
 * 
 * @param manager Pointer to manager instance
 * @param timeout_ms New timeout value
 */
void button_event_manager_set_click_debounce(ButtonEventManager_t *manager,
                                              uint32_t timeout_ms);

/**
 * @brief Set active application context
 * 
 * @param manager Pointer to manager instance
 * @param app_name Name of current app (or NULL for menu)
 * @param context Application-specific context data
 */
void button_event_manager_set_active_app(ButtonEventManager_t *manager,
                                          const char *app_name,
                                          void *context);

/**
 * @brief Get active application name
 * 
 * @param manager Pointer to manager instance
 * @return Active app name or NULL if none
 */
const char *button_event_manager_get_active_app(const ButtonEventManager_t *manager);

/**
 * @brief Check if button event manager is enabled
 * 
 * @param manager Pointer to manager instance
 * @return true if enabled, false otherwise
 */
bool button_event_manager_is_enabled(const ButtonEventManager_t *manager);

/**
 * @brief Handle hardware button event
 * 
 * This function should be called from hardware interrupt or poll loop
 * when a button event is detected.
 * 
 * @param manager Pointer to manager instance
 * @param event Raw hardware event
 * @return true if event processed, false if ignored
 */
bool button_event_manager_handle_hw_event(ButtonEventManager_t *manager,
                                           const ButtonEvent_t *event);

/**
 * @brief Poll for button events (non-blocking)
 * 
 * Checks hardware buffer and dispatches to appropriate handlers.
 * 
 * @param manager Pointer to manager instance
 */
void button_event_manager_poll(ButtonEventManager_t *manager);

/**
 * @brief Simulate button event for testing
 * 
 * @param manager Pointer to manager instance
 * @param button_id Button to simulate
 * @param event_type Event type to simulate
 */
void button_event_manager_simulate(ButtonEventManager_t *manager,
                                    ButtonId_t button_id,
                                    ButtonEventType_t event_type);

/**
 * @brief Clear button state (e.g., for app exit)
 * 
 * @param manager Pointer to manager instance
 */
void button_event_manager_clear_state(ButtonEventManager_t *manager);

/**
 * @brief Get button state for debugging
 * 
 * @param manager Pointer to manager instance
 * @param button_id Button to query
 * @return Pointer to button state, or NULL if invalid
 */
const ButtonState_t *button_event_manager_get_button_state(const ButtonEventManager_t *manager,
                                                            ButtonId_t button_id);

/**
 * @brief Print button state to stdout (debugging)
 * 
 * @param manager Pointer to manager instance
 */
void button_event_manager_print_state(ButtonEventManager_t *manager);

/* ================================================================
 *  NAVIGATION ACTION DISPATCHERS
 * ================================================================ */

/**
 * @brief Dispatch back action (current app or navigation)
 * 
 * @param manager Pointer to manager instance
 */
void button_event_manager_dispatch_back(ButtonEventManager_t *manager);

/**
 * @brief Dispatch menu action (return to main menu)
 * 
 * @param manager Pointer to manager instance
 */
void button_event_manager_dispatch_menu(ButtonEventManager_t *manager);

/**
 * @brief Dispatch select/enter action (activate focused item)
 * 
 * @param manager Pointer to manager instance
 */
void button_event_manager_dispatch_select(ButtonEventManager_t *manager);

/**
 * @brief Dispatch rotate up action (scroll up)
 * 
 * @param manager Pointer to manager instance
 */
void button_event_manager_dispatch_rotate_up(ButtonEventManager_t *manager);

/**
 * @brief Dispatch rotate down action (scroll down)
 * 
 * @param manager Pointer to manager instance
 */
void button_event_manager_dispatch_rotate_down(ButtonEventManager_t *manager);

#endif /* BUTTON_EVENT_HANDLERS_H */
