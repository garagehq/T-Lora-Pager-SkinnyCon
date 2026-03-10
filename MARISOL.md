# MARISOL.md - Pipeline Context for T-Lora-Pager-SkinnyCon

## Project Overview

**LilyGoLib** is an Arduino library for LilyGo hardware devices. This SkinnyCon fork focuses on the **T-LoRa-Pager** — a handheld LoRa messaging device with an ultra-wide display.

- **MCU**: ESP32-S3 (Xtensa dual-core, 240 MHz, PSRAM)
- **Display**: 480×222 ultra-wide IPS LCD (ST7796U), RGB565, SPI interface
- **GUI Framework**: LVGL 9.2
- **Radio**: LoRa via SX1262 (also supports SX1280, CC1101, LR1121, Si4432, nRF24L01)
- **Peripherals**: GPS (u-blox), NFC (ST25R3911B), BHI260AP 6-axis IMU, DRV2605 haptic, PCF85063 RTC, BQ25896 PMU, BQ27220 fuel gauge, AW9364 backlight (0-16 levels), TCA8418 keyboard matrix, ES8311 audio codec, PDM mic, SD card, rotary encoder
- **Build System**: Arduino framework via PlatformIO (ESP32 core >= 3.3.0)
- **Factory App**: 20+ screens including messaging, radio, GPS, audio, NFC, settings, sensors

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

## Testing

### 4-Tier Architecture

| Tier | Environment | What It Tests | Tests | Speed |
|------|-------------|--------------|-------|-------|
| 1 | `native` | Hardware logic, constants, GPS parsing, RGB565 | 32 | <3s |
| 2 | `native_lvgl` | LVGL widgets, rendering, framebuffer, screenshots | 20 | ~7s |
| 3 | `native_factory` | Factory app screens with real fonts (Alibaba 12/24/40/100px), 9 icons, 6 SkinnyCon screens | 15 | ~7s |
| 4 | `tlora_pager` | Full ESP32-S3 compilation (build check) | — | ~60s |

### Test Files

- `test/test_hardware/test_hardware.c` — 32 tests: display dimensions, brightness clamping, PowerCtrlChannel enum, RotaryMsg_t, GPS NMEA parsing, RGB565 byte swap, hardware presence masks, keyboard states
- `test/test_lvgl_render/test_lvgl_render.c` — 20 tests: display init, labels, buttons, click handlers, rendering to framebuffer, PPM export, multi-widget layouts
- `test/test_factory_sim/test_factory_sim.c` — 15 tests: main menu with real icons, clock screen, settings, LoRa chat, logo, monitor, nametag, about, code of conduct, badgeshark, schedule, net tools, font quality, icon grid, screenshot validation
- `test/simulator/sim_main.c/h` — Headless LVGL 9.2 simulator (480×222 RGB565 framebuffer)
- `test/simulator/lv_conf.h` — LVGL config for simulator (LV_STDLIB_CLIB)
- `test/mocks/` — Mock headers: Arduino.h, SPI.h, Wire.h, FreeRTOS, ESP-IDF GPIO/I2S/SPI

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
| `tests.yml` | Native Tests + LVGL Simulator (PlatformIO) | PR, push to master |
| `base_examples_ci.yml` | Arduino compile: factory, BLE, peripheral, power, sensor, sleep (31 sketches × tlora_pager) | PR, push |
| `lvgl_examples_ci.yml` | Arduino compile: 42 LVGL widget examples × tlora_pager | PR, push |
| `radio_examples_ci.yml` | Arduino compile: 13 radio examples × tlora_pager | PR, push |

**Board matrix**: `tlora_pager` only (twatch_ultra and twatchs3 removed — not targets for this fork)

**Build tool**: arduino-cli on x86_64 CI runners (NOT PlatformIO — arduino-cli is x86-only)

**Note**: Large Arduino repos with many examples take 15-30+ min in CI. Each sketch compiles independently.

## Hardware Specs

