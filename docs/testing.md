# Testing Documentation for T-Lora-Pager-SkinnyCon

This document describes the testing infrastructure for the T-LoRa-Pager project, including unit tests, LVGL simulation, and hardware validation.

## Overview

The project uses a 4-tier testing architecture that allows comprehensive validation without requiring physical hardware for most tests:

| Tier | Environment | What It Tests | Tests | Speed |
|------|-------------|---------------|-------|-------|
| 1 | `native` | Brightness clamping, GPS NMEA parsing, RGB565 byte swap, power channel contiguity, hardware mask bitfields, rotary struct layout | 24 | <3s |
| 2 | `native_lvgl` | LVGL widgets, rendering, flex layouts, sliders, framebuffer, screenshots | 20 | ~7s |
| 3 | `native_factory` | Factory app screens with real fonts (Alibaba 12/24/40/100px), 9 icons, 6 SkinnyCon screens | 15 | ~7s |
| 4 | `tlora_pager` | Full ESP32-S3 compilation (build check only) | — | ~120s |

## Running Tests

### PlatformIO Test Commands

```bash
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

### Test Environments

The `platformio.ini` file defines four test environments:

1. **native**: Pure C tests for hardware logic without LVGL
2. **native_lvgl**: LVGL 9.2 headless simulator for widget testing
3. **native_factory**: Factory app simulation with real assets
4. **tlora_pager**: ESP32-S3 target for compilation verification

## Test Files

### Hardware Logic Tests (`test/test_hardware/test_hardware.c`)

24 comprehensive tests covering:

- **Brightness Controller**: Boundary tests and sweep tests for brightness clamping
- **GPS NMEA Parsing**: 6 test cases including Dallas, Sao Paulo, equator, Huntsville, antimeridian, and null inputs
- **RGB565 Byte Swap**: Channel extraction and byte order validation
- **Power Channel Contiguity**: Verification that power channels are contiguous in memory
- **Hardware Mask Bitfields**: Presence mask operations and bitfield manipulation
- **Rotary Encoder**: Struct layout validation

These tests use pure C code with no ESP32 dependencies, running on the native environment.

### LVGL Rendering Tests (`test/test_lvgl_render/test_lvgl_render.c`)

20 tests validating LVGL widget behavior:

- Display initialization
- Label rendering with real fonts
- Button widgets and click handlers
- Rendering to framebuffer
- Flex layouts and sliders
- PPM screenshot export
- Multi-widget layouts

### Factory Simulation Tests (`test/test_factory_sim/test_factory_sim.c`)

15 tests for the factory application:

- Main menu with real icons
- Clock screen
- Settings screen
- LoRa chat interface
- Logo display
- Monitor screen
- Nametag screen
- About screen
- Code of conduct
- Badgeshark integration
- Schedule display
- Network tools
- Font quality validation
- Icon grid rendering
- Screenshot validation

## LVGL Headless Simulator

The headless simulator renders to a 480×222 RGB565 framebuffer matching the physical display. Screenshots are saved as PPM files and uploaded as CI artifacts for visual regression testing.

### Key API Functions

```c
// Initialize the LVGL simulator
void lvgl_sim_init(void);

// Deinitialize the simulator
void lvgl_sim_deinit(void);

// Run LVGL timers for specified milliseconds
void lvgl_test_run(uint32_t ms);

// Save framebuffer as PPM image
int lvgl_test_save_ppm(const char *filename);

