# MARISOL.md - Pipeline Context for T-Lora-Pager-SkinnyCon

## Project Overview

**LilyGoLib** is an Arduino library for LilyGo hardware devices, including the T-LoRa-Pager, T-Watch-S3, and T-Watch-Ultra. This repository is the SkinnyCon fork, focused on the **T-LoRa-Pager** variant.

- **MCU**: ESP32-S3 (Xtensa dual-core, 240 MHz, PSRAM)
- **Display**: 480x222 ultra-wide IPS LCD, RGB565 (16-bit color), SPI interface
- **GUI Framework**: LVGL 9.2 (not v8)
- **Radio**: LoRa (SX1262/SX1280/CC1101/LR1121/Si4432 variants)
- **Peripherals**: GPS (TinyGPSPlus), NFC, BHI260AP sensor, DRV2605 haptic, PCF85063 RTC, BQ25896 PMU, BQ27220 fuel gauge, AW9364 backlight driver, rotary encoder, TCA8418 keyboard matrix, SD card, codec audio, PDM microphone
- **Build System**: Arduino framework via PlatformIO (ESP32 core >= 3.3.0)

## Build & Run

```bash
# Install PlatformIO CLI
pip install platformio

# Run native unit tests (no hardware needed)
pio test -e native -v

# Run LVGL headless simulator tests
pio test -e native_lvgl -v

# Compile for ESP32-S3 (build check only, no upload)
pio run -e tlora_pager

# Upload to device (requires connected T-LoRa-Pager)
pio run -e tlora_pager --target upload
```

## Testing Strategy

### 3-Tier Testing Architecture

| Tier | Environment | What It Tests | Speed |
|------|-------------|--------------|-------|
| 1 | `native` | Hardware logic, data structures, constants, parsing | <1s |
| 2 | `native_lvgl` | LVGL widget rendering, display simulation, screenshots | ~5s |
| 3 | `tlora_pager` | Full ESP32-S3 compilation (build check, no upload) | ~60s |

### Test Files

- `test/test_hardware/test_hardware.c` — 30+ tests: brightness clamping, PowerCtrlChannel enum, RotaryMsg_t, GPS NMEA parsing, RGB565 byte swap, hardware presence masks, keyboard states
- `test/test_lvgl_render/test_lvgl_render.c` — 20+ tests: display resolution, widget creation (labels, buttons), click handlers, rendering verification, PPM screenshot export, rotation modes
- `test/simulator/sim_main.c` — Headless LVGL 9.2 simulator (480x222 RGB565 framebuffer, no SDL dependency)
- `test/mocks/` — Mock headers for Arduino.h, SPI.h, Wire.h, FreeRTOS, ESP-IDF drivers

### Screenshot Testing

The LVGL simulator exports PPM screenshots after each render test. These are uploaded as CI artifacts for visual regression checking. PPM format was chosen for zero external dependencies.

## Hardware Details

| Component | Chip | Interface | Notes |
|-----------|------|-----------|-------|
| Display | ST7796 (480x222) | SPI | AW9364 backlight, 0-16 brightness levels |
| PMU | BQ25896 | I2C | 128-2048mA charge current, 64mA steps |
| Fuel Gauge | BQ27220 | I2C | Battery capacity tracking |
| RTC | PCF85063 | I2C | Real-time clock with backup |
| Haptic | DRV2605 | I2C | Vibration feedback, 0-123 effects |
| Sensor | BHI260AP | SPI | 6-axis IMU |
| GPS | u-blox | UART | TinyGPSPlus parser |
| Radio | SX1262/SX1280/etc | SPI | LoRa, shared SPI bus |
| Keyboard | TCA8418 | I2C | Matrix keyboard, backlight |
| NFC | ST25R3911B | SPI | RFAL stack |
| Rotary | Mechanical | GPIO | Rotary encoder + center button |
| Audio | ES8311 codec | I2S | Mono mic + speaker |

## Known Issues

- The library uses `LV_STDLIB_CUSTOM` malloc for the device target but tests use `LV_STDLIB_CLIB` for native compatibility
- `BrightnessController` is a CRTP template class requiring FreeRTOS timers; tested via extracted logic only
- ESP32 compilation requires many vendor libraries (XPowersLib, SensorLib, RadioLib, etc.) that are not mocked for native tests
- The `LilyGoLib.h` header has compile guards that require `ARDUINO_T_LORA_PAGER` to be defined

## Pipeline History

| Date | Phase | Result |
|------|-------|--------|
| 2026-03-09 | Initial setup | Created platformio.ini, test infrastructure, LVGL simulator, mock headers, CI workflow |
