# UI Navigation Logic, Button Event Handlers, and Screen State Management

This document describes the UI navigation improvements implemented for T-Lora-Pager-SkinnyCon.

## Overview

This implementation provides three core modules for robust UI navigation:

1. **Screen State Management** - State tracking and transitions
2. **Button Event Handlers** - Hardware button abstraction and event dispatching
3. **Navigation Logic Helpers** - Tileview-based navigation patterns

All modules are designed to work together and integrate with LVGL 9.2.

---

## Module 1: Screen State Manager

**Location**: `application/SkinnyCon/src/screen_state_manager.{h,c}`  
**Test**: `test/test_native/test_screen_state.c`

### Purpose

Manages screen state transitions, navigation history, and power state integration for the T-LoRa-Pager UI.

### Key Features

- **Screen States**: IDLE, MENU, APP, APP_NESTED, LOW_POWER, POWER_OFF
- **Navigation History**: Stack-based history with max depth (10 records)
- **Callbacks**: Hooks for state changes, power events, app lifecycle
- **Timeouts**: Activity-based timeout detection for low power
- **Configuration**: Enable/disable history and callbacks per instance

### API Overview

```c
// Initialization
void screen_state_manager_init(ScreenStateManager_t *manager);

// State transitions
void screen_state_manager_transition(ScreenStateManager_t *manager,
                                      ScreenState_t new_state,
                                      TransitionType_t type,
                                      const char *app_name);

// State queries
ScreenState_t screen_state_manager_get_state(const ScreenStateManager_t *manager);
bool screen_state_manager_is_in_app(const ScreenStateManager_t *manager);
bool screen_state_manager_is_low_power(const ScreenStateManager_t *manager);

// Navigation history
bool screen_state_manager_has_history(const ScreenStateManager_t *manager);
bool screen_state_manager_navigate_back(ScreenStateManager_t *manager);
void screen_state_manager_clear_history(ScreenStateManager_t *manager);

// Timeout management
void screen_state_manager_update_activity(ScreenStateManager_t *manager, uint32_t tick);
bool screen_state_manager_check_timeout(const ScreenStateManager_t *manager, uint32_t tick);
```

### Integration Example

```c
// Initialize
ScreenStateManager_t screen_state;
screen_state_manager_init(&screen_state);

// Set callbacks
screen_state_manager_set_state_callback(&screen_state, my_state_callback);
screen_state_manager_set_app_enter_callback(&screen_state, my_app_enter);

// Transition to app
screen_state_manager_transition(&screen_state, SCREEN_STATE_APP, TRANSITION_MANUAL, "LoRa Chat");

// Poll for timeout
if (screen_state_manager_check_timeout(&screen_state, lv_tick_get())) {
    screen_state_manager_transition(&screen_state, SCREEN_STATE_LOW_POWER, TRANSITION_AUTO, NULL);
}
```

### State Machine Flow

```
IDLE → MENU → APP → APP_NESTED
  ↑              ↓
  └────── LOW_POWER ← timeout
        ↑
   activity → wake up
```

---

## Module 2: Button Event Handlers

**Location**: `application/SkinnyCon/src/button_event_handlers.{h,c}`  
**Test**: `test/test_native/test_button_events.c`

### Purpose

Centralized button event handling with hardware abstraction, providing unified event dispatch for navigation and application actions.

### Key Features

- **Button Types**: Encoder rotation/click, double-click, long-press, back, menu, select
- **Event Dispatch**: Global → Navigation → Application handler chain
- **Hardware Abstraction**: Mock-friendly interface for testing
- **App Context**: Track active application for scoped events
- **Navigation Dispatchers**: Helper functions for common actions

### API Overview

```c
// Initialization
void button_event_manager_init(ButtonEventManager_t *manager);

// Callbacks
void button_event_manager_set_global_handler(ButtonEventManager_t *manager,
                                              ButtonEventHandler_t handler);
void button_event_manager_set_nav_handler(ButtonEventManager_t *manager,
                                           ButtonEventHandler_t handler);
void button_event_manager_set_app_handler(ButtonEventManager_t *manager,
                                           ButtonEventHandler_t handler);

// Configuration
void button_event_manager_set_navigation_enabled(ButtonEventManager_t *manager,
                                                  bool enabled);
void button_event_manager_set_active_app(ButtonEventManager_t *manager,
                                          const char *app_name,
                                          void *context);

// Event dispatch
bool button_event_manager_handle_hw_event(ButtonEventManager_t *manager,
                                           const ButtonEvent_t *event);
void button_event_manager_poll(ButtonEventManager_t *manager);

// Navigation dispatchers
void button_event_manager_dispatch_back(ButtonEventManager_t *manager);
void button_event_manager_dispatch_menu(ButtonEventManager_t *manager);
void button_event_manager_dispatch_select(ButtonEventManager_t *manager);
```

### Event Handler Chain

