<div align="center" markdown="1">
  <img src=".github/LilyGo_logo.png" alt="LilyGo logo" width="100"/>
</div>

<h1 align="center">T-LoRa-Pager (SkinnyCon Fork)</h1>

<p align="center">
  A handheld LoRa messaging device with a 480x222 ultra-wide display, full QWERTY keyboard, GPS, NFC, and 20+ built-in apps.
</p>

<p align="center">
  <img src="https://github.com/garagehq/T-Lora-Pager-SkinnyCon/actions/workflows/tests.yml/badge.svg" alt="Tests & Build">
</p>

## Hardware Overview

| Component | Chip | Interface | Notes |
|-----------|------|-----------|-------|
| **MCU** | ESP32-S3 | — | Xtensa dual-core 240 MHz, PSRAM |
| **Display** | ST7796U (480x222) | SPI | Ultra-wide IPS, AW9364 backlight (0-16 levels) |
| **Radio** | SX1262 | SPI | LoRa, also supports SX1280, CC1101, LR1121, Si4432, nRF24L01 |
| **GPS** | u-blox MIA-M10Q | UART | NMEA via TinyGPSPlus |
| **NFC** | ST25R3911B | SPI | RFAL stack, WiFi/URL/text tags |
| **IMU** | BHI260AP | SPI | 6-axis accelerometer + gyroscope |
| **Keyboard** | TCA8418 | I2C | Full QWERTY matrix with backlight |
| **Audio** | ES8311 | I2S | Mono codec, PDM microphone input |
| **Haptic** | DRV2605 | I2C | 124 waveform effects |
| **PMU** | BQ25896 | I2C | 128-2048 mA charge current, 64 mA steps |
| **Fuel Gauge** | BQ27220 | I2C | Battery %, voltage, current, time-to-empty |
| **RTC** | PCF85063 | I2C | Alarm, timer, battery backup |
| **Rotary** | Mechanical | GPIO | Encoder + center button |
| **Storage** | MicroSD | SPI | FAT32, music playback |

## Quick Start

### Prerequisites

- [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation.html)

### Install

```bash
git clone https://github.com/garagehq/T-Lora-Pager-SkinnyCon.git
cd T-Lora-Pager-SkinnyCon
pip install platformio
```

### Build & Flash

```bash
# Compile for ESP32-S3
pio run -e tlora_pager

# Upload to device
pio run -e tlora_pager --target upload

# Serial monitor
pio device monitor -b 115200
```

### Run Tests

```bash
# Run all tests (no hardware required)
pio test -e native -e native_lvgl -e native_factory -v

# Run individual test suites
pio test -e native -v          # Hardware logic tests (24 tests, <3s)
pio test -e native_lvgl -v     # LVGL widget tests (20 tests, ~7s)
pio test -e native_factory -v  # Factory app simulation (15 tests, ~7s)
```

## Testing Architecture

### 4-Tier Strategy

| Tier | Environment | Tests | What It Tests | Speed |
|------|-------------|-------|---------------|-------|
| 1 - Unit | `native` | 27 | Brightness clamping, GPS NMEA parsing (6 cases), RGB565 byte swap + channel extraction, power channel contiguity, hardware mask bitfields, rotary struct layout, LoRa frequency validation | <3s |
| 2 - Widgets | `native_lvgl` | 20 | LVGL 9.2 display init, labels, buttons, click handlers, flex layouts, sliders, rendering to framebuffer, PPM screenshot export | ~7s |
| 3 - Factory Sim | `native_factory` | 15 | Real factory app screens with actual Alibaba fonts (12/24/40/100px), 9 app icons, and 6 SkinnyCon screens | ~7s |
| 4 - Build | `tlora_pager` | — | Full ESP32-S3 compilation against real Arduino SDK and vendor libraries | ~60s |

| 5 - Interaction | `native_factory` | 8 | Real nametag app code with stubs: keyboard typing, tab/backspace, encoder click, mode rotation, group lifecycle | ~4s |

**Total: 98 tests, all running without hardware.**

### LVGL Headless Simulator

The simulator renders to a 480x222 RGB565 framebuffer matching the physical display. No SDL or graphics dependencies required.

