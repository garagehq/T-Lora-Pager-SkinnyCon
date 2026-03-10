# MARISOL.md - Pipeline Context for T-Lora-Pager-SkinnyCon

## Project Overview

**LilyGoLib** is an Arduino library for LilyGo hardware devices. This SkinnyCon fork focuses on the **T-LoRa-Pager** — a handheld LoRa messaging device with an ultra-wide display, repurposed as a conference badge for SkinnyCon 2026.

- **MCU**: ESP32-S3 (Xtensa dual-core, 240 MHz, PSRAM)
- **Display**: 480×222 ultra-wide IPS LCD (ST7796U), RGB565, SPI interface
- **GUI Framework**: LVGL 9.2
- **Radio**: LoRa via SX1262 (also supports SX1280, CC1101, LR1121, Si4432, nRF24L01)
- **Peripherals**: GPS (u-blox), NFC (ST25R3911B), BHI260AP 6-axis IMU, DRV2605 haptic, PCF85063 RTC, BQ25896 PMU, BQ27220 fuel gauge, AW9364 backlight (0-16 levels), TCA8418 keyboard matrix, ES8311 audio codec, PDM mic, SD card, rotary encoder
- **Build System**: Arduino framework via PlatformIO (ESP32 core >= 3.3.0)
- **Factory App**: 24+ screens including messaging, radio, GPS, audio, NFC, settings, sensors, and SkinnyCon badge apps

## Build & Run

```bash
# Install PlatformIO CLI
pip install platformio

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

# Upload to device (requires connected T-LoRa-Pager)
pio run -e tlora_pager --target upload
```

## Dual Build System

This project has **two independent build paths** — understanding this is critical for CI:

### PlatformIO (local dev + tests.yml CI)
- Used for unit tests (`native`, `native_lvgl`, `native_factory`) and ESP32-S3 compilation (`tlora_pager`)
- Config: `platformio.ini` with 4 environments
- The `tlora_pager` env uses `build_src_filter = +<*> +<../examples/factory/>` to compile factory app files
- PlatformIO does NOT auto-process `.ino` files outside `src_dir` — needs `pio_main.cpp` wrapper that `#include "factory.ino"`
- PlatformIO defines the `PLATFORMIO` macro automatically

### Arduino CLI (base/lvgl/radio_examples_ci.yml)
- Used by GitHub Actions CI to compile all example sketches (31 base + 42 LVGL + 13 radio = 86 sketches)
- Uses `arduino-cli compile` with `--library .` flag — this includes `src/` as a library for ALL sketches
- Arduino IDE compiles ALL `.c`/`.cpp` files in a sketch directory (e.g., `examples/factory/`)
- Arduino IDE does NOT define the `PLATFORMIO` macro

### The pio_main.cpp Problem (and fix)

**Problem**: `examples/factory/pio_main.cpp` wraps `factory.ino` for PlatformIO. But Arduino IDE also compiles it alongside `factory.ino`, causing duplicate `setup()`/`loop()` definitions.

**Failed fix**: Moving `pio_main.cpp` to `src/` broke all non-factory Arduino sketches because `src/` is included as a library (via `--library .`) for every sketch, and `factory.ino` doesn't exist in their include path → `fatal error: factory.ino: No such file or directory`.

**Correct fix**: Keep `pio_main.cpp` in `examples/factory/` but wrap with `#ifdef PLATFORMIO`. PlatformIO defines this macro; Arduino IDE does not. Arduino IDE sees an empty translation unit — no duplicate symbols, no missing files.

```cpp
#ifdef PLATFORMIO
#include "Arduino.h"
#include "factory.ino"
#endif
```

**Key rule**: NEVER put files in `src/` that reference example-specific code. `src/` is the library root and gets compiled for every Arduino sketch that depends on this library.

## Testing

### 4-Tier Architecture

| Tier | Environment | What It Tests | Tests | Speed |
|------|-------------|--------------|-------|-------|
| 1 | `native` | Brightness clamping, GPS NMEA parsing (6 cases), RGB565 byte swap + channels, power channel contiguity, hardware mask bitfields, rotary struct layout | 24 | <3s |
| 2 | `native_lvgl` | LVGL widgets, rendering, flex layouts, sliders, framebuffer, screenshots | 20 | ~7s |
| 3 | `native_factory` | Factory app screens with real fonts (Alibaba 12/24/40/100px), 9 icons, 6 SkinnyCon screens | 15 | ~7s |
| 4 | `tlora_pager` | Full ESP32-S3 compilation (build check) | — | ~120s |

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

### Workflows (4 total)

| Workflow | Jobs | Trigger |
|----------|------|---------|
| `tests.yml` | Native Tests + LVGL Simulator + Factory Sim + ESP32 Build (PlatformIO) | PR, push to master/supercon-port |
| `base_examples_ci.yml` | Arduino compile: factory, BLE, peripheral, power, sensor, sleep (31 sketches × tlora_pager) | PR, push |
| `lvgl_examples_ci.yml` | Arduino compile: 42 LVGL widget examples × tlora_pager | PR, push |
| `radio_examples_ci.yml` | Arduino compile: 13 radio examples × tlora_pager | PR, push |

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

## Known Issues

- `src/lv_conf.h` uses `LV_STDLIB_CUSTOM`; test simulator uses `LV_STDLIB_CLIB` (build flag override)
- `BrightnessController` is a CRTP template — tested via extracted logic, not full template
- ESP32 build requires vendor libs (XPowersLib, SensorLib, RadioLib) not mocked for native
- `LilyGoLib.h` needs `ARDUINO_T_LORA_PAGER` define to compile
- arduino-cli is x86_64 only — use PlatformIO for ARM64 sandbox testing
- Espressif package servers have intermittent 404s in CI — re-run failed jobs

## Pipeline History

| Date | Phase | Result |
|------|-------|--------|
| 2026-03-09 | Initial setup | 52 tests (32 hw + 20 LVGL), simulator, CI, mocks |
| 2026-03-09 | CI fix | Removed twatch_ultra/twatchs3 from board matrix, fixed LVGL linking |
| 2026-03-10 | Factory sim | 61 tests (32 hw + 20 LVGL + 9 factory), real fonts/icons, visual regression |
| 2026-03-10 | SkinnyCon apps | 67 tests (32 hw + 20 LVGL + 15 factory). 4 new apps + 2 info screens |
| 2026-03-10 | Build fix | pio_main.cpp: PLATFORMIO guard fixes Arduino IDE duplicate symbols. Moving to src/ breaks non-factory sketches |
| 2026-03-10 | Test audit | 59 tests (24 hw + 20 LVGL + 15 factory). Removed 8 tautological tests (constant==constant), replaced with real logic tests (GPS edge cases, mask bitfield ops, flex layouts, sliders) |
