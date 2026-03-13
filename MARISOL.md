# MARISOL.md — Pipeline Context for T-Lora-Pager-SkinnyCon

## Project Overview
T-Lora-Pager-SkinnyCon is a handheld LoRa pager device firmware project based on the LilyGo T-LoRa-Pager hardware. This SkinnyCon fork extends the original with custom applications including messaging, monitoring, and utility tools. The project uses PlatformIO for cross-platform development with native testing environments (native, native_lvgl, native_factory) for CI testing and ESP32-S3 target compilation.

## Build & Run
- **Language**: C (test files), C++ (application examples)
- **Framework**: PlatformIO, LVGL 9.2
- **Docker image**: Not explicitly defined in repository
- **Install deps**: `pip install platformio`
- **Run**: `pio run -e tlora_pager` (ESP32-S3 build check), `pio test -e native -v` (unit tests)

## Testing
- **Test framework**: PlatformIO native testing
- **Test command**: `pio test -e native -v` (unit tests), `pio test -e native_lvgl -v` (LVGL rendering), `pio test -e native_factory -v` (factory app simulation)
- **Hardware mocks needed**: Yes (Arduino.h, SPI.h, Wire.h, FreeRTOS, ESP-IDF GPIO/I2S/SPI mocks in test/mocks/)
- **Known test issues**: 
  - C vs C++ compilation: Test files are .c but app examples are .cpp with iostream/vector/string includes
  - Icon v8 vs v9: img_sports and img_calendar are v8-only, use _v9 variants in v9 builds
  - Include guards needed for ui_define.h to prevent redefinition
  - Static variable collisions between app .cpp files require unique naming

## Pipeline History
- 2024-03-13: Initial repository setup with PlatformIO native environments
- 2024-03-13: Test structure established with 4-tier architecture (native, native_lvgl, native_factory, tlora_pager)
- 2024-03-13: LVGL 9.2 headless simulator configured for PPM screenshot generation
- 2024-03-13: Mock infrastructure created for Arduino/ESP-IDF dependencies

## Known Issues
- src/lv_conf.h uses LV_STDLIB_CUSTOM; test simulator uses LV_STDLIB_CLIB (build flag override)
- BrightnessController is a CRTP template — tested via extracted logic, not full template
- ESP32 build requires vendor libs (XPowersLib, SensorLib, RadioLib) not mocked for native
- LilyGoLib.h needs ARDUINO_T_LORA_PAGER define to compile
- arduino-cli is x86_64 only — use PlatformIO for ARM64 sandbox testing
- Espressif package servers have intermittent 404s in CI — re-run failed jobs

## Notes
- **Test Files**: test/test_hardware/test_hardware.c (24 tests), test/test_lvgl_render/test_lvgl_render.c (20 tests), test/test_factory_sim/test_factory_sim.c (15 tests)
- **Simulator**: test/simulator/sim_main.c/h provides headless LVGL 9.2 simulator with 480×222 RGB565 framebuffer
- **Mock Headers**: test/mocks/ contains Arduino.h, SPI.h, Wire.h, FreeRTOS, ESP-IDF GPIO/I2S/SPI mocks
- **Dual Build System**: pio_main.cpp guarded with #ifdef PLATFORMIO to prevent Arduino IDE duplicate symbol errors
- **Dependencies**: Adafruit TCA8418, Adafruit BusIO, ESP32 BLE Keyboard Fork, IRremoteESP8266, ESP8266Audio, NimBLE-Arduino
- **Libraries**: lib_extra_dirs = libraries, lib_ldf_mode = deep+

---

**CRITICAL: Test Infrastructure Rules**
- All test files must be .c (C language) for compatibility with test framework
- App .cpp files cannot be #included from C test files — recreate UI directly in standalone C test functions
- LVGL v9 requires _v9 icon variants (img_msgchat, img_monitoring, img_configuration, img_wifi)
- Static variables in different .cpp files must have unique names (e.g., net_stats_label vs stats_label)
- pio_main.cpp must be guarded with #ifdef PLATFORMIO to avoid Arduino IDE conflicts

**Mock Architecture**
- test/mocks/ provides stub implementations for:
  - Arduino.h (digitalWrite, pinMode, Serial, etc.)
  - SPI.h (SPI class, transfer, begin, end)
  - Wire.h (Wire class, begin, endTransmission, requestFrom)
  - FreeRTOS.h (task, semaphore, queue primitives)
  - ESP-IDF GPIO/I2S/SPI (esp32-specific hardware interfaces)

**LVGL Simulator API**
- lvgl_sim_init() / lvgl_sim_deinit() — setup/teardown per test
- lvgl_test_run(ms) — advance LVGL timers
- lvgl_test_save_ppm(filename) — export framebuffer as PPM image
- lvgl_sim_get_framebuffer() — direct framebuffer access for pixel verification

**PlatformIO Dual Build Systems**
- PlatformIO environments: native, native_lvgl, native_factory, tlora_pager
- Arduino IDE build: examples/factory/pio_main.cpp (guarded with #ifdef PLATFORMIO)
- CI workflows: .github/workflows/ci_native.yml, .github/workflows/ci_lvgl.yml, .github/workflows/ci_factory.yml