```
Button Event
    ↓
[Global Handler] ← Called first, receives ALL events
    ↓
[Navigation Handler] ← If navigation enabled
    ↓
[App Handler] ← If app actions enabled and in app context
```

### Integration Example

```c
// Initialize
ButtonEventManager_t btn_manager;
button_event_manager_init(&btn_manager);

// Set handlers
button_event_manager_set_nav_handler(&btn_manager, on_nav_event);
button_event_manager_set_app_handler(&btn_manager, on_app_event);

// Enter app
button_event_manager_set_active_app(&btn_manager, "LoRa Chat", NULL);

// Poll (call from main loop or interrupt)
button_event_manager_poll(&btn_manager);

// Or dispatch common actions
button_event_manager_dispatch_back(&btn_manager);
```

---

## Module 3: Navigation Logic Helpers

**Location**: `application/SkinnyCon/src/nav_logic_helpers.{h,c}`  
**Test**: `test/test_native/test_nav_logic.c`

### Purpose

Reusable navigation logic for tileview-based UIs with support for multiple navigation modes.

### Key Features

- **Navigation Modes**:
  - **Direct**: Normal navigation, stops at boundaries
  - **Circular**: Wrap around at boundaries
  - **Tree**: Navigation stack for parent-child relationships

- **Tileview Helpers**: Get current tile, animate navigation, focus management

- **LVGL Event Handler**: Built-in encoder event handling for tileview

### API Overview

```c
// Context management
void nav_context_init(NavContext_t *context, lv_obj_t *tileview, NavMode_t mode);

// Navigation actions
bool nav_next(NavContext_t *context);
bool nav_prev(NavContext_t *context);
bool nav_back(NavContext_t *context);  // Tree mode aware
void nav_first(NavContext_t *context);
void nav_last(NavContext_t *context);
bool nav_goto(NavContext_t *context, uint32_t tile);

// Tileview helpers
uint32_t nav_get_tile_count(lv_obj_t *tileview);
uint32_t nav_get_current_visible_tile(lv_obj_t *tileview);
void nav_set_tile(lv_obj_t *tileview, uint32_t row, uint32_t col, bool anim);

// Focus management
bool nav_focus_next(lv_obj_t *tileview);
bool nav_focus_first(lv_obj_t *tileview);
void nav_set_default_focus(lv_obj_t *tileview);

// LVGL integration
void nav_create_event_handler(lv_obj_t *tileview, NavContext_t *context);
void nav_handle_rotate_up(lv_obj_t *tileview, NavContext_t *context);
void nav_handle_rotate_down(lv_obj_t *tileview, NavContext_t *context);
```

### Integration Example

```c
// Initialize context
NavContext_t nav_ctx;
nav_context_init(&nav_ctx, main_tileview, NAV_MODE_CIRCULAR);

// Set up encoder events
nav_create_event_handler(main_tileview, &nav_ctx);

// Or call dispatchers manually
nav_next(&nav_ctx);  // Right encoder rotation
nav_prev(&nav_ctx);  // Left encoder rotation
```

---

## Module Integration

### Recommended Architecture

```
┌─────────────────────────────────────────────────┐
│                 Main Loop                        │
├─────────────────────────────────────────────────┤
│                                                  │
│  ┌──────────────────────────────────────────┐  │
│  │           Screen State Manager           │  │
│  │  - Track current screen state           │  │
│  │  - Manage transitions                   │  │
│  │  - Handle timeouts                      │  │
│  └──────────────────────────────────────────┘  │
│                    ↑      ↓                     │
│  ┌──────────────────────────────────────────┐  │
│  │       Button Event Manager               │  │
│  │  - Receive hardware button events       │  │
│  │  - Dispatch to appropriate handler      │  │
│  └──────────────────────────────────────────┘  │
│                    ↑      ↓                     │
│  ┌──────────────────────────────────────────┐  │
│  │    Navigation Logic Helpers              │  │
│  │  - Tileview state management            │  │
│  │  - Focus handling                       │  │
│  └──────────────────────────────────────────┘  │
│                                                  │
└─────────────────────────────────────────────────┘
```

### Complete Integration Example

