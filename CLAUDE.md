# Project Guidelines

## Git & PRs
- Do NOT add `Co-Authored-By` lines to commits or PRs.
- When creating PRs, include screenshots of EVERY application screen rendered by the test suite.
- After every commit that changes UI code, regenerate screenshots and update them on the PR before pushing.
- After updating a PR, check CI results after a few minutes (use background bash with sleep). If tests fail, note which ones failed and work on a fix. Repeat until all tests pass.
- Before pushing new commits, cancel any in-progress GitHub Actions runs for the branch (`gh run list --branch <branch> --status in_progress` then `gh run cancel <id>`). We only care about the latest run.

## Testing
- Write failing tests BEFORE fixing bugs. Do not rely on the user to flash hardware and manually test.
- Use the test-first approach: write test -> confirm it fails -> fix code -> confirm it passes.
- For UI app tests, include ACTUAL app source code (`#include "../../application/SkinnyCon/ui_*.cpp"`) with hardware stubs rather than recreating app UIs from scratch in the test.
- Test real behaviors: keyboard input, scroll position, group state, pixel artifacts — not just visual appearance.
- The interaction test suite (`test/test_app_interaction/`) compiles with `#define ARDUINO 1` and stubs for `hw_*` functions so the real app keyboard callback code runs.
- Screenshot tests should generate PPM files via `lvgl_test_save_ppm()` and convert to PNG for PR review.

## SkinnyCon Theme
- Color palette defined in `application/SkinnyCon/ui_skinnycon_theme.h`
- Background: `#EFF6F6`, Teal panels: `#BDE4E6`, Orange titles: `#F96123`, Logo teal: `#5BBEC0`, Text: `#000000`
- Orange (`SC_ACCENT`) is reserved for titles only — not accent bars or borders.
- All LVGL objects used for icons/shapes must call `lv_obj_remove_style_all()` after creation to strip default theme styling that causes grey artifacts.
- Use `icon_shape()`, `icon_fill()`, `icon_ring()` helpers from `ui_main.cpp` for drawing clean icon shapes.

## Hardware (T-LoRa-Pager)
- Display: 480x222 IPS LCD (very short height — text must use small fonts to fit)
- Input: Rotary encoder (LEFT/RIGHT rotate, ENTER click) + physical QWERTY keyboard
- No ESC key on hardware — apps must use encoder click or LVGL menu back button to exit
- `font_alibaba_100` only has digit glyphs, not letters — do not use for text, only for clock digits
- `font_alibaba_12` is the smallest custom font — use for info/body text on tight screens
- Apps using `create_menu()` get a built-in back button (scroll up and click)
- Apps with custom containers (like Nametag) need explicit encoder click handling for back navigation
- Any app that needs encoder key events must add a focusable object to the default group via `lv_group_add_obj()`

## App Architecture
- Apps register via `app_t` struct with `setup_func_cb`, `exit_func_cb`, `loop_func_cb`
- `menu_show()` returns to the main scrolling menu
- `set_default_group(app_g)` is called when entering an app, `set_default_group(menu_g)` when returning to menu
- Keyboard input on nametag uses `hw_set_keyboard_read_callback()` — this is `#ifdef ARDUINO` gated
- The idle/clock screen checks `nametag_user_name` and shows the user's name when it's been changed from "YOUR NAME"