| Component | Chip | Interface | Notes |
|-----------|------|-----------|-------|
| Display | ST7796U (480×222) | SPI | AW9364 backlight, 17 brightness levels (0-16) |
| PMU | BQ25896 | I2C | 128-2048mA charge, 64mA steps |
| Fuel Gauge | BQ27220 | I2C | Battery %, voltage, current |
| RTC | PCF85063 | I2C | Alarm, timer, backup |
| Haptic | DRV2605 | I2C | 124 waveform effects |
| IMU | BHI260AP | SPI | 6-axis accel + gyro |
| GPS | u-blox MIA-M10Q | UART | NMEA via TinyGPSPlus |
| Radio | SX1262 | SPI | LoRa, RadioLib 7.x |
| Keyboard | TCA8418 | I2C | Matrix keyboard with backlight |
| NFC | ST25R3911B | SPI | RFAL stack, WiFi/URL/text tags |
| Rotary | Mechanical | GPIO | Encoder + center button |
| Audio | ES8311 | I2S | Mono codec, PDM mic input |

## Factory App Architecture

The factory example (`examples/factory/`) is the reference application with a tileview-based navigation:

- **Main screen** (tile 0,0): App grid with icons
- **App screens** (tile 0,1): Dynamic per-app content
- **24+ apps**: Messaging, Radio, nRF24, GPS, Monitor, Power, Audio, Microphone, Sensor, Keyboard, System, Calendar, NFC, BLE, IR Remote, Camera Remote, Tools, Factory Test, Settings, **Nametag**, **BadgeShark**, **Schedule**, **Net Tools**
- **Input**: Rotary encoder, TCA8418 keyboard (full QWERTY, used for nametag editing), touch (optional)
- **Thread safety**: FreeRTOS mutex for concurrent hardware+GUI access

## Known Issues

- `src/lv_conf.h` uses `LV_STDLIB_CUSTOM`; test simulator uses `LV_STDLIB_CLIB` (build flag override)
- `BrightnessController` is a CRTP template — tested via extracted logic, not full template
- ESP32 build requires vendor libs (XPowersLib, SensorLib, RadioLib) not mocked for native
- `LilyGoLib.h` needs `ARDUINO_T_LORA_PAGER` define to compile
- arduino-cli is x86_64 only — use PlatformIO for ARM64 sandbox testing

## Pipeline History

| Date | Phase | Result |
|------|-------|--------|
| 2026-03-09 | Initial setup | 52 tests (32 hw + 20 LVGL), simulator, CI, mocks |
| 2026-03-09 | CI fix | Removed twatch_ultra/twatchs3 from board matrix, fixed LVGL linking |
| 2026-03-10 | Factory sim | 61 tests (32 hw + 20 LVGL + 9 factory), real fonts/icons, visual regression |
| 2026-03-10 | SkinnyCon apps | 67 tests (32 hw + 20 LVGL + 15 factory). Nametag (editable, keyboard input, 5 modes), BadgeShark, Schedule (3-day real data), Net Tools, About, Code of Conduct. Fixed pio_main.cpp Arduino build conflict |

## SkinnyCon 2026 Integration

Conference-specific apps for SkinnyCon 2026 (May 12-14, Huntsville AL, I2C Invention to Innovation Center, UAH campus):

- **Nametag** (`ui_nametag.cpp`): 5 display modes — name+subtitle (keyboard editable via TCA8418), fullscreen, About SkinnyCon, Code of Conduct, badge hardware info. `hw_set_keyboard_read_callback()` for real-time text input. Tab switches name/subtitle editing
- **Schedule** (`ui_schedule.cpp`): 3-day conference schedule with Left/Right day switching. Real talk data (TSCM topics, training sessions, panels). Scrollable with visual highlight
- **BadgeShark** (`ui_badgeshark.cpp`): LoRa packet sniffer, Wireshark-style hex dump, RSSI color-coding, auto-scroll, max 50 packets
- **Net Tools** (`ui_nettools.cpp`): LoRa mesh diagnostics — two-panel layout (ping log + peer discovery), stats bar with loss%/avg RTT
- **pio_main.cpp**: Guarded with `#ifdef PLATFORMIO` to prevent Arduino IDE duplicate symbol errors
