# SkinnyCon 2026 Badge — T-LoRa-Pager

Conference badge firmware for **SkinnyCon 2026** (May 12-14, Huntsville AL) running on the [LilyGo T-LoRa-Pager](https://www.lilygo.cc/).

## Hardware

| Component | Detail |
|-----------|--------|
| MCU | ESP32-S3 (dual-core 240 MHz, PSRAM) |
| Display | 480x222 IPS LCD (ST7796U) |
| Radio | SX1262 LoRa 915 MHz |
| GPS | u-blox MIA-M10Q |
| NFC | ST25R3911B |
| Input | Rotary encoder + QWERTY keyboard |
| Battery | LiPo with BQ25896 PMU |

## Apps

| App | Description |
|-----|-------------|
| **Nametag** | Editable name badge with 5 modes (name, fullscreen, about, CoC, badge info) |
| **Schedule** | 3-day conference schedule browser |
| **BadgeShark** | LoRa packet sniffer (Wireshark-style) |
| **Net Tools** | LoRa mesh ping/pong diagnostics |
| **LoRa** | Radio configuration and testing |
| **LoRa Chat** | Text messaging over LoRa |
| **Settings** | Display brightness, charger, system info |
| **Wireless** | WiFi configuration |
| **GPS** | Satellite tracking and location |
| **Power** | Shutdown/sleep controls |
| **Microphone** | Audio spectrum analyzer |
| **IMU** | Accelerometer/gyroscope display |

## Build

```bash
# Install PlatformIO
pip install platformio

# Run all native tests (no hardware needed)
pio test -e native
pio test -e native_factory

# Build for hardware
pio run -e tlora_pager

# Upload to device
pio run -e tlora_pager --target upload

# Serial monitor
pio device monitor -b 115200
```

## Tests

98 automated tests across 4 test suites:

| Suite | Count | Description |
|-------|-------|-------------|
| `test_hardware` | 27 | Hardware constants, data structures |
| `test_screen_state` | 46 | Screen state manager logic |
| `test_factory_sim` | 17 | Screenshot rendering of all app screens |
| `test_app_interaction` | 8 | Real app code interaction (typing, scrolling, navigation) |

The interaction tests include actual app source code (`ui_nametag.cpp`) with hardware stubs, testing real keyboard input, encoder navigation, and group lifecycle.

```bash
# Run specific test suite
pio test -e native --filter test_hardware
pio test -e native --filter test_screen_state
pio test -e native_factory --filter test_factory_sim
pio test -e native_factory --filter test_app_interaction
```

## Theme

SkinnyCon light theme with brand colors:

| Color | Hex | Usage |
|-------|-----|-------|
| Background | `#EFF6F6` | Main screen bg |
| Teal | `#BDE4E6` | Headers, panels, accent bars |
| Logo teal | `#5BBEC0` | SKINNYCON circle O |
| Orange | `#F96123` | Titles only |
| Text | `#000000` | Primary text |
| Dim | `#5A6672` | Secondary text |

All 12 menu icons are custom LVGL-drawn (no PNG assets).

## Navigation

- **Encoder rotate**: Scroll through items / switch nametag modes
- **Encoder click**: Select item / go back (nametag) / confirm edit
- **Menu back button**: Scroll up to back arrow and click (Settings, Schedule, BadgeShark, Net Tools)
- **Keyboard**: Type to edit nametag name/subtitle

## Credits

- **Conference**: SkinnyCon 2026 — Skinny Research and Development
- **Badge firmware**: Cyril Engmann / [The Garage Agency, LLC](https://thegarage.dev/)
- **Hardware**: [LilyGo T-LoRa-Pager](https://www.lilygo.cc/)
- **Base firmware**: Lewis He / ShenZhen XinYuan Electronic Technology Co., Ltd
