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
# 5. Open examples/factory/factory.ino and upload
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
| 1 | `native` | 24 | Brightness clamping, GPS NMEA parsing (6 cases), RGB565 byte swap + channels, power channel contiguity, hardware mask bitfields, rotary struct layout | <3s |
| 2 | `native_lvgl` | 20 | LVGL widgets, rendering, flex layouts, sliders, framebuffer, screenshots | ~7s |
| 3 | `native_factory` | 15 | Factory app screens with real fonts (Alibaba 12/24/40/100px), 9 icons, 6 SkinnyCon screens | ~7s |
| 4 | `tlora_pager` | — | Full ESP32-S3 compilation (build check) | ~120s |

### Test Files

- `test/test_hardware/test_hardware.c` — 24 tests: brightness clamping (boundary + sweep), GPS NMEA parsing (Dallas, Sao Paulo, equator, Huntsville, antimeridian, null inputs), RGB565 byte swap + channel extraction, PowerCtrlChannel contiguity, hardware presence mask bitfield ops, rotary struct layout
- `test/test_lvgl_render/test_lvgl_render.c` — 20 tests: display init, labels, buttons, click handlers, rendering to framebuffer, flex layouts, sliders, PPM export, multi-widget layouts
- `test/test_factory_sim/test_factory_sim.c` — 15 tests: main menu with real icons, clock screen, settings, LoRa chat, logo, monitor, nametag, about, code of conduct, badgeshark, schedule, net tools, font quality, icon grid, screenshot validation
- `test/simulator/sim_main.c/h` — Headless LVGL 9.2 simulator (480×222 RGB565 framebuffer)
- `test/simulator/lv_conf.h` — LVGL config for simulator (LV_STDLIB_CLIB)
- `test/mocks/` — Mock headers: Arduino.h, SPI.h, Wire.h, FreeRTOS, ESP-IDF GPIO/I2S/SPI

### Test Gotchas

- **C vs C++ compilation**: Test files are `.c` (C). SkinnyCon app files are `.cpp` and include `ui_define.h` which pulls in `<iostream>`, `<vector>`, `<string>`. Cannot `#include` app `.cpp` files from C test files — must recreate UI directly in standalone C test functions
- **Icon v8 vs v9**: `img_sports` and `img_calendar` are v8-only (`#if LVGL_VERSION_MAJOR == 8`), producing empty symbols in v9 builds. Always use icons with `_v9.c` variants (e.g., `img_msgchat`, `img_monitoring`, `img_configuration`, `img_wifi`)
- **Include guards**: `ui_define.h` needs `#ifndef UI_DEFINE_H` guard to prevent redefinition when multiple `.cpp` files are included in one translation unit
- **Static variable collisions**: Apps in different `.cpp` files may use the same `static` variable names (e.g., `stats_label`). Must rename to unique names (e.g., `net_stats_label`) when both are linked

### LVGL Simulator

The headless simulator renders to a 480×222 RGB565 framebuffer matching the physical display. Screenshots are saved as PPM files and uploaded as CI artifacts for visual regression testing.

Key API:
- `lvgl_sim_init()` / `lvgl_sim_deinit()` — setup/teardown per test
- `lvgl_test_run(ms)` — advance LVGL timers
- `lvgl_test_save_ppm(filename)` — export framebuffer as PPM image
- `lvgl_sim_get_framebuffer()` — direct framebuffer access for pixel verification

## CI/CD

### Workflows (5 total)

| Workflow | Jobs | Trigger |
|----------|------|---------|
| `tests.yml` | Native Tests + LVGL Simulator + Factory Sim + ESP32 Build (PlatformIO) | PR, push to master/supercon-port |
| `base_examples_ci.yml` | Arduino compile: factory, BLE, peripheral, power, sensor, sleep (31 sketches × tlora_pager) | PR, push |
| `lvgl_examples_ci.yml` | Arduino compile: 42 LVGL widget examples × tlora_pager | PR, push |
| `radio_examples_ci.yml` | Arduino compile: 13 radio examples × tlora_pager | PR, push |
| `release.yml` | ESP32-S3 firmware build + artifact upload (firmware.bin, bootloader.bin, partitions.bin, firmware.elf) | Push to master, manual |

