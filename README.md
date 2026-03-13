# T-Lora-Pager-SkinnyCon

LilyGo T-LoRa-Pager firmware for SkinnyCon conference with full UI, LoRa chat, and hardware monitoring.

## Quick Start

### Prerequisites
- PlatformIO CLI: `pip install platformio`
- ESP32-S3 device with 480x222 RGB565 display
- Rotary encoder (A, B, SW pins)
- I2C OLED display for status (optional)

### Build & Test

```bash
# Run native unit tests (no hardware needed)
pio test -e native -v

# Run LVGL headless simulator tests (generates PPM screenshots)
pio test -e native_lvgl -v

# Run factory app simulation (real fonts + icons, PPM screenshots)
pio test -e native_factory -v

# Run all tests
pio test -e native -e native_lvgl -e native_factory -v

# Compile for ESP32-S3 (build check only, no upload)
pio run -e tlora_pager

# Upload to device (requires connected T-Lora-Pager)
pio run -e tlora_pager --target upload
```

## Architecture

### 4-Tier Test Suite

| Tier | Environment | What It Tests | Tests | Speed |
|------|-------------|---------------|-------|-------|
| 1 | `native` | Brightness clamping, GPS NMEA parsing, RGB565 byte swap, power channels, hardware masks, rotary layout | 24 | <3s |
| 2 | `native_lvgl` | LVGL widgets, rendering, flex layouts, sliders, framebuffer, screenshots | 20 | ~7s |
| 3 | `native_factory` | Factory app screens with real fonts, 9 icons, 6 SkinnyCon screens | 15 | ~7s |
| 4 | `tlora_pager` | Full ESP32-S3 compilation (build check) | — | ~120s |

### UI Logic: Nametag/Clock Switching

The pager implements a dual-mode display system:

**Clock Screen** (default):
- Large digital time display
- Date and temperature overlay
- Auto-sleep after 30s idle
- Wake on rotary encoder press

**Nametag Screen** (conference mode):
- Editable name badge
- 5 display modes cycled via rotary:
  1. Name + subtitle
  2. Name + badge type
  3. Name + company
  4. Name + pronouns
  5. Name + social handles
- Persistent storage via EEPROM

**Screen Switching Logic**:
```c
// In ui_main.cpp - main loop handles screen transitions
if (clock_page_hidden && !nametag_active) {
    lv_obj_add_flag(clock_page, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(main_screen, LV_OBJ_FLAG_HIDDEN);
}

// Rotary encoder triggers mode switch
if (rotary_pressed) {
    nametag_cycle_mode();  // Cycle through 5 nametag modes
    clock_wakeup();         // Wake from sleep
}
```

### Configuration Parsing

**Config File Format** (`config.json`):
```json
{
  "device": {
    "brightness": 128,
    "sleep_timeout": 30,
    "oled_i2c_addr": 0x3C
  },
  "lora": {
    "frequency": 433,
    "spread_factor": 7,
    "power": 22
  },
  "user": {
    "name": "Alice",
    "badge_type": "Attendee",
    "pronouns": "she/her"
  }
}
```

**Parsing Implementation**:
```c
// In config_parser.cpp
bool parse_config_file(const char* path, config_t* config) {
    FILE* f = fopen(path, "r");
    if (!f) return false;
    
    char buffer[512];
    size_t len = fread(buffer, 1, sizeof(buffer)-1, f);
    buffer[len] = '\0';
    fclose(f);
    
    // Simple JSON parsing (no external deps)
    config->brightness = json_get_int(buffer, "brightness");
    config->sleep_timeout = json_get_int(buffer, "sleep_timeout");
    // ... more fields
    
    return true;
}
```

### LVGL Widget Rendering (Headless)

**Headless Simulator** (`test/simulator/`):
- LVGL 9.2 with `LV_STDLIB_CLIB` backend
- 480×222 RGB565 framebuffer
- PPM screenshot export for CI artifacts

**Key API**:
```c
// Setup/teardown per test
lvgl_sim_init();
lvgl_sim_deinit();

// Advance LVGL timers
lvgl_test_run(ms);  // e.g., 16ms for 60fps

// Export framebuffer as PPM
lvgl_test_save_ppm("screenshot.ppm");

// Direct framebuffer access for pixel verification
uint16_t* fb = lvgl_sim_get_framebuffer();
```

**Widget Rendering Tests**:
- Labels with variable fonts (12/24/40/100px)
- Buttons with click handlers
- Flex layouts (row/column)
- Sliders for brightness control
- Multi-widget layouts

### Hardware Abstraction Layer (Mocked)

**Mock Headers** (`test/mocks/`):
- `Arduino.h` - Mock Arduino functions
- `SPI.h` - Mock SPI bus
- `Wire.h` - Mock I2C bus
- `FreeRTOS.h` - Mock RTOS
- `ESP-IDF` - Mock GPIO/I2S/SPI

**Hardware Interface** (`lilygo-t-lora-pager.h`):
```c
class LilyGoPager {
public:
    // Display
    void setBrightness(uint8_t level);  // 0-255, clamped
    void clearDisplay();
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    
    // I2C OLED
    void initOLED();
    void oledPrint(const char* text);
    
    // LoRa
    void initLoRa();
    bool sendPacket(const uint8_t* data, size_t len);
    bool receivePacket(uint8_t* buf, size_t* len);
    
    // Rotary Encoder
    int getRotation();
    bool isPressed();
};
```

**Native Build Guards**:
```c
#ifdef NATIVE_BUILD
    // Test code - uses mocks
    #include "mocks/Arduino.h"
#else
    // ESP32-S3 hardware
    #include <Arduino.h>
    #include <Wire.h>
    #include <SPI.h>
#endif
```

## Test Files

- `test/test_hardware/test_hardware.c` — 24 tests: brightness, GPS, RGB565, power, hardware masks, rotary
- `test/test_lvgl_render/test_lvgl_render.c` — 20 tests: widgets, rendering, layouts, screenshots
- `test/test_factory_sim/test_factory_sim.c` — 15 tests: main menu, clock, settings, LoRa, badgeshark, schedule
- `test/simulator/sim_main.c/h` — Headless LVGL 9.2 simulator
- `test/mocks/` — Mock headers for C++ Arduino code

## Known Issues & Gotchas

### C vs C++ Compilation
- Test files are `.c` (C), app files are `.cpp` (C++)
- Cannot `#include` app `.cpp` files from C test files
- Tests must recreate UI logic independently

### Icon Versioning
- `img_sports` and `img_calendar` are v8-only
- Always use `_v9.c` variants (e.g., `img_msgchat`, `img_monitoring`)

### Static Variable Collisions
- Apps may use same `static` variable names
- Rename to unique names (e.g., `net_stats_label`) when both linked

### PlatformIO Guard
- `pio_main.cpp` guarded with `#ifdef PLATFORMIO`
- Prevents Arduino IDE duplicate symbol errors

## Contributing

1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Run `pio test -e native -e native_lvgl -e native_factory -v`
5. Submit PR

## License

MIT License - see LICENSE file for details.

## Acknowledgments

- LilyGo for T-LoRa-Pager hardware
- LVGL for widget library
- SkinnyCon conference organizers
- GarageHQ community contributors
