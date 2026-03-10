<div align="center" markdown="1">
  <img src=".github/LilyGo_logo.png" alt="LilyGo logo" width="100"/>
</div>

<h1 align = "center">🌟LilyGoLib🌟</h1>

### `1` Get Started

- 🔧 **[T-LoRa-Pager Arduino IDE Quick Start](./docs/lilygo-t-lora-pager.md)**
- 🔧 **[T-Watch-Ultra Arduino IDE Quick Start](./docs/lilygo-t-watch-ultra.md)**
- 🔧 **[T-Watch-S3 Arduino IDE Quick Start](./docs/lilygo-t-watch-s3.md)**
- 🔧 **[T-Watch-S3-Plus Arduino IDE Quick Start](./docs/lilygo-t-watch-s3-plus.md)**

### `2` Hardware Information

- 🔧 **[T-LoRa-Pager Hardware Information](./docs/hardware/lilygo-t-lora-pager.md)**
- 🔧 **[T-Watch-Ultra Hardware Information](./docs/hardware/lilygo-t-watch-ultra.md)**
- 🔧 **[T-Watch-S3 Hardware Information](./docs/hardware/lilygo-t-watch-s3.md)**
- 🔧 **[T-Watch-S3-Plus Hardware Information](./docs/hardware/lilygo-t-watch-s3-plus.md)**

### `3` GPS AssistNow Usage Guide

- 🔧 **[GPS AssistNow Usage Guide](./docs/assistNow/assistNow.md)**

### `4` Unable to flash again

- ⚠️ **[Flash Steps Guide](./firmware/README.md)**

### `5` Testing

![Tests & Build](https://github.com/garagehq/T-Lora-Pager-SkinnyCon/actions/workflows/tests.yml/badge.svg)

This project uses a 3-tier testing strategy with [PlatformIO](https://platformio.org/):

```bash
# Install PlatformIO
pip install platformio

# Run native unit tests (no hardware required)
pio test -e native -v

# Run LVGL headless simulator tests (480x222 display, PPM screenshots)
pio test -e native_lvgl -v

# ESP32-S3 compilation check (build only, no upload)
pio run -e tlora_pager
```

**Test tiers:**
| Tier | PIO Environment | What It Tests |
|------|----------------|---------------|
| 1 - Unit | `native` | Hardware logic, data structures, GPS parsing, RGB565, brightness |
| 2 - Simulator | `native_lvgl` | LVGL 9.2 widgets, rendering, display properties, screenshots |
| 3 - Build | `tlora_pager` | Full ESP32-S3 compilation against real SDK |

### `6` Development Setup

1. Install [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation.html)
2. Clone this repository
3. Run `pio test -e native -v` to verify your setup
4. For ESP32-S3 builds: `pio run -e tlora_pager`

**Display specs:** 480x222 ultra-wide IPS, RGB565 (16-bit), SPI interface, 0-16 brightness levels
