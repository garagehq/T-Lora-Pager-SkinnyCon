/**
 * @file      button_event_handlers.c
 * @brief     Button event handler implementation for T-LoRa-Pager-SkinnyCon
 * 
 * See button_event_handlers.h for API documentation.
 * 
 * This module provides centralized button event handling for navigation
 * and application actions, abstracting hardware-specific inputs.
 */

#include "button_event_handlers.h"
#include <string.h>
#include <stdio.h>
#include "hal_interface.h"

/* ================================================================
 *  INTERNAL HELPERS
 * ================================================================ */

static const char *button_id_names[] = {
    "ROTATE",
    "CLICK",
    "DOUBLE_CLICK",
    "LONG_PRESS",
    "BACK",
    "MENU",
    "SELECT"
};

static const char *event_type_names[] = {
    "PRESS_START",
    "PRESS_END",
    "CLICK",
    "DOUBLE_CLICK",
    "LONG_PRESS",
    "ROTATE_UP",
    "ROTATE_DOWN"
};

/* Check if button ID is valid */
static bool is_valid_button_id(ButtonId_t button_id)
{
    return button_id >= 0 && button_id < BUTTON_COUNT;
}

/* Check if event type is valid */
static bool is_valid_event_type(ButtonEventType_t event_type)
{
    return event_type >= 0 && event_type < BUTTON_EVENT_COUNT;
}

/* Dispatch event to global handler */
static void dispatch_to_global(ButtonEventManager_t *manager,
                                const ButtonEvent_t *event)
{
    if (manager && manager->global_handler && is_valid_event_type(event->event_type)) {
        manager->global_handler(event);
    }
}

/* Dispatch event to navigation handler */
static void dispatch_to_nav(ButtonEventManager_t *manager,
                             const ButtonEvent_t *event)
{
    if (manager && manager->nav_handler && 
        manager->navigation_enabled && 
        is_valid_event_type(event->event_type)) {
        manager->nav_handler(event);
    }
}

/* Dispatch event to app handler */
static void dispatch_to_app(ButtonEventManager_t *manager,
                             const ButtonEvent_t *event)
{
    if (manager && manager->app_handler && 
        manager->app_actions_enabled && 
        is_valid_event_type(event->event_type)) {
        manager->app_handler(event);
    }
}

/* ================================================================
 *  CORE FUNCTIONS
 * ================================================================ */

void button_event_manager_init(ButtonEventManager_t *manager)
{
    if (!manager) {
        return;
    }
    
    /* Initialize button states */
    for (int i = 0; i < BUTTON_COUNT; i++) {
        manager->button_states[i].press_time_ms = 0;
        manager->button_states[i].last_press_time = 0;
        manager->button_states[i].press_count = 0;
        manager->button_states[i].is_pressed = false;
        manager->button_states[i].has_double_click = false;
        manager->button_states[i].has_long_press = false;
    }
    
    /* Initialize callbacks (null by default) */
    manager->global_handler = NULL;
    manager->nav_handler = NULL;
    manager->app_handler = NULL;
    
    /* Initialize configuration */
    manager->long_press_threshold_ms = 1000;
    manager->double_click_threshold_ms = 300;
    manager->click_debounce_ms = 50;
    
    /* Initialize navigation state */
    manager->navigation_enabled = true;
    manager->app_actions_enabled = true;
    
    /* Initialize hardware abstraction */
    manager->hw_has_keyboard = hw_has_keyboard;
    manager->hw_flush_keyboard = hw_flush_keyboard;
    manager->hw_read_button_event = NULL;  /* Set by caller if custom */
    
    /* Initialize context */
    manager->active_app_context = NULL;
    manager->active_app_name = NULL;
}

void button_event_manager_set_global_handler(ButtonEventManager_t *manager,
                                              ButtonEventHandler_t handler)
{
    if (manager) {
        manager->global_handler = handler;
    }
}

void button_event_manager_set_nav_handler(ButtonEventManager_t *manager,
                                           ButtonEventHandler_t handler)
{
    if (manager) {
        manager->nav_handler = handler;
    }
}

void button_event_manager_set_app_handler(ButtonEventManager_t *manager,
                                           ButtonEventHandler_t handler)
{
    if (manager) {
        manager->app_handler = handler;
    }
}