```c
// Test API
lvgl_sim_init();                        // Initialize headless display
lvgl_test_run(200);                     // Advance LVGL timers by 200ms
lvgl_test_save_ppm("screenshot.ppm");   // Export framebuffer as PPM image
lvgl_sim_get_framebuffer();             // Direct pixel access for verification
lvgl_sim_deinit();                      // Cleanup
```

### Factory App Simulation

The `native_factory` environment renders actual factory app screens using real assets from `application/SkinnyCon/src/`:

- **Fonts**: Alibaba PuHuiTi Bold at 12px, 24px, 40px, and 100px (same fonts used on device)
- **Icons**: 9 app icons (Settings, Radio, Chat, Music, Monitor, Keyboard, Bluetooth, WiFi, Test)
- **Screens**: Main menu with tileview, clock screen, settings panel, LoRa chat, system monitor, plus 6 SkinnyCon screens (Nametag, About, Code of Conduct, BadgeShark, Schedule, Net Tools)

Screenshots are saved as PPM files and uploaded as CI artifacts for visual regression review.

### Test Files

| File | Tests | Description |
|------|-------|-------------|
| `test/test_hardware/test_hardware.c` | 27 | Brightness clamping, GPS NMEA parsing, RGB565 byte swap, power channels, hardware masks, rotary input, LoRa frequency |
| `test/test_lvgl_render/test_lvgl_render.c` | 20 | LVGL widgets, rendering, framebuffer verification |
| `test/test_factory_sim/test_factory_sim.c` | 17 | Factory app screens with real fonts, icons, and SkinnyCon screens |
| `test/test_app_interaction/test_app_interaction.cpp` | 8 | Real nametag code: typing, tab, backspace, click, rotate, group lifecycle |
| `test/test_screen_state/test_screen_state.c` | 46 | Screen state manager unit tests |
| `test/simulator/sim_main.c` | — | Headless LVGL 9.2 simulator (480x222 RGB565) |
| `test/simulator/lv_conf.h` | — | LVGL config for simulator |
| `test/mocks/` | — | Mock headers for Arduino, SPI, Wire, FreeRTOS, ESP-IDF |

## SkinnyCon Badge App

The main application (`application/SkinnyCon/`) is the SkinnyCon conference badge with 20+ screens:

### Navigation

- **Tileview layout**: Tile[0,0] = App menu (horizontal scroll), Tile[0,1] = Active app
- **Rotary encoder**: Scroll through apps, click to select
- **Keyboard**: Full QWERTY input for messaging and text entry
- **Auto-sleep**: 10s inactivity -> clock screen (80 MHz low power), display timeout configurable

### Built-in Apps

| App | Icon | Description |
|-----|------|-------------|
| **Nametag** | Person | Editable nametag with 5 modes: name (keyboard editable), fullscreen, about, code of conduct, badge info. ESC to return to menu |
| **Schedule** | Calendar | SkinnyCon 2026 3-day schedule browser (May 12-14, Huntsville AL). ESC to return to menu |
| **BadgeShark** | Monitor | LoRa packet sniffer — Wireshark-style hex dump with RSSI, auto-scroll. ESC to return to menu |
| **Net Tools** | Diagnostic | LoRa mesh diagnostics — ping/pong latency, packet loss, peer discovery. ESC to return to menu |
| LoRa | Radio | SX1262 radio TX/RX with configurable frequency, bandwidth, power, SF |
| LoRa Chat | Chat | Text messaging over LoRa with RSSI display |
| Setting | Gear | Brightness, keyboard backlight, charge current, WiFi, display timeout |
| Wireless | WiFi | WiFi scan, connect, status, IP address |
| Music | Note | SD card music player with FFT visualizer |
| GPS | Globe | Satellite tracking, coordinates, speed, NMEA debug |
| Power | Battery | Charger control, OTG toggle, current settings |
| Microphone | Mic | Audio input with FFT frequency bands visualization |
| IMU | Gyro | BHI260AP 6-axis orientation, roll/pitch/heading |
| Keyboard | Keys | Keyboard test mode |
| Bluetooth | BT | BLE serial communication |
| NFC | NFC | ST25R3911B tag reading (WiFi config, URLs, text) |
| IR Remote | IR | IR transmit/receive (if hardware present) |
| Compass | Compass | QMC5883P magnetic heading (if hardware present) |