```c
// Global managers
ScreenStateManager_t screen_state;
ButtonEventManager_t btn_manager;
NavContext_t nav_ctx;

// Initialization (during app setup)
void app_init(void) {
    screen_state_manager_init(&screen_state);
    button_event_manager_init(&btn_manager);
    
    screen_state_manager_set_state_callback(&screen_state, on_state_change);
    button_event_manager_set_nav_handler(&btn_manager, on_nav_event);
    
    nav_context_init(&nav_ctx, main_tileview, NAV_MODE_DIRECT);
}

// Main loop
void app_loop(void) {
    // Check for low power timeout
    if (screen_state_manager_check_timeout(&screen_state, lv_tick_get())) {
        screen_state_manager_transition(&screen_state, 
                                         SCREEN_STATE_LOW_POWER, 
                                         TRANSITION_AUTO, 
                                         NULL);
    }
    
    // Poll for button events
    button_event_manager_poll(&btn_manager);
    
    // Handle encoder navigation (if using tileview)
    lv_group_t *group = lv_group_get_default();
    if (group) {
        lv_indev_t *indev = lv_indev_get_next(NULL);
        while (indev) {
            if (lv_indev_get_type(indev) == LV_INDEV_TYPE_ENCODER) {
                lv_point_t point;
                lv_indev_get_point(indev, &point);
                
                if (point.y < -5) {
                    nav_handle_rotate_up(main_tileview, &nav_ctx);
                } else if (point.y > 5) {
                    nav_handle_rotate_down(main_tileview, &nav_ctx);
                }
            }
            indev = lv_indev_get_next(indev);
        }
    }
    
    // Update activity (button press, touch, etc.)
    screen_state_manager_update_activity(&screen_state, lv_tick_get());
}

// Callbacks
void on_state_change(ScreenState_t new_state, ScreenState_t old_state) {
    printf("State changed: %s → %s\n", 
           screen_state_manager_get_state_name(old_state),
           screen_state_manager_get_state_name(new_state));
    
    // Update UI based on state
    if (new_state == SCREEN_STATE_LOW_POWER) {
        // Dim screen, reduce CPU frequency
    }
}

void on_nav_event(const ButtonEvent_t *event) {
    switch (event->event_type) {
        case BUTTON_EVENT_ROTATE_UP:
            nav_next(&nav_ctx);
            break;
        case BUTTON_EVENT_ROTATE_DOWN:
            nav_prev(&nav_ctx);
            break;
        case BUTTON_EVENT_CLICK:
            // Activate focused widget
            break;
    }
}
```

---

## Testing

### Running Tests

All tests run on the native platform (no ESP32 hardware required):

```bash
# Run all native tests
pio test -e native -v

# Run specific test groups
pio test -e native -t "test_screen_state*"
pio test -e native -t "test_button_events*"
pio test -e native -t "test_nav_logic*"
```

### Test Coverage

| Test File | Tests | Coverage |
|-----------|-------|----------|
| test_screen_state.c | 40+ | State transitions, history, timeouts, callbacks |
| test_button_events.c | 35+ | Event dispatching, state tracking, navigation |
| test_nav_logic.c | 30+ | Navigation modes, boundary conditions |

### Test Scenarios

**Screen State Manager Tests**:
- Initialization defaults
- State transitions (IDLE → MENU → APP)
- Navigation history recording
- Back navigation with history
- Low power transitions
- Activity timeout detection
- Callback enable/disable

**Button Event Handler Tests**:
- Event manager initialization
- Callback setting and execution
- Navigation enable/disable
- Active app context
- Button state tracking (click count, flags)
- Dispatcher functions (back, menu, select)
- State clearing

**Navigation Logic Tests**:
- Direct mode navigation
- Circular mode wrapping
- Tree mode boundary behavior
- First/last navigation
- Tile counting
- Focus management
- Complex navigation scenarios

---

## Files Modified/Created

### New Files
- `application/SkinnyCon/src/screen_state_manager.h` - Screen state API
- `application/SkinnyCon/src/screen_state_manager.c` - Screen state implementation
- `application/SkinnyCon/src/button_event_handlers.h` - Button events API
- `application/SkinnyCon/src/button_event_handlers.c` - Button events implementation
- `application/SkinnyCon/src/nav_logic_helpers.h` - Navigation helpers API
- `application/SkinnyCon/src/nav_logic_helpers.c` - Navigation helpers implementation
- `test/test_native/test_screen_state.c` - Screen state tests
- `test/test_native/test_button_events.c` - Button event tests
- `test/test_native/test_nav_logic.c` - Navigation logic tests

### Modified Files
- `platformio.ini` - Added test filters and include paths for new modules

---

## Design Principles

1. **Testability**: All modules can run on native platform without hardware
2. **Separation of Concerns**: State, events, and navigation are separate modules
3. **Hardware Abstraction**: Button handlers abstract hardware interfaces
4. **Configurability**: Navigation modes, timeouts, and callbacks are configurable
5. **Memory Efficiency**: Fixed-size history stacks, no dynamic allocation
6. **LVGL Integration**: Designed to work seamlessly with LVGL 9.2 widgets

---

## Future Improvements

- Add state machine diagram documentation
- Implement app-specific navigation presets
- Add gesture detection integration
- Create LVGL widget navigation adapter
- Add performance metrics for state transitions

---

## References

- [LVGL 9.2 Documentation](https://docs.lvgl.io/)
- [T-LoRa-Pager-SkinnyCon GitHub](https://github.com/garagehq/T-Lora-Pager-SkinnyCon)
- [PlatformIO Documentation](https://docs.platformio.org/)