**Board matrix**: `tlora_pager` only (twatch_ultra and twatchs3 removed — not targets for this fork)

**Build tool**: arduino-cli on x86_64 CI runners (NOT PlatformIO — arduino-cli is x86-only)

**Note**: Large Arduino repos with many examples take 15-30+ min in CI. Each sketch compiles independently. Transient failures (Espressif package server 404s) can fail unrelated sketches — re-run failed jobs.

### CI Gotchas

- **`--library .` flag**: Arduino CI includes the entire repo as a library for every sketch. Any file in `src/` is compiled for ALL sketches, not just factory. Never put example-specific code in `src/`
- **ci.json**: Per-example config files (`examples/*/ci.json`) control which boards build which sketches. Factory has no ci.json — defaults to `should_build=true`, `active_radio=Radio_SX1262`
- **Transient 404 failures**: Espressif's package server occasionally returns 404 for `esp32c6-libs` etc. These are NOT code bugs — re-run failed jobs
- **tests.yml branches**: Must include branch names in `push.branches` for CI to run on pushes (currently: `master`, `supercon-port`)

## Factory App Architecture

The factory example (`examples/factory/`) is the reference application with a tileview-based navigation:

- **Main screen** (tile 0,0): App grid with icons (horizontal scroll)
- **App screens** (tile 0,1): Dynamic per-app content
- **24+ apps**: Messaging, Radio, nRF24, GPS, Monitor, Power, Audio, Microphone, Sensor, Keyboard, System, Calendar, NFC, BLE, IR Remote, Camera Remote, Tools, Factory Test, Settings, **Nametag**, **BadgeShark**, **Schedule**, **Net Tools**
- **Input**: Rotary encoder, TCA8418 keyboard (full QWERTY, used for nametag editing), touch (optional)
- **Thread safety**: FreeRTOS mutex for concurrent hardware+GUI access

### Adding a New App

1. Create `examples/factory/ui_newapp.cpp` with `static void app_setup(lv_obj_t *parent)`, `static void app_exit(lv_obj_t *parent)`, and `app_t ui_newapp_main = {app_setup, app_exit, NULL};`
2. In `ui_main.cpp`: add `extern app_t ui_newapp_main;` and `create_app(panel, "App Name", &img_icon, &ui_newapp_main);`
3. Use `#ifndef ARDUINO` blocks for demo/simulation data, `#ifdef ARDUINO` for real hardware calls
4. For keyboard input: `hw_set_keyboard_read_callback(callback)` in setup, `hw_set_keyboard_read_callback(NULL)` in exit
5. Add a standalone C test function in `test/test_factory_sim/test_factory_sim.c` that recreates the UI directly (do NOT include the `.cpp` file)

## SkinnyCon 2026 Integration

Conference-specific apps for SkinnyCon 2026 (May 12-14, Huntsville AL, I2C Invention to Innovation Center, UAH campus):

- **Nametag** (`ui_nametag.cpp`): 5 display modes — name+subtitle (keyboard editable via TCA8418), fullscreen, About SkinnyCon, Code of Conduct, badge hardware info. `hw_set_keyboard_read_callback()` for real-time text input. Tab switches name/subtitle editing
- **Schedule** (`ui_schedule.cpp`): 3-day conference schedule with Left/Right day switching. Real talk data (TSCM topics, training sessions, panels). Scrollable with visual highlight
- **BadgeShark** (`ui_badgeshark.cpp`): LoRa packet sniffer, Wireshark-style hex dump, RSSI color-coding, auto-scroll, max 50 packets
- **Net Tools** (`ui_nettools.cpp`): LoRa mesh diagnostics — two-panel layout (ping log + peer discovery), stats bar with loss%/avg RTT
- **pio_main.cpp**: Guarded with `#ifdef PLATFORMIO` to prevent Arduino IDE duplicate symbol errors (see Dual Build System section)

