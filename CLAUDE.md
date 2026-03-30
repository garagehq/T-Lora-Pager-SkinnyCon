# MARISOL.md — Pipeline Context for T-Lora-Pager-SkinnyCon

## Repository Metadata
- **Project Name**: T-Lora-Pager-SkinnyCon
- **Base Hardware**: LilyGo T-LoRa-Pager (ESP32-S3 based)
- **Framework**: Arduino Core v3.3.0+ / PlatformIO
- **GUI Framework**: LVGL 9.2
- **Primary MCU**: ESP32-S3 dual-core 240 MHz with PSRAM
- **Display**: ST7796U 480x222 RGB565 via SPI
- **Radio**: SX1262 (LoRa), supports SX1280, CC1101, LR1121, Si4432
- **GPS**: u-blox MIA-M10Q via UART (NMEA)
- **NFC**: ST25R3911B via SPI (RFAL stack)
- **Keyboard**: TCA8418 I2C QWERTY matrix
- **IMU**: BHI260AP 6-axis (accelerometer + gyroscope)
- **Build System**: Dual — PlatformIO for tests/ESP32, Arduino CLI for examples

## Build Configuration

### PlatformIO Environment Matrix
| Environment | Purpose | Platform | Test Framework | Build Flags |
|-------------|---------|----------|----------------|-------------|
| `native` | Unit tests (no hardware) | native | Unity | C++17, LVGL_SIMULATOR |
| `native_lvgl` | LVGL widget tests | native | Unity | LVGL 9.2 headless sim |
| `native_factory` | Factory app simulation | native | Unity | Real fonts/icons assets |
| `tlora_pager` | ESP32-S3 firmware | espressif32@6.10.0 | — | Arduino framework |

**Platform**: espressif32@6.10.0
**Board**: tlora_pager (custom JSON in boards/)
**Variant**: variants/tlora_pager/pins_arduino.h
**Partition**: application/SkinnyCon/partitions.csv (16MB: 3MB app / 9.9MB FATFS)
**Upload Speed**: 921600 bps
**Monitor Speed**: 115200 bps

### Arduino Board Settings
| Setting | Value |
|---------|-------|
| Board | LilyGo-T-LoRa-Pager |
| USB CDC On Boot | Enabled |
| CPU Frequency | 240MHz |
| Upload Speed | 921600 |
| Upload Mode | UART0/Hardware CDC |
| USB Mode | CDC and JTAG |
| Partition Scheme | 16M Flash (3M APP / 9.9MB FATFS) |

### Radio Configuration Options
- Radio-SX1262 (Sub 1G LoRa) — **Default**
- Radio-SX1280 (2.4G LoRa)
- Radio-CC1101 (Sub 1G GMSK/2FSK/4FSK/ASK/OOK)
- Radio-LR1121 (Sub 1G + 2.4G LoRa)
- Radio-SI4432 (Sub 1G ISM)

## Third-Party Dependencies

### Platform Dependencies (library.json)
- RadioLib 7.1.2
- lvgl 9.2.2
- XPowersLib 0.2.9
- SensorLib 0.3.1

### Additional Libraries (library.properties)
- Adafruit TCA8418 1.0.2
- Adafruit BusIO 1.17.0
- ESP8266Audio 2.0.0
- TinyGPSPlus 1.1.0
- NimBLE-Arduino
- IRremoteESP8266
- NFC-RFAL Fork 1.0.1
- ST25R3916 Fork 1.1.0
- ESP32 BLE Keyboard Fork 0.3.3
- ESP32 BLE Mouse Fork 0.3.1

## Test Environment Status

**Test Framework**: Unity (embedded, standalone compilation)
**Test Count**: 62 total (4-tier strategy)
- Tier 1 (native): 27 tests — hardware logic, data parsing, struct layouts
- Tier 2 (native_lvgl): 20 tests — LVGL widgets, rendering, framebuffer
- Tier 3 (native_factory): 15 tests — factory screens with real assets
- Tier 4 (tlora_pager): Build verification — full ESP32-S3 compilation