// Get direct framebuffer access for pixel verification
uint16_t *lvgl_sim_get_framebuffer(void);
```

### Simulator Configuration

The simulator uses `test/simulator/lv_conf.h` with `LV_STDLIB_CLIB` for pure C implementation. This differs from the main `src/lv_conf.h` which uses `LV_STDLIB_CUSTOM`.

### Framebuffer Verification

Tests can directly access the framebuffer for pixel-level validation:

```c
uint16_t *fb = lvgl_sim_get_framebuffer();
int non_zero = 0;
for (int i = 0; i < 480 * 222; i++) {
    if (fb[i] != 0) non_zero++;
}
TEST_ASSERT_TRUE(non_zero > 1000);  // Verify significant rendering
```

## Mock Libraries

The test infrastructure includes mock headers for ESP32 dependencies:

- `test/mocks/Arduino.h` - Arduino core mock
- `test/mocks/SPI.h` - SPI communication mock
- `test/mocks/Wire.h` - I2C communication mock
- `test/mocks/FreeRTOS.h` - RTOS mock
- `test/mocks/esp32/` - ESP-IDF GPIO/I2S/SPI mocks

These mocks allow hardware-independent testing of logic that would normally require ESP32 peripherals.

## Known Issues and Gotchas

### C vs C++ Compilation

Test files are `.c` (C language), but SkinnyCon app files are `.cpp` and include `ui_define.h` which pulls in `<iostream>`, `<vector>`, `<string>`. **Cannot** `#include` app `.cpp` files from C test files — must recreate UI directly in standalone C test functions.

### Icon Version Compatibility

- `img_sports` and `img_calendar` are v8-only (`#if LVGL_VERSION_MAJOR == 8`)
- They produce empty symbols in v9 builds
- Always use icons with `_v9.c` variants (e.g., `img_msgchat`, `img_monitoring`, `img_configuration`, `img_wifi`)

### Include Guards

`ui_define.h` needs `#ifndef UI_DEFINE_H` guard to prevent redefinition when multiple `.cpp` files are included in one translation unit.

### Static Variable Collisions

Apps in different `.cpp` files may use the same `static` variable names (e.g., `stats_label`). Must rename to unique names (e.g., `net_stats_label`) when both are linked.

### Hardware Dependencies

- `BrightnessController` is a CRTP template — tested via extracted logic, not full template
- ESP32 build requires vendor libs (XPowersLib, SensorLib, RadioLib) not mocked for native
- `LilyGoLib.h` needs `ARDUINO_T_LORA_PAGER` define to compile
- arduino-cli is x86_64 only — use PlatformIO for ARM64 sandbox testing
- Espressif package servers have intermittent 404s in CI — re-run failed jobs

## CI/CD Integration

Tests are automatically run in GitHub Actions for pull requests and main branch pushes. PPM screenshots from `native_lvgl` and `native_factory` environments are uploaded as CI artifacts for visual regression comparison.

### Artifact Upload

The CI workflow automatically uploads:
- Test results (JSON format)
- PPM screenshots from LVGL tests
- Coverage reports (when available)

### Failed Test Recovery

If ESP32 package downloads fail due to intermittent 404s, the CI will automatically retry the job.

## Python Analysis Tools

The project includes Python scripts for test analysis and data processing:

- `tools/test_analysis.py` - Parse test results and generate reports
- `tools/ppm_viewer.py` - View PPM screenshots from LVGL tests
- `tools/hardware_parser.py` - Parse hardware configuration files

### Running Python Tools

```bash
# Install dependencies
pip install -r requirements.txt

# Run test analysis
python tools/test_analysis.py --results test_results.json

# View PPM screenshots
python tools/ppm_viewer.py factory_main_menu.ppm
```

## Contributing New Tests

When adding new tests:

1. **Hardware Logic**: Add to `test/test_hardware/` with pure C code
2. **LVGL Widgets**: Add to `test/test_lvgl_render/` using simulator API
3. **Factory App**: Add to `test/test_factory_sim/` with real assets
4. **Mock Dependencies**: Update `test/mocks/` if new ESP32 dependencies are needed

### Test Structure

All tests follow the Unity test framework pattern:

```c
void test_feature_name(void) {
    // Setup
    lvgl_sim_init();
    
    // Test code
    lv_obj_t *obj = lv_label_create(...);
    lvgl_test_run(100);
    
    // Verification
    uint16_t *fb = lvgl_sim_get_framebuffer();
    TEST_ASSERT_NOT_EQUAL(0, fb[0]);
    
    // Cleanup
    lvgl_sim_deinit();
}
```

## References

- [MARISOL.md](../MARISOL.md) - Pipeline context and build instructions
- [PlatformIO Documentation](https://docs.platformio.org/)
- [LVGL Documentation](https://docs.lvgl.io/)
- [Unity Test Framework](https://www.throwtheswitch.org/unity)