## Dual Build System

This project has **two independent build paths** — understanding this is critical:

| Build Tool | Used For | Config |
|------------|----------|--------|
| **PlatformIO** | Local dev, unit tests (`native`, `native_lvgl`, `native_factory`), ESP32-S3 compilation (`tlora_pager`) | `platformio.ini` |
| **Arduino CLI** | GitHub Actions CI for all example sketches (31 base + 42 LVGL + 13 radio = 86 sketches) | `arduino-cli compile --library .` |

**Key difference**: Arduino CLI uses `--library .` which compiles everything in `src/` for ALL sketches. PlatformIO uses `build_src_filter` to selectively include `examples/factory/`. Never put example-specific code in `src/`.

**`pio_main.cpp`**: PlatformIO doesn't auto-process `.ino` files outside `src_dir`, so `examples/factory/pio_main.cpp` wraps `factory.ino` with `#include "factory.ino"`. It's guarded with `#ifdef PLATFORMIO` so Arduino IDE (which compiles all `.cpp` in the sketch directory) sees an empty translation unit — no duplicate `setup()`/`loop()` definitions.

## Known Issues

- `src/lv_conf.h` uses `LV_STDLIB_CUSTOM`; test simulator uses `LV_STDLIB_CLIB` (build flag override)
- `BrightnessController` is a CRTP template — tested via extracted logic, not full template
- ESP32 build requires vendor libs (XPowersLib, SensorLib, RadioLib) not mocked for native
- `LilyGoLib.h` needs `ARDUINO_T_LORA_PAGER` define to compile
- arduino-cli is x86_64 only — use PlatformIO for ARM64 sandbox testing
- Espressif package servers have intermittent 404s in CI — re-run failed jobs

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

## License

MIT License - See [LICENSE](./LICENSE) for details.

## Architecture Overview

### Hardware Abstraction Layer

The firmware uses a hardware abstraction layer (HAL) to support multiple radio chips and peripherals:

| Component | Implementation |
|-----------|----------------|
| **Radio** | RadioLib wrapper with SX1262, SX1280, CC1101, LR1121, Si4432, nRF24L01 support |
| **GPS** | TinyGPSPlus with NMEA parsing, AssistNow offline almanac support |
| **Display** | LVGL 9.2 with ST7796U driver, RGB565 framebuffer |
| **Keyboard** | TCA8418 I2C matrix driver with interrupt handling |
| **Power** | BQ25896 charger, BQ27220 fuel gauge, PCF85063 RTC |
| **Audio** | ES8311 I2S codec with PDM microphone input |

### File Structure

```
T-Lora-Pager-SkinnyCon/
├── src/                    # Core library code
│   ├── main.cpp           # Main entry point
│   ├── LilyGoLib.h        # Library header
│   ├── lv_conf.h          # LVGL configuration
│   └── ...                # Hardware drivers
├── examples/              # Example sketches
│   ├── factory/           # Factory app (main example)
│   │   ├── factory.ino
│   │   ├── pio_main.cpp   # PlatformIO wrapper
│   │   └── ui_*.cpp       # UI modules
│   ├── base/              # 31 base examples
│   ├── lvgl/              # 42 LVGL examples
│   └── radio/             # 13 radio examples
├── test/                  # Unit tests
│   ├── test_hardware/     # Hardware logic tests
│   ├── test_lvgl_render/  # LVGL rendering tests
│   ├── test_factory_sim/  # Factory app simulation
│   ├── simulator/         # Headless LVGL simulator
│   └── mocks/             # Mock headers
├── variants/              # Board variants
│   └── tlora_pager/       # T-LoRa-Pager config
├── libraries/             # External libraries
└── docs/                  # Documentation
```

### Build Configuration