### Test Coverage Summary
| Test Suite | Files | Tests | Primary Coverage |
|------------|-------|-------|------------------|
| test_hardware | test_hardware.c | 27 | Brightness clamping, GPS NMEA parsing, RGB565 byte swap, power channels, hardware masks, rotary input, LoRa frequency validation |
| test_lvgl_render | test_lvgl_render.c | 20 | LVGL 9.2 display init, labels, buttons, click handlers, flex layouts, sliders, framebuffer rendering, PPM export |
| test_factory_sim | test_factory_sim.c | 15 | Factory app screens, real Alibaba fonts (12/24/40/100px), 9 app icons, 6 SkinnyCon screens |

### CI/CD Workflows
| Workflow | Jobs | Trigger | Description |
|----------|------|---------|-------------|
| tests.yml | Native Tests, LVGL Sim, Factory Sim, ESP32 Build | PR, push to master | All 4-tier tests + build verification |
| base_examples_ci.yml | 31 base sketches × tlora_pager | PR, push | Arduino CLI compilation of base examples |
| lvgl_examples_ci.yml | 42 LVGL examples × tlora_pager | PR, push | Arduino CLI compilation of LVGL examples |
| radio_examples_ci.yml | 13 radio examples × tlora_pager | PR, push | Arduino CLI compilation of radio examples |
| release.yml | ESP32-S3 firmware build + artifacts | Push to master, manual | Auto-generate firmware binaries |

**Total Example Compilation**: 86 sketches (31 base + 42 LVGL + 13 radio)

## Firmware Artifacts

### Release Artifacts (from release.yml)
- `tlora-pager-firmware-<sha>.bin` — Main firmware (flash at 0x10000)
- `tlora-pager-bootloader-<sha>.bin` — Bootloader (flash at 0x0)
- `tlora-pager-partitions-<sha>.bin` — Partition table (flash at 0x8000)

### Factory Firmware
- `factory.tlora.pager.lr1121.20251219.bin` — LR1121 radio variant
- `factory.tlora.pager.sx1262.20251219.bin` — SX1262 radio variant (default)

### Flash Commands
```bash
# Manual flash with esptool.py
esptool.py --chip esp32s3 --port /dev/ttyUSB0 --baud 921600 \
  write_flash 0x0 tlora-pager-bootloader-<sha>.bin \
              0x8000 tlora-pager-partitions-<sha>.bin \
              0x10000 tlora-pager-firmware-<sha>.bin

# Alternative compact command (when using combined binary)
esptool --chip esp32s3 --baud 921600 --before default_reset --after hard_reset \
  write_flash -z --flash_mode dio --flash_freq 80m 0x0 firmware.bin
```

## Display Specifications

| Property | Value |
|----------|-------|
| Resolution | 480 × 222 pixels (ultra-wide landscape) |
| Color Depth | RGB565 (16-bit, 65,536 colors) |
| Controller | ST7796U |
| Interface | SPI |
| Backlight Controller | AW9364 (17 levels, 0-16) |
| GUI Framework | LVGL 9.2 (LV_STDLIB_CUSTOM) |
| Backlight Levels | 0-16 (17 total) |

## Power Management

| Feature | Details |
|---------|---------|
| Charger IC | BQ25896 (128-2048 mA in 64 mA steps) |
| Fuel Gauge | BQ27220 I2C (battery %, voltage, current, time-to-empty) |
| PMU/RTC | PCF85063 I2C (alarm, timer, battery backup) |
| Sleep Modes | Clock mode (80 MHz), display off, light sleep |
| Display Timeout | Configurable via Settings app |
| Low Battery Shutdown | Auto-shutdown at 3300 mV with warning screen |
| OTG Support | Toggleable via Power app |

## Radio Specifications

### SX1262 (Default)
- **Frequency Range**: 410-525 MHz, 862-1020 MHz
- **Default**: 915.0 MHz (US ISM band)
- **Bandwidth**: 7.8-500 kHz (default: 125 kHz)
- **Spreading Factor**: 5-12 (default: 9)
- **TX Power**: -9 to 22 dBm (default: 10 dBm)
- **Coding Rate**: 4/5 to 4/8 (default: 4/5)