### SkinnyCon Apps

Built for [SkinnyCon 2026](https://skinnycon.com) (May 12-14, Huntsville AL), these apps turn the T-LoRa-Pager into a conference badge. Design inspired by the [2025 Supercon badge](https://github.com/zx96/2025-supercon-badge).

- **Nametag**: Editable name badge — type your name on the QWERTY keyboard. 5 display modes cycled via rotary: name+subtitle (editable), fullscreen name, About SkinnyCon, Code of Conduct, and badge hardware info. SkinnyCon light theme (`#EFF6F6` bg, `#F96123` orange titles, `#5BBEC0` teal). Tab switches between name and subtitle editing. Encoder click to go back.
- **BadgeShark**: LoRa packet sniffer with teal-tinted theme. Shows packet number, length, RSSI (color-coded red/green by signal strength), and hex dump preview. Auto-scrolling list with max 50 packets, real-time stats bar. LVGL menu with back button.
- **Schedule**: SkinnyCon 2026 3-day schedule browser. Rotate scrolls through talks, wraps to next/prev day at boundaries. Each entry shows time and talk title. Break sessions dimmed. LVGL menu with back button.
- **Net Tools**: LoRa mesh diagnostic tool with two-panel layout. Left panel shows ping/pong results with round-trip time and color-coded status (green=fast, yellow=slow, red=timeout). Right panel lists discovered peers with RSSI. Stats bar shows sent/recv/loss%/avg RTT. LVGL menu with back button.

### Idle Screen (Low Power Mode)

After 10 seconds of inactivity, the device shows the badge idle screen:
- If user has set their name: large name centered with subtitle below
- If name is default ("YOUR NAME"): SKINNYCON logo with teal circle O
- Footer bar with time, date, and battery percentage
- CPU downclocked to 80 MHz for power savings
- Any input wakes back to app menu at 240 MHz

## CI/CD

| Workflow | Jobs | Trigger |
|----------|------|---------|
| `tests.yml` | Native Tests + LVGL Simulator + Factory Sim + ESP32 Build | PR, push to master |
| `release.yml` | ESP32-S3 firmware build + artifact upload | Push to master, manual |

### Firmware Downloads

Every merge to `master` triggers a firmware build. Download the latest artifacts from the [Actions tab](../../actions/workflows/release.yml):

- `tlora-pager-firmware-<sha>.bin` — Main firmware (flash at 0x10000)
- `tlora-pager-bootloader-<sha>.bin` — Bootloader (flash at 0x0)
- `tlora-pager-partitions-<sha>.bin` — Partition table (flash at 0x8000)

Artifacts are retained for 90 days. To flash manually:

```bash
esptool.py --chip esp32s3 --port /dev/ttyUSB0 --baud 921600 \
  write_flash 0x0 tlora-pager-bootloader-<sha>.bin \
              0x8000 tlora-pager-partitions-<sha>.bin \
              0x10000 tlora-pager-firmware-<sha>.bin
```

## Project Structure

```
T-Lora-Pager-SkinnyCon/
├── application/
│   ├── SkinnyCon/            # Main SkinnyCon badge application (20+ screens)
│   │   ├── SkinnyCon.ino     # Entry point (setup + loop) — Arduino IDE
│   │   ├── pio_main.cpp      # PlatformIO entry (#ifdef PLATFORMIO guard)
│   │   ├── ui_main.cpp       # Tileview navigation, menu, idle screen
│   │   ├── ui_tools.cpp      # Widget builders (slider, switch, button, etc.)
│   │   ├── ui_define.h       # Public UI API
│   │   ├── hal_interface.h   # Hardware abstraction layer (50+ functions)
│   │   ├── event_define.h    # Event/message IDs
│   │   └── src/              # Assets
│   │       ├── font/         # Alibaba PuHuiTi Bold (12/24/40/60/70/100px)
│   │       ├── img_*.c       # 56 compiled image assets (icons, backgrounds)
│   │       └── ui_*.cpp      # Per-app screen implementations
│   ├── base/                 # Basic peripheral examples
│   ├── lvgl/                 # LVGL widget examples (42 sketches)
│   └── radio/                # Radio examples (SX1262, SX1280, nRF24, etc.)
├── src/                      # LilyGoLib library source
│   ├── LilyGoLib.h           # Main library header
│   └── lv_conf.h             # LVGL config for ESP32 (LV_STDLIB_CUSTOM)
├── test/
│   ├── test_hardware/        # Tier 1: Hardware logic tests
│   ├── test_lvgl_render/     # Tier 2: LVGL widget tests
│   ├── test_factory_sim/     # Tier 3: Factory app simulation
│   ├── simulator/            # Headless LVGL simulator
│   └── mocks/                # Arduino/ESP-IDF mock headers
├── platformio.ini            # Build configurations (4 environments)
├── MARISOL.md                # Pipeline context document
└── .github/workflows/        # CI workflows (5 total)
```

## Display Specifications

| Property | Value |
|----------|-------|
| Resolution | 480 x 222 pixels (ultra-wide landscape) |
| Color depth | RGB565 (16-bit, 65,536 colors) |
| Controller | ST7796U |
| Interface | SPI |
| Backlight | AW9364, 17 levels (0-16) |
| GUI Framework | LVGL 9.2 |

## Power Management

| Feature | Details |
|---------|---------|
| Charger | BQ25896, 128-2048 mA in 64 mA steps |
| Fuel Gauge | BQ27220, battery %, voltage, current, time-to-empty |
| Sleep modes | Clock mode (80 MHz), display off, light sleep |
| Display timeout | Configurable via Settings app |
| Low battery | Auto-shutdown at 3300 mV with warning screen |

## Radio Configuration

The default radio is SX1262 (LoRa). Configurable parameters:

| Parameter | Range | Default |
|-----------|-------|---------|
| Frequency | 410-525 MHz, 862-1020 MHz | 915.0 MHz (US ISM band) |
| Bandwidth | 7.8-500 kHz | 125 kHz |
| Spreading Factor | 5-12 | 9 |
| TX Power | -9 to 22 dBm | 10 dBm |
| Coding Rate | 4/5 to 4/8 | 4/5 |

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Can't flash | Hold BOOT button while pressing RESET, then upload |
| Display blank | Check SPI connections, verify backlight level > 0 |
| No LoRa signal | Verify antenna connected, check frequency matches receiver |
| GPS no fix | Move outdoors, wait 30-60s for cold start acquisition |
| Keyboard unresponsive | Check I2C address (TCA8418), verify `USING_INPUT_DEV_KEYBOARD` defined |
| Build fails | Run `pio run -e tlora_pager` and check errors |
| Tests fail | Run `pio test -e native -v` first to verify PlatformIO setup |

## Additional Documentation

- [T-LoRa-Pager Hardware Information](./docs/hardware/lilygo-t-lora-pager.md)
- [GPS AssistNow Usage Guide](./docs/assistNow/assistNow.md)
- [Flash Recovery Steps](./firmware/README.md)

## Theme

SkinnyCon light theme (defined in `application/SkinnyCon/ui_skinnycon_theme.h`):

| Color | Hex | Usage |
|-------|-----|-------|
| Background | `#EFF6F6` | Main screen bg |
| Teal | `#BDE4E6` | Headers, panels, accent bars |
| Logo teal | `#5BBEC0` | SKINNYCON circle O, icon borders |
| Orange | `#F96123` | Titles only |
| Text | `#000000` | Primary text |
| Dim | `#5A6672` | Secondary/hint text |

All 12 menu icons are custom LVGL-drawn (no PNG assets).

## Credits

- **Conference**: [SkinnyCon 2026](https://skinnycon.com) — Skinny Research and Development
- **Badge firmware**: Cyril Engmann / [The Garage Agency, LLC](https://thegarage.dev/)
- **Hardware**: [LilyGo T-LoRa-Pager](https://www.lilygo.cc/)
- **Base firmware**: Lewis He / ShenZhen XinYuan Electronic Technology Co., Ltd

## License

MIT License - See [LICENSE](./LICENSE) for details.