void button_event_manager_set_navigation_enabled(ButtonEventManager_t *manager,
                                                  bool enabled)
{
    if (manager) {
        manager->navigation_enabled = enabled;
    }
}

void button_event_manager_set_app_actions_enabled(ButtonEventManager_t *manager,
                                                   bool enabled)
{
    if (manager) {
        manager->app_actions_enabled = enabled;
    }
}

void button_event_manager_set_long_press_threshold(ButtonEventManager_t *manager,
                                                    uint32_t timeout_ms)
{
    if (manager) {
        manager->long_press_threshold_ms = timeout_ms;
    }
}

void button_event_manager_set_double_click_threshold(ButtonEventManager_t *manager,
                                                      uint32_t timeout_ms)
{
    if (manager) {
        manager->double_click_threshold_ms = timeout_ms;
    }
}

void button_event_manager_set_click_debounce(ButtonEventManager_t *manager,
                                              uint32_t timeout_ms)
{
    if (manager) {
        manager->click_debounce_ms = timeout_ms;
    }
}

void button_event_manager_set_active_app(ButtonEventManager_t *manager,
                                          const char *app_name,
                                          void *context)
{
    if (manager) {
        manager->active_app_name = app_name;
        manager->active_app_context = context;
    }
}

const char *button_event_manager_get_active_app(const ButtonEventManager_t *manager)
{
    if (!manager) {
        return NULL;
    }
    return manager->active_app_name;
}

bool button_event_manager_is_enabled(const ButtonEventManager_t *manager)
{
    if (!manager) {
        return false;
    }
    return manager->navigation_enabled || manager->app_actions_enabled;
}

bool button_event_manager_handle_hw_event(ButtonEventManager_t *manager,
                                           const ButtonEvent_t *event)
{
    if (!manager || !event || !is_valid_button_id(event->button_id)) {
        return false;
    }
    
    uint32_t current_time = lv_tick_get();
    ButtonState_t *state = &manager->button_states[event->button_id];
    
    /* Update timestamp */
    state->last_press_time = current_time;
    
    /* Dispatch to global handler first */
    dispatch_to_global(manager, event);
    
    /* Handle by event type */
    switch (event->event_type) {
        case BUTTON_EVENT_CLICK:
            state->press_count++;
            dispatch_to_nav(manager, event);
            dispatch_to_app(manager, event);
            break;
            
        case BUTTON_EVENT_DOUBLE_CLICK:
            state->has_double_click = true;
            dispatch_to_app(manager, event);
            break;
            
        case BUTTON_EVENT_LONG_PRESS:
            state->has_long_press = true;
            dispatch_to_app(manager, event);
            break;
            
        case BUTTON_EVENT_ROTATE_UP:
        case BUTTON_EVENT_ROTATE_DOWN:
            dispatch_to_nav(manager, event);
            break;
            
        default:
            break;
    }
    
    return true;
}

void button_event_manager_poll(ButtonEventManager_t *manager)
{
    if (!manager) {
        return;
    }
    
    /* If custom hardware reader is provided, use it */
    if (manager->hw_read_button_event) {
        ButtonEvent_t *event = manager->hw_read_button_event();
        if (event) {
            button_event_manager_handle_hw_event(manager, event);
        }
        return;
    }
    
    /* Default: use hardware abstraction layer */
    if (manager->hw_has_keyboard && !manager->hw_has_keyboard()) {
        return;
    }
    
    /* Flush keyboard buffer if needed */
    if (manager->hw_flush_keyboard) {
        manager->hw_flush_keyboard();
    }
}

void button_event_manager_simulate(ButtonEventManager_t *manager,
                                    ButtonId_t button_id,
                                    ButtonEventType_t event_type)
{
    if (!manager || !is_valid_button_id(button_id) || !is_valid_event_type(event_type)) {
        return;
    }
    
    ButtonEvent_t event = {
        .button_id = button_id,
        .event_type = event_type,
        .timestamp_ms = lv_tick_get(),
        .user_data = NULL
    };
    
    button_event_manager_handle_hw_event(manager, &event);
}