### Alternative Radios
- **SX1280**: 2.4 GHz LoRa (up to 1000 km range at 300 bps)
- **CC1101**: Sub-1 GHz (GMSK, 2FSK, 4FSK, ASK, OOK)
- **LR1121**: Dual-band LoRa (sub-1 GHz + 2.4 GHz)
- **Si4432**: Sub-1 GHz ISM (frequency hopping, FSK, GFSK)

## GPS Specifications

- **Chip**: u-blox MIA-M10Q
- **Interface**: UART
- **Protocol**: NMEA sentences
- **Parsing Library**: TinyGPSPlus 1.1.0
- **Satellite Systems**: GPS, GLONASS, Galileo, QZSS, BeiDou
- **Cold Start**: 30-60 seconds acquisition outdoors
- **Features**: Coordinates, speed, altitude, satellite count, fix quality

## NFC Specifications

- **Chip**: ST25R3911B
- **Interface**: SPI
- **Protocol**: ISO 14443 Type A
- **Stack**: RFAL (NFC-RFAL Fork 1.0.1)
- **Tag Types**: WiFi config, URLs, plain text, custom data
- **Range**: Up to 4 cm
- **Applications**: WiFi provisioning, web shortcuts, text notes

## IMU Specifications

- **Chip**: BHI260AP (6-axis)
- **Interface**: SPI
- **Axes**: Accelerometer (3-axis) + Gyroscope (3-axis)
- **Features**: Orientation, roll, pitch, heading
- **Libraries**: SensorLib 0.3.1
- **Compass**: QMC5883P (if hardware present)

## Audio Specifications

- **Codec**: ES8311 I2S mono audio
- **Microphone**: PDM (Pulse Density Modulation)
- **Haptic**: DRV2605 I2C (124 waveform effects)
- **Playback**: SD card MP3 via ESP8266Audio
- **Visualization**: FFT frequency bands on display

## Keyboard Specifications

- **Controller**: TCA8418 I2C
- **Layout**: Full QWERTY matrix
- **Features**: Backlight control (left orange button + button B)
- **Modes**: Number/symbol (Space + Key), Capitalization (CAP + Key)
- **Library**: Adafruit TCA8418 1.0.2
- **Bus**: Adafruit BusIO 1.17.0

## Rotary Encoder

- **Type**: Mechanical encoder with center button
- **Interface**: GPIO
- **Functions**: Scroll through menus, click to select
- **Integration**: Custom driver in src/rotary/

## Memory Requirements

| Metric | Value |
|--------|-------|
| **Total RAM** | 327,680 bytes (ESP32-S3 PSRAM) |
| **Typical Usage** | ~75,708 bytes (23.1%) |
| **Total Flash** | 4,194,304 bytes (4MB) |
| **Firmware Usage** | ~2,471,085 bytes (58.9%) |
| **Partition Scheme** | 3MB app / 9.9MB FATFS (16MB total) |

## File Paths and Structure

### Source Files
- **Main App**: application/SkinnyCon/SkinnyCon.ino (Arduino IDE), application/SkinnyCon/pio_main.cpp (PlatformIO)
- **Hardware Abstraction**: application/SkinnyCon/hal_interface.h/cpp (50+ functions)
- **UI Components**: application/SkinnyCon/ui_*.cpp (per-app screen implementations)
- **Assets**: application/SkinnyCon/src/ (fonts, images)
- **Core Library**: src/LilyGoLib.h (main library header)

### Test Files
- **Unit Tests**: test/test_hardware/test_hardware.c (27 tests)
- **LVGL Tests**: test/test_lvgl_render/test_lvgl_render.c (20 tests)
- **Factory Sim**: test/test_factory_sim/test_factory_sim.c (15 tests)
- **Simulator**: test/simulator/sim_main.c (headless LVGL 9.2)
- **Mock Headers**: test/mocks/ (Arduino, SPI, Wire, FreeRTOS, ESP-IDF)

