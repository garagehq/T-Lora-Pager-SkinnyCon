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
- Orange (`SC_ACCENT`) is reserved for titles only — not accent bars, borders, or icon shapes.
- All LVGL objects used for icons/shapes must call `lv_obj_remove_style_all()` after creation to strip default theme styling that causes grey artifacts.
- Use `icon_shape()`, `icon_fill()`, `icon_ring()` helpers from `ui_main.cpp` for drawing clean icon shapes. NEVER use raw `lv_obj_create()` for icon elements — the helpers strip default styling that leaks grey borders/backgrounds.
- See `AESTHETICS.md` for the full visual reference.

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

## LVGL Menu + Encoder Group Management (CRITICAL)

This section documents hard-won lessons from the Schedule app debugging. These rules
apply to ANY app that uses `lv_menu` with subpages and encoder navigation.

### The Back Button Must Be In The Group — Always
- `create_menu()` creates a back button, but it is NOT automatically added to the encoder group.
- You MUST manually add it: `lv_group_add_obj(g, lv_menu_get_main_header_back_button(menu))`
- Add it as the FIRST item in the group so scrolling up reaches it.
- Add it on EVERY group swap — initial setup, entering subpages, AND returning to the main page.
- If you forget this on any code path, the back button becomes unreachable by encoder.

### Group Swapping Pattern for Subpage Navigation
When an app has a main page (e.g., day list) and subpages (e.g., talk lists), the encoder
group must be rebuilt each time the user navigates between pages:
1. Use `lv_group_remove_all_objs(g)` to clear the group.
2. Add the back button FIRST.
3. Add the items for the current page.
4. Do this via a deferred timer (`lv_timer_create(..., 50, NULL)`) — swapping the group
   inside an lv_menu event handler causes race conditions with LVGL's internal state.

### lv_menu_cont Clicks Cause Freezes
- `lv_menu_cont_create()` objects are recognized by `lv_menu` internally. When clicked,
  `lv_menu` tries to load an associated subpage.
- If a `lv_menu_cont` has NO associated page (e.g., talk rows that are display-only),
  clicking it FREEZES the entire UI — lv_menu hangs trying to navigate nowhere.
- **Fix**: Use plain `lv_obj_create()` + `lv_obj_remove_style_all()` for display-only rows.
  Only use `lv_menu_cont_create()` for items that genuinely navigate to subpages.
- `lv_event_stop_bubbling()` / `lv_event_stop_processing()` do NOT fix this — lv_menu's
  internal handler fires before user-registered callbacks.
- `lv_obj_remove_flag(cont, LV_OBJ_FLAG_CLICKABLE)` also does NOT fix this — lv_menu
  still recognizes the object type.

### Focus Highlight After remove_style_all
- `lv_obj_remove_style_all()` strips ALL styling including the theme's focus highlight.
- If you use plain `lv_obj_create()` + `lv_obj_remove_style_all()` for focusable items,
  you MUST manually add focus styling:
  ```c
  lv_obj_set_style_bg_color(obj, HIGHLIGHT_COLOR, LV_STATE_FOCUS_KEY);
  lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_STATE_FOCUS_KEY);
  ```
- Without this, the encoder scrolls through items but the user can't see which is focused.

### Scroll-Into-View for Encoder Navigation
- `lv_menu` does NOT automatically scroll to follow encoder focus on non-menu-cont objects.
- Add a `LV_EVENT_FOCUSED` handler that calls `lv_obj_scroll_to_view(target, LV_ANIM_ON)`.
- Without this, the encoder moves focus past the visible area but the screen doesn't scroll.

### Reference Implementation
`ui_schedule.cpp` is the canonical example of all these patterns working together:
- Main page: `lv_menu_cont_create()` for navigable day entries + `lv_menu_set_load_page_event()`
- Subpages: plain `lv_obj_create()` for display-only talk rows
- Deferred group swap via `lv_timer_create()` on every navigation
- Back button added to group on every page transition
- Focus highlight and scroll-into-view on all plain objects

## Nametag App
- Uses `create_menu()` with subpages for Edit Name, About, Code of Conduct, Badge Info.
- Textarea in edit mode captures keyboard input via `LV_EVENT_KEY`.
- `LV_EVENT_VALUE_CHANGED` forces uppercase on all input.
- `enable_keyboard()` on focus, `disable_keyboard()` on blur/save.
- Saves name/subtitle to flash via Preferences API.
- Back button handler: clean up menu, call `menu_show()`.

## Schedule App
- Three-day conference schedule with day list → talk list navigation.
- Talk data is static `talk_t` arrays (time string + title + is_break flag).
- Day entries use `lv_menu_cont_create()` with `lv_menu_set_load_page_event()`.
- Talk rows use plain `lv_obj_create()` to avoid click-freeze (see rules above).
- `MAX_TALKS` is 15 per day — increase if adding more sessions.
- To update the schedule: edit the `day1_talks[]`, `day2_talks[]`, `day3_talks[]` arrays
  and the `day_names[]` array. No other changes needed.
- Break/lunch items use `is_break: true` which renders them in `SC_TEXT_DIM` (dimmed).