void button_event_manager_clear_state(ButtonEventManager_t *manager)
{
    if (!manager) {
        return;
    }
    
    /* Clear all button states */
    for (int i = 0; i < BUTTON_COUNT; i++) {
        manager->button_states[i].press_count = 0;
        manager->button_states[i].is_pressed = false;
        manager->button_states[i].has_double_click = false;
        manager->button_states[i].has_long_press = false;
    }
    
    /* Clear app context */
    manager->active_app_name = NULL;
    manager->active_app_context = NULL;
}

const ButtonState_t *button_event_manager_get_button_state(const ButtonEventManager_t *manager,
                                                            ButtonId_t button_id)
{
    if (!manager || !is_valid_button_id(button_id)) {
        return NULL;
    }
    return &manager->button_states[button_id];
}

void button_event_manager_print_state(ButtonEventManager_t *manager)
{
    if (!manager) {
        printf("ButtonEventManager: NULL\n");
        return;
    }
    
    printf("ButtonEventManager State:\n");
    printf("  Navigation Enabled: %s\n", manager->navigation_enabled ? "true" : "false");
    printf("  App Actions Enabled: %s\n", manager->app_actions_enabled ? "true" : "false");
    printf("  Long Press Threshold: %lu ms\n", manager->long_press_threshold_ms);
    printf("  Double Click Threshold: %lu ms\n", manager->double_click_threshold_ms);
    printf("  Click Debounce: %lu ms\n", manager->click_debounce_ms);
    printf("  Active App: %s\n", manager->active_app_name ? manager->active_app_name : "none");
    
    for (int i = 0; i < BUTTON_COUNT; i++) {
        const ButtonState_t *state = &manager->button_states[i];
        printf("  Button [%s]:\n", button_id_names[i]);
        printf("    Press Count: %lu\n", state->press_count);
        printf("    Pressed: %s\n", state->is_pressed ? "true" : "false");
        printf("    Has Double Click: %s\n", state->has_double_click ? "true" : "false");
        printf("    Has Long Press: %s\n", state->has_long_press ? "true" : "false");
    }
}

/* ================================================================
 *  NAVIGATION ACTION DISPATCHERS
 * ================================================================ */

void button_event_manager_dispatch_back(ButtonEventManager_t *manager)
{
    if (!manager) {
        return;
    }
    
    ButtonEvent_t event = {
        .button_id = BUTTON_BACK,
        .event_type = BUTTON_EVENT_CLICK,
        .timestamp_ms = lv_tick_get(),
        .user_data = NULL
    };
    
    dispatch_to_global(manager, &event);
    dispatch_to_nav(manager, &event);
}

void button_event_manager_dispatch_menu(ButtonEventManager_t *manager)
{
    if (!manager) {
        return;
    }
    
    ButtonEvent_t event = {
        .button_id = BUTTON_MENU,
        .event_type = BUTTON_EVENT_CLICK,
        .timestamp_ms = lv_tick_get(),
        .user_data = NULL
    };
    
    dispatch_to_global(manager, &event);
    dispatch_to_nav(manager, &event);
}

void button_event_manager_dispatch_select(ButtonEventManager_t *manager)
{
    if (!manager) {
        return;
    }
    
    ButtonEvent_t event = {
        .button_id = BUTTON_SELECT,
        .event_type = BUTTON_EVENT_CLICK,
        .timestamp_ms = lv_tick_get(),
        .user_data = NULL
    };
    
    dispatch_to_global(manager, &event);
    dispatch_to_app(manager, &event);
}

void button_event_manager_dispatch_rotate_up(ButtonEventManager_t *manager)
{
    if (!manager) {
        return;
    }
    
    ButtonEvent_t event = {
        .button_id = BUTTON_ENCODER_ROTATE,
        .event_type = BUTTON_EVENT_ROTATE_UP,
        .timestamp_ms = lv_tick_get(),
        .user_data = NULL
    };
    
    dispatch_to_global(manager, &event);
    dispatch_to_nav(manager, &event);
}

void button_event_manager_dispatch_rotate_down(ButtonEventManager_t *manager)
{
    if (!manager) {
        return;
    }
    
    ButtonEvent_t event = {
        .button_id = BUTTON_ENCODER_ROTATE,
        .event_type = BUTTON_EVENT_ROTATE_DOWN,
        .timestamp_ms = lv_tick_get(),
        .user_data = NULL
    };
    
    dispatch_to_global(manager, &event);
    dispatch_to_nav(manager, &event);
}