### Configuration Files
- **PlatformIO**: platformio.ini (4 environments)
- **LVGL Config**: src/lv_conf.h (ESP32-S3 optimized)
- **Board Config**: boards/tlora_pager.json (custom board definition)
- **Variant**: variants/tlora_pager/pins_arduino.h (pin mapping)

### Documentation
- **README.MD**: Main project documentation (user-facing)
- **MARISOL.md**: Pipeline context (build/test/history info)
- **docs/lilygo-t-lora-pager.md**: Arduino IDE quick start
- **docs/third_party.md**: Dependency versions
- **docs/hardware/lilygo-t-lora-pager.md**: Hardware reference
- **docs/assistNow/**: GPS AssistNow usage guide

## CI Artifacts and Downloads

### GitHub Actions Artifacts
- **Location**: [Actions tab](../../actions/workflows/release.yml)
- **Retention**: 90 days
- **Upload**: Auto-deploy on merge to master
- **Trigger**: Manual or push to master

### Test Screenshots
- **Format**: PPM (portable pixel map)
- **Export**: lvgl_test_save_ppm("screenshot.ppm")
- **Upload**: CI artifacts for visual regression review
- **Location**: test/test_factory_sim/ outputs

## Troubleshooting Reference

| Issue | Solution | Verification Command |
|-------|----------|---------------------|
| Can'\''t flash | Hold BOOT + RST, verify 921600 baud | `pio run -e tlora_pager --target upload` |
| Display blank | Check SPI, verify backlight > 0 | `pio test -e native` |
| No LoRa signal | Verify antenna, match frequency | `pio test -e native` |
| GPS no fix | Move outdoors, wait 60s | Check `app_gps.cpp` output |
| Keyboard unresponsive | Check I2C address TCA8418 | `pio test -e native` |
| Build fails | Verify ESP32 Core >= 3.3.0 | `pio run -e tlora_pager` |
| Tests fail | Check PlatformIO setup | `pio test -e native -v` |
| Arduino CI: duplicates | Verify `#ifdef PLATFORMIO` guard | Check pio_main.cpp |
| Arduino CI: transient 404 | Re-run failed jobs | GitHub Actions UI |

## Known Issues and Limitations

- **PlatformIO ESP32 Core**: v3.3.0+ requires manual installation (platformio not fully supported for latest Arduino Core)
- **USB Port Flakiness**: May require download mode (BOOT + RST)
- **GPS Cold Start**: 30-60 seconds outdoors
- **Display Timeout**: Configurable in Settings (default: 10s to clock screen)
- **Auto-Sleep**: 10s inactivity -> 80 MHz clock, display remains on
- **USB CDC**: Must be enabled for serial output

## Development Guidelines

### Build System Dual-Path Warning
- **PlatformIO**: Uses `build_src_filter` to selectively include application/SkinnyCon/
- **Arduino CLI**: Compiles ALL src/ for every example via `--library .`
- **Rule**: Never put example-specific code in src/ — use examples/ subdirectories

### pio_main.cpp Guard Pattern
```cpp
#ifdef PLATFORMIO
#include "SkinnyCon.ino"
#endif
// Empty translation unit for Arduino IDE builds
```

### CI Best Practices
- Test locally with `pio test -e native -v` before PR
- Verify all 3 test environments pass (native, native_lvgl, native_factory)
- Run `pio run -e tlora_pager` for final build check
- Check Arduino CI logs for transient 404 errors (re-run jobs)

## Pipeline Status Summary

- **Unit Tests**: 27/27 passing (native environment)
- **LVGL Widget Tests**: 20/20 passing (native_lvgl environment)
- **Factory Sim Tests**: 15/15 passing (native_factory environment)
- **ESP32 Build**: Successful (tlora_pager environment)
- **CI Workflows**: All green (tests.yml, base_examples, lvgl_examples, radio_examples)
- **Release Artifacts**: Auto-uploaded on master merge

---

Last Updated: 2026-03-27
Pipeline Version: 1.0
Documentation Maintained By: MARISOL Pipeline System
