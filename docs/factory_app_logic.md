# Factory App Logic

## Overview
The factory app provides a comprehensive diagnostic and configuration interface for the T-LoRa-Pager device. It runs on ESP32-S3 with LVGL 9.2 for the display interface.

## Architecture

### Main Entry Point
- **File**: `examples/factory/factory.ino`
- **Class**: `FactoryApp` instance
- **Loop**: Handles LVGL timer processing and device state management

### Key Components

#### Hardware Abstraction Layer (HAL)
- **Interface**: `hal_interface.h`
- **Purpose**: Abstracts hardware operations for testability
- **Functions**: 
  - `hw_get_device_online()` - Returns hardware presence bitmask
  - `hw_set_disp_backlight(uint8_t level)` - Sets display brightness
  - `hw_get_disp_backlight()` - Returns current brightness level
  - `hw_get_battery_voltage()` - Returns battery voltage in mV
  - `hw_get_wifi_status()` - Returns WiFi connection status
  - `hw_get_gps_info(gps_params_t &param)` - Retrieves GPS data

#### LVGL Display Management
- **Framebuffer**: 480×222 RGB565 (physical display resolution)
- **Screen Navigation**: Flex layout with scrollable content
- **Widgets**: Labels, buttons, sliders, images, lists

#### Event System
- **File**: `event_define.h`
- **Events**: Keyboard input, rotary encoder, touch events
- **Processing**: Event queue with priority handling

## Screen Flow

### 1. Main Menu
- **Icons**: msgchat, monitoring, configuration, wifi (v9 variants)
- **Options**:
  - Clock Screen
  - Settings
  - LoRa Chat
  - Logo Screen
  - Monitor
  - Nametag
  - About
  - Code of Conduct
  - Badgeshark
  - Schedule
  - Net Tools

### 2. Clock Screen
- **Display**: Real-time clock with date
- **Format**: 12/24 hour toggleable
- **Timezone**: GMT offset configurable

### 3. Settings Screen
- **Brightness**: Slider control (0-255)
- **Volume**: Media key control
- **WiFi**: Network configuration
- **GPS**: Position settings

### 4. LoRa Chat Screen
- **Protocol**: SX12XX radio (868 MHz default)
- **Fixed Frequency**: 920 MHz option available
- **Message Display**: Scrollable text area

### 5. Monitor Screen
- **Hardware Status**: Online/offline indicators
- **Sensor Data**: Temperature, pressure, GPS coordinates
- **Battery**: Voltage and charge level

## Testing

### Native Simulation
```bash
pio test -e native_factory -v
```

### Test Coverage
- 15 factory simulation tests
- Real fonts: Alibaba 12/24/40/100px
- 9 icons loaded from assets
- 6 SkinnyCon screens rendered

### PPM Screenshots
- Screenshots saved as PPM files
- Visual regression testing in CI
- Framebuffer verification via `lvgl_sim_get_framebuffer()`

## Known Issues

### Icon Version Compatibility
- v8 icons (`img_sports`, `img_calendar`) not available in LVGL 9
- Use v9 variants (`img_msgchat`, `img_monitoring`, etc.)

### Static Variable Collisions
- Multiple `.cpp` files may use same `static` variable names
- Rename to unique names (e.g., `net_stats_label` vs `stats_label`)

### UI_DEFINE_H Guards
- Must include `#ifndef UI_DEFINE_H` guard
- Prevents redefinition when multiple files include the header

## Data Flow

```
User Input (Rotary/Touch)
    ↓
Event Queue
    ↓
FactoryApp::loop()
    ↓
Screen Handler
    ↓
LVGL Widgets Update
    ↓
Framebuffer Render
    ↓
Display Output (480×222 RGB565)
```

## Hardware Dependencies

### Required Components
- ESP32-S3 microcontroller
- ST7789 display (480×222)
- SX12XX LoRa radio
- XPowers PMU (battery monitoring)
- BHI260AP sensor hub
- QMI8658 accelerometer
- RTC module

### Optional Components
- GPS module (Dallas/Sao Paulo)
- NFC reader (ST25R3916)
- SD card interface
- Keyboard module

## Build Configuration

### PlatformIO Environment
```ini
[env:tlora_pager]
platform = espressif32
board = t-lora-pager
framework = arduino
build_flags = 
    -D ARDUINO_T_LORA_PAGER
    -D USING_ST25R3916
    -D RADIO_FIXED_FREQUENCY=920.0
```

### LVGL Configuration
- `LV_STDLIB_CLIB` for native builds
- `LV_STDLIB_CUSTOM` for ESP32 builds
- 480×222 resolution
- RGB565 color depth
