# SkinnyCon Badge Visual Reference

## Color Palette

All colors defined in `application/SkinnyCon/ui_skinnycon_theme.h`.

| Name             | Hex       | Usage                                    |
|------------------|-----------|------------------------------------------|
| `SC_BG`          | `#EFF6F6` | Light background                         |
| `SC_BG_DARK`     | `#BDE4E6` | Teal-tinted panel backgrounds            |
| `SC_PANEL`       | `#FFFFFF` | White card/panel backgrounds             |
| `SC_PANEL_ALT`   | `#E4F0F0` | Alternating row backgrounds              |
| `SC_HEADER`      | `#BDE4E6` | Header bar backgrounds                   |
| `SC_BORDER`      | `#A0D0D2` | Subtle teal borders                      |
| `SC_ACCENT`      | `#F96123` | SkinnyCon orange (titles ONLY)           |
| `SC_TEAL`        | `#5BBEC0` | Logo teal, icon accents, motion arcs     |
| `SC_GREEN`       | `#2A9D8F` | Teal-green                               |
| `SC_GREEN_BRIGHT`| `#059669` | Status OK                                |
| `SC_CYAN`        | `#2563EB` | Info blue, focus highlight on plain objs |
| `SC_RED`         | `#DC2626` | Error/warning                            |
| `SC_TEXT`        | `#000000` | Primary text (black)                     |
| `SC_TEXT_WHITE`  | `#FFFFFF` | Text on accent/dark backgrounds          |
| `SC_TEXT_DIM`    | `#5A6672` | Dimmed/secondary text (breaks, lunch)    |

## Color Rules

- **Orange (`SC_ACCENT`)** is ONLY for titles and text headings. Never use it for borders,
  accent bars, icon outlines, or decorative elements.
- **Teal (`SC_TEAL`)** is the go-to accent for icon elements (arcs, rings, decorative shapes).
- **`SC_TEXT`** (black) for icon bodies, primary shapes, and readable text.
- **`SC_TEXT_DIM`** for secondary/dimmed items (schedule breaks, lunch, inactive states).
- **`SC_CYAN`** for encoder focus highlights on custom (non-themed) objects.

## Icon Drawing

### Helpers (defined in `ui_main.cpp`)

All icon elements MUST use these helpers. NEVER use raw `lv_obj_create()` for icon parts.

| Helper         | What it creates                        | When to use                          |
|----------------|----------------------------------------|--------------------------------------|
| `icon_shape()` | Transparent container, all styles stripped | Container/layout for icon composition |
| `icon_fill()`  | Solid filled shape with color + radius | Dots, bars, solid bodies             |
| `icon_ring()`  | Border-only shape (no fill)            | Circles, arcs, outlines              |

### Why helpers matter

`lv_obj_create()` applies the LVGL default theme, which adds grey backgrounds, borders,
and shadows. On the SkinnyCon light theme, these appear as grey artifacts (vertical lines,
dark rectangles). The helpers call `lv_obj_remove_style_all()` first, then apply only
the styles you explicitly set.

### Icon composition pattern

```c
static void draw_icon_example(lv_obj_t *parent)
{
    lv_obj_t *c = icon_shape(parent, 48, 48);  // Outer container
    lv_obj_center(c);

    lv_obj_t *body = icon_fill(c, 28, 28, SC_TEXT, 4);     // Solid body
    lv_obj_align(body, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *ring = icon_ring(c, 40, 40, SC_TEAL, 2, 20); // Decorative ring
    lv_obj_align(ring, LV_ALIGN_CENTER, 0, 0);
}
```

### Partial borders (arcs)

For signal wave / motion arc effects, create an `icon_ring()` then restrict which sides show:

```c
lv_obj_t *arc = icon_ring(c, 30, 15, SC_TEAL, 2, 30);
lv_obj_set_style_border_side(arc, (lv_border_side_t)(LV_BORDER_SIDE_TOP), 0);
```

## Fonts

| Font               | Glyphs           | Usage                          |
|--------------------|------------------|--------------------------------|
| `font_alibaba_100` | Digits only      | Clock display (NOT for text)   |
| `font_alibaba_12`  | Full character set| Body text on tight screens     |
| LVGL default       | Full character set| General UI text                |

The display is 480x222 pixels (very short). Use small fonts to fit content.

## Focus & Highlight Styling

### Themed objects (lv_menu_cont, lv_btn, etc.)

These get focus highlighting automatically from the LVGL theme. No extra work needed.

### Custom objects (plain lv_obj after remove_style_all)

You MUST manually add focus styling:

```c
lv_obj_set_style_bg_color(obj, SC_CYAN, LV_STATE_FOCUS_KEY);
lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_STATE_FOCUS_KEY);
```

Use `LV_STATE_FOCUS_KEY` (not `LV_STATE_FOCUSED`) — the encoder uses key-focus, not
pointer-focus.

### Scroll-into-view

Custom objects inside scrollable containers need a focus callback:

```c
static void focus_scroll_cb(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target_obj(e);
    lv_obj_scroll_to_view(target, LV_ANIM_ON);
}
// ... in setup:
lv_obj_add_event_cb(obj, focus_scroll_cb, LV_EVENT_FOCUSED, NULL);
```

## Menu App Layouts

### Simple menu (most apps)

Use `create_menu()` + `lv_menu_cont_create()` for all items. The LVGL theme handles
everything — focus highlight, scroll, click navigation. No manual group management needed
beyond what `create_menu()` provides.

### Menu with display-only subpage items (Schedule pattern)

When a subpage has items that should be scrollable/focusable but NOT clickable:

1. **Navigable items** (main page): `lv_menu_cont_create()` + `lv_menu_set_load_page_event()`
2. **Display-only items** (subpages): `lv_obj_create()` + `lv_obj_remove_style_all()` +
   manual focus highlight + scroll-into-view callback
3. **Group management**: Deferred swap via `lv_timer_create()` on every page transition,
   always including the back button as the first group item

See `ui_schedule.cpp` for the complete working implementation.