| Parameter | Value |
|-----------|-------|
| **Framework** | Arduino ESP32 >= 3.3.0 |
| **Board** | tlora_pager (ESP32-S3-WROOM-1) |
| **Memory** | 16MB PSRAM, 3MB flash, 9MB FAT |
| **Upload Speed** | 921600 baud |
| **USB Mode** | Hardware CDC |
| **Clock Speed** | 240 MHz |
| **Flash Mode** | QIO |
| **Flash Size** | 16MB |
| **Partition** | app3M_fat9M_16MB |

### Power Management

| Feature | Details |
|---------|---------|
| **Charger** | BQ25896, 128-2048 mA in 64 mA steps |
| **Fuel Gauge** | BQ27220, battery %, voltage, current, time-to-empty |
| **Sleep modes** | Clock mode (80 MHz), display off, light sleep |
| **Display timeout** | Configurable via Settings app |
| **Low battery** | Auto-shutdown at 3300 mV with warning screen |

### Radio Configuration

The default radio is SX1262 (LoRa). Configurable parameters:

| Parameter | Range | Default |
|-----------|-------|---------|
| **Frequency** | 410-525 MHz, 862-1020 MHz | 868.0 MHz |
| **Bandwidth** | 7.8-500 kHz | 125 kHz |
| **Spreading Factor** | 5-12 | 9 |
| **TX Power** | -9 to 22 dBm | 10 dBm |
| **Coding Rate** | 4/5 to 4/8 | 4/5 |

### Display Specifications

| Parameter | Value |
|-----------|-------|
| **Resolution** | 480×222 pixels |
| **Color Depth** | RGB565 (16-bit) |
| **Interface** | SPI |
| **Driver** | ST7796U |
| **Backlight** | AW9364, 17 levels (0-16) |
| **GUI Framework** | LVGL 9.2 |

## Pipeline History

| Date | Phase | Result |
|------|-------|--------|
| 2026-03-09 | Initial setup | 52 tests (32 hw + 20 LVGL), simulator, CI, mocks |
| 2026-03-09 | CI fix | Removed twatch_ultra/twatchs3 from board matrix, fixed LVGL linking |
| 2026-03-10 | Factory sim | 61 tests (32 hw + 20 LVGL + 9 factory), real fonts/icons, visual regression |
| 2026-03-10 | SkinnyCon apps | 67 tests (32 hw + 20 LVGL + 15 factory). 4 new apps + 2 info screens |
| 2026-03-10 | Build fix | pio_main.cpp: PLATFORMIO guard fixes Arduino IDE duplicate symbols. Moving to src/ breaks non-factory sketches |
| 2026-03-10 | Test audit | 59 tests (24 hw + 20 LVGL + 15 factory). Removed 8 tautological tests (constant==constant), replaced with real logic tests (GPS edge cases, mask bitfield ops, flex layouts, sliders) |
| 2026-03-10 | Release CI | Added `release.yml` — builds firmware on every merge to master, uploads versioned artifacts (90-day retention) |

## Every Merge to Master

Every merge to `master` triggers a firmware build. Download the latest artifacts from the [Actions tab](../../actions/workflows/release.yml):

- `tlora-pager-firmware-<sha>.bin` — Main firmware (flash with `esptool.py`)
- `bootloader.bin` — Bootloader binary
- `partitions.bin` — Partition table
- `firmware.elf` — Debuggable ELF file

## Contributing

Contributions are welcome! Please follow these guidelines:

1. **Test first**: All new features should have unit tests in the appropriate tier
2. **Keep it modular**: Separate hardware drivers from UI code
3. **Document**: Update README.md and inline comments
4. **CI passes**: Ensure all workflows pass before submitting PRs
5. **SkinnyCon focus**: This fork focuses exclusively on the T-LoRa-Pager

## CI Artifacts

**CI artifacts**: PPM screenshots from factory simulation are uploaded for visual review on every PR.

## License

MIT License - See [LICENSE](./LICENSE) for details.