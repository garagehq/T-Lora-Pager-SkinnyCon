# Factory App Logic Documentation

## Overview
The factory app provides a comprehensive testing and demonstration interface for the T-LoRa-Pager device. It includes multiple screens for testing various hardware components and UI features.

## Architecture

### Screen Management
The factory app uses LVGL 9.x screen management with the following key screens:

1. **Main Menu** - Entry point with icon-based navigation
2. **Clock Screen** - Real-time clock display with analog/digital modes
3. **Settings Screen** - Device configuration options
4. **LoRa Chat Screen** - LoRa communication testing
5. **Logo Screen** - Branding display
6. **Monitor Screen** - Hardware sensor monitoring
7. **Nametag Screen** - User identification display
8. **About Screen** - Device information and firmware version
9. **Code of Conduct Screen** - Usage guidelines
10. **Badgeshark Screen** - Achievement display
11. **Schedule Screen** - Event calendar
12. **Net Tools Screen** - Network utilities

### Navigation Flow
```
Main Menu
├── Clock (lvgl_test_screen_clock)
├── Settings (lvgl_test_screen_settings)
├── LoRa Chat (lvgl_test_screen_lora_chat)
├── Logo (lvgl_test_screen_logo)
├── Monitor (lvgl_test_screen_monitor)
├── Nametag (lvgl_test_screen_nametag)
├── About (lvgl_test_screen_about)
├── Code of Conduct (lvgl_test_screen_code_of_conduct)
├── Badgeshark (lvgl_test_screen_badgeshark)
├── Schedule (lvgl_test_screen_schedule)
└── Net Tools (lvgl_test_screen_net_tools)
```

## Key Functions

### lvgl_test_screen_main()
Main menu initialization with icon grid layout.
- Uses real icons (img_msgchat, img_monitoring, img_configuration, img_wifi)
- Implements flex layout for responsive design
- Handles button click events for navigation

### lvgl_test_screen_clock()
Clock display with analog and digital modes.
- Analog: Rotating hour/minute/second hands
- Digital: HH:MM:SS format
- Updates every second via lv_timer
- Handles 12/24 hour format switching

### lvgl_test_screen_monitor()
Hardware sensor monitoring screen.
- Displays GPS coordinates (latitude/longitude)
- Shows battery voltage and power status
- Monitors LoRa signal strength (RSSI)
- Real-time data refresh

### lvgl_test_screen_settings()
Device configuration interface.
- Brightness adjustment slider
- Theme selection (light/dark)
- Display timeout settings
- LoRa configuration options

## Event Handling

### Button Click Handlers
All screens use LVGL button click callbacks:
```c
static void button_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    
    if (code == LV_EVENT_CLICKED) {
        // Handle button click
        const char * label = lv_label_get_text(lv_obj_get_child(btn, NULL));
        // Navigate to corresponding screen
    }
}
```

### Touch Input
- Touch events trigger screen transitions
- Swipe gestures for back navigation
- Long press for context menus

## State Management

### Screen State Machine
The factory app maintains screen state through:
- Current screen identifier
- Previous screen for back navigation
- Screen initialization flags
- Data persistence across screen changes

### Persistent Settings
Settings are stored in NVS (Non-Volatile Storage):
- Brightness level (0-255)
- Theme selection
- LoRa parameters
- User preferences

## Testing Integration

### Unit Tests
Located in `test/test_factory_sim/test_factory_sim.c`:
- 15 test cases covering all main screens
- Font quality validation (Alibaba 12/24/40/100px)
- Icon rendering verification
- Screenshot generation for visual regression

### Simulation Mode
Uses headless LVGL simulator:
- 480×222 RGB565 framebuffer
- PPM screenshot export
- No hardware required for testing

## Known Issues

1. **Icon Version Compatibility**: `img_sports` and `img_calendar` are v8-only icons. Use `_v9.c` variants in LVGL 9 builds.

2. **Static Variable Collisions**: Multiple `.cpp` files may use same static variable names. Rename to unique names (e.g., `net_stats_label` instead of `stats_label`).

3. **UI_DEFINE_H Guards**: Include guards required in `ui_define.h` to prevent redefinition when multiple `.cpp` files are included.

## Development Guidelines

### Adding New Screens
1. Create new `lvgl_test_screen_*()` function
2. Implement screen initialization
3. Add event callbacks for interactive elements
4. Update main menu navigation
5. Add corresponding test case

### Icon Usage
- Use v9-compatible icons only
- Icons located in `examples/factory/icons/`
- Follow naming convention: `img_<name>_v9.c`

### Font Selection
- Primary: Alibaba (12/24/40/100px)
- Ensure proper font loading in simulator
- Test font quality in factory app

## References
- [LVGL Documentation](https://docs.lvgl.io/)
- [T-LoRa-Pager Hardware](../firmware/README.md)
- [Testing Guide](./testing.md)
