/**
 * @file      ui_skinnycon_theme.h
 * @brief     Shared SkinnyCon color palette for all badge apps
 * @details   Light theme based on SkinnyCon brand colors.
 *            All SkinnyCon apps (Nametag, BadgeShark, Schedule, Net Tools)
 *            should use these colors for visual consistency.
 *
 *            Brand colors:
 *              Background: #BDE4E6 / #EFF6F6
 *              Text:       #000000
 *              Accent:     #F96123 (SkinnyCon orange)
 *              Teal:       #5BBEC0 (logo circle)
 */
#ifndef UI_SKINNYCON_THEME_H
#define UI_SKINNYCON_THEME_H

#include <lvgl.h>

/* ── Core palette ─────────────────────────────────────────────── */
#define SC_BG             lv_color_hex(0xEFF6F6)  /* Light background */
#define SC_BG_DARK        lv_color_hex(0xBDE4E6)  /* Teal-tinted panels */
#define SC_PANEL          lv_color_hex(0xFFFFFF)  /* White card/panel bg */
#define SC_PANEL_ALT      lv_color_hex(0xE4F0F0)  /* Alternating row bg */
#define SC_HEADER         lv_color_hex(0xBDE4E6)  /* Header bar bg */
#define SC_BORDER         lv_color_hex(0xA0D0D2)  /* Subtle teal borders */

/* ── Accent colors ────────────────────────────────────────────── */
#define SC_ACCENT         lv_color_hex(0xF96123)  /* Primary: SkinnyCon orange */
#define SC_TEAL           lv_color_hex(0x5BBEC0)  /* Logo teal circle */
#define SC_GREEN          lv_color_hex(0x2A9D8F)  /* Teal-green */
#define SC_GREEN_BRIGHT   lv_color_hex(0x059669)  /* Status OK green */
#define SC_CYAN           lv_color_hex(0x2563EB)  /* Info blue */
#define SC_RED            lv_color_hex(0xDC2626)  /* Error/warning red */

/* ── Text colors ──────────────────────────────────────────────── */
#define SC_TEXT           lv_color_hex(0x000000)  /* Primary text (black) */
#define SC_TEXT_WHITE     lv_color_hex(0xFFFFFF)  /* Text on accent/dark bg */
#define SC_TEXT_DIM       lv_color_hex(0x5A6672)  /* Dimmed/secondary text */

#endif /* UI_SKINNYCON_THEME_H */
