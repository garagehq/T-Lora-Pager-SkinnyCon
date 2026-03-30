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

- [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation.html) (for testing and native builds)
- [Arduino IDE](https://www.arduino.cc/en/software) or [arduino-cli](https://arduino.github.io/arduino-cli/) (for flashing)
- ESP32 Arduino Core >= 3.3.0

### Install

```bash
# Clone the repository
git clone https://github.com/garagehq/T-Lora-Pager-SkinnyCon.git
cd T-Lora-Pager-SkinnyCon

# Install PlatformIO (if not already installed)
pip install platformio
```

### Build & Flash

```bash
# Compile for ESP32-S3 (build check only)
pio run -e tlora_pager

# Upload to device (requires connected T-LoRa-Pager via USB)
pio run -e tlora_pager --target upload

# Or use Arduino IDE:
# 1. Add ESP32 board URL: https://espressif.github.io/arduino-esp32/package_esp32_dev_index.json
# 2. Install esp32 board package
# 3. Select board: ESP32S3 Dev Module
# 4. Set: USB Mode=Hardware CDC, Upload Speed=921600, Partition=app3M_fat9M_16MB
# 5. Open application/SkinnyCon/SkinnyCon.ino and upload
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

**Total: 62 tests, all running without hardware.**

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
| `test/test_factory_sim/test_factory_sim.c` | 15 | Factory app screens with real fonts, icons, and SkinnyCon screens |
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

- **Nametag**: Editable name badge — type your name on the QWERTY keyboard. 5 display modes cycled via rotary: name+subtitle (editable), fullscreen giant name, About SkinnyCon, Code of Conduct, and badge hardware info. Hackaday color palette (grey `#1A1A1A`, yellow `#E39810`, sage green `#ABC5A0`). Tab switches between name and subtitle editing.
- **BadgeShark**: LoRa packet sniffer with terminal-style dark theme. Shows packet number, length, RSSI (color-coded red/green by signal strength), and hex dump preview. Auto-scrolling list with max 50 packets, real-time stats bar.
- **Schedule**: SkinnyCon 2026 3-day schedule browser. Left/Right arrows switch days (Tue/Wed/Thu). Each entry shows time and talk title. Keyboard/rotary Up/Down navigates with visual highlight. Break sessions dimmed.
- **Net Tools**: LoRa mesh diagnostic tool with two-panel layout. Left panel shows ping/pong results with round-trip time and color-coded status (green=fast, yellow=slow, red=timeout). Right panel lists discovered peers with RSSI. Stats bar shows sent/recv/loss%/avg RTT.

### Idle Screen (Low Power Mode)

After 10 seconds of inactivity, the device shows the SkinnyCon badge screen:
- Large "SKINNYCON" conference name in accent yellow
- "2026  Huntsville, AL" subtitle in green
- Footer bar with time, date, and battery percentage
- CPU downclocked to 80 MHz for power savings
- Any input wakes back to app menu at 240 MHz

## CI/CD

### Workflows

| Workflow | Jobs | Trigger |
|----------|------|---------|
| `tests.yml` | Native Tests + LVGL Simulator + Factory Sim + ESP32 Build | PR, push to master |
| `base_examples_ci.yml` | Arduino compile: 31 base sketches x tlora_pager | PR, push |
| `lvgl_examples_ci.yml` | Arduino compile: 42 LVGL widget examples x tlora_pager | PR, push |
| `radio_examples_ci.yml` | Arduino compile: 13 radio examples x tlora_pager | PR, push |
| `release.yml` | ESP32-S3 firmware build + artifact upload | Push to master, manual |

**Board matrix**: `tlora_pager` only (this fork focuses exclusively on the T-LoRa-Pager).

**CI artifacts**: PPM screenshots from factory simulation are uploaded for visual review on every PR.

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

## Dual Build System

This project has **two independent build paths** — understanding this is critical:

| Build Tool | Used For | Config |
|------------|----------|--------|
| **PlatformIO** | Local dev, unit tests (`native`, `native_lvgl`, `native_factory`), ESP32-S3 compilation (`tlora_pager`) | `platformio.ini` |
| **Arduino CLI** | GitHub Actions CI for all example sketches (31 base + 42 LVGL + 13 radio = 86 sketches) | `arduino-cli compile --library .` |

**Key difference**: Arduino CLI uses `--library .` which compiles everything in `src/` for ALL sketches. PlatformIO uses `build_src_filter` to selectively include `application/SkinnyCon/`. Never put example-specific code in `src/`.

**`pio_main.cpp`**: PlatformIO doesn't auto-process `.ino` files outside `src_dir`, so `application/SkinnyCon/pio_main.cpp` wraps `SkinnyCon.ino` with `#include "SkinnyCon.ino"`. It's guarded with `#ifdef PLATFORMIO` so Arduino IDE (which compiles all `.cpp` in the sketch directory) sees an empty translation unit — no duplicate `setup()`/`loop()` definitions.

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Can't flash | Hold BOOT button while pressing RESET, then upload |
| Display blank | Check SPI connections, verify backlight level > 0 |
| No LoRa signal | Verify antenna connected, check frequency matches receiver |
| GPS no fix | Move outdoors, wait 30-60s for cold start acquisition |
| Keyboard unresponsive | Check I2C address (TCA8418), verify `USING_INPUT_DEV_KEYBOARD` defined |
| Build fails | Ensure ESP32 Arduino Core >= 3.3.0, install all lib_deps |
| Tests fail | Run `pio test -e native -v` first to verify PlatformIO setup |
| Arduino CI: duplicate symbols | Check `pio_main.cpp` has `#ifdef PLATFORMIO` guard |
| Arduino CI: transient 404s | Espressif package server intermittent — re-run failed jobs |

## Additional Documentation

- [T-LoRa-Pager Arduino IDE Quick Start](./docs/lilygo-t-lora-pager.md)
- [T-LoRa-Pager Hardware Information](./docs/hardware/lilygo-t-lora-pager.md)
- [GPS AssistNow Usage Guide](./docs/assistNow/assistNow.md)
- [Flash Recovery Steps](./firmware/README.md)
- [Third-Party Dependencies](./docs/third_party.md)

## License

MIT License - See [LICENSE](./LICENSE) for details.

---

## Contributing Guidelines

### Code Style

- Use 4-space indentation
- Follow Arduino IDE coding conventions for .ino files
- C++ files should follow Google C++ Style Guide
- Function names: camelCase (e.g., `initializeDisplay()`)
- Class names: PascalCase (e.g., `ScreenStateManager`)
- Constants: UPPER_SNAKE_CASE (e.g., `MAX_BRIGHTNESS_LEVEL`)

### Commit Message Convention

```
<type>: <subject>

[optional body]

[optional footer]
```

**Types**: feat, fix, docs, style, refactor, test, chore, perf

**Examples**:
- `feat: add Nametag editable mode with keyboard input`
- `fix: correct GPS NMEA parsing for GGA sentences`
- `docs: update hardware specifications table`
- `test: add test case for brightness clamping edge cases`

### Pull Request Requirements

1. All tests passing (`pio test -e native -v`)
2. LVGL simulator tests passing (`pio test -e native_lvgl -v`)
3. Factory simulation tests passing (`pio test -e native_factory -v`)
4. ESP32 build successful (`pio run -e tlora_pager`)
5. No new linting warnings
6. Documentation updated if API changed

### Testing Checklist

- [ ] Unit test coverage for new features
- [ ] LVGL widget rendering verified
- [ ] Memory usage within limits (RAM < 80%, Flash < 80%)
- [ ] Hardware compatibility verified (if applicable)
- [ ] Edge cases handled (null pointers, empty buffers, etc.)

## Version History

### v1.2.x (Current Development)

- **2026-03-27**: Enhanced documentation, pipeline context updates
- **2025-12-19**: Factory firmware releases (SX1262 and LR1121 variants)
- **2025-11**: LVGL 9.2 migration complete
- **2025-10**: SkinnyCon 2026 badge apps finalized

### v1.1.x (Stable Release)

- **2025-09**: Initial SkinnyCon badge app release
- **2025-08**: GPS NMEA parsing improvements
- **2025-07**: NFC tag reading functionality added

### v1.0.x (Foundation)

- **2025-04**: Base firmware releases
- **2025-03**: LilyGoLib library v0.1.0 release
- **2025-02**: Initial ESP32-S3 port

## Changelog

See [CHANGELOG.md](./CHANGELOG.md) for detailed version history and API changes.

## Support and Community

### Official Channels

- **GitHub Issues**: [Report bugs and feature requests](https://github.com/garagehq/T-Lora-Pager-SkinnyCon/issues)
- **GitHub Discussions**: [Community discussions and questions](https://github.com/garagehq/T-Lora-Pager-SkinnyCon/discussions)
- **LilyGo Forum**: [Official LilyGo support](https://forum.lilygo.cc)

### Community Resources

- **Meshtastic**: [Alternative firmware](https://meshtastic.org)
- **LVGL Documentation**: [Official LVGL 9 docs](https://docs.lvgl.io)
- **ESP32 Arduino**: [Arduino Core for ESP32](https://github.com/espressif/arduino-esp32)

## Security

### Firmware Security

- **Secure Boot**: Enabled by default on ESP32-S3
- **Flash Encryption**: Optional, configurable in partition scheme
- **USB CDC**: Can be disabled for secure deployments

### Reporting Security Issues

Please report security vulnerabilities responsibly to the maintainers. Do not create public issues for security-sensitive matters.

## Acknowledgments

### Hardware

- **LilyGo**: T-LoRa-Pager hardware design and manufacturing
- **Espressif Systems**: ESP32-S3 chip and ESP-IDF framework
- **STMicroelectronics**: ST7796U display controller, AW9364 backlight, ST25R3911B NFC

### Open Source Libraries

- **LVGL**: Light and Versatile Graphics Library
- **RadioLib**: LoRa library for Arduino
- **TinyGPSPlus**: GPS NMEA parsing library
- **Adafruit Libraries**: TCA8418, BusIO, GFX, SH110X

### Community Contributors

- All GitHub contributors who have submitted code, documentation, or bug reports
- SkinnyCon community for badge app requirements and testing

---

**T-Lora-Pager-SkinnyCon** — A conference badge powered by LilyGo hardware and ESP32-S3.

Last Updated: March 27, 2026
Version: 1.2.x
License: MIT

