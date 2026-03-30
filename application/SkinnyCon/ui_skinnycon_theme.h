/**
 * @file      ui_skinnycon_theme.h
 * @brief     Shared SkinnyCon color palette for all badge apps
 * @details   Based on the Hackaday Supercon aesthetic. All SkinnyCon apps
 *            (Nametag, BadgeShark, Schedule, Net Tools) should use these
 *            colors for visual consistency.
 */
#ifndef UI_SKINNYCON_THEME_H
#define UI_SKINNYCON_THEME_H

#include <lvgl.h>

/* ── Core palette ─────────────────────────────────────────────── */
#define SC_BG             lv_color_hex(0x1A1A1A)  /* Background: Hackaday grey */
#define SC_BG_DARK        lv_color_hex(0x0D1117)  /* Darker bg for terminals/logs */
#define SC_PANEL          lv_color_hex(0x2A2A2A)  /* Card/panel background */
#define SC_PANEL_ALT      lv_color_hex(0x161B22)  /* Alternating row bg */
#define SC_HEADER         lv_color_hex(0x21262D)  /* Header bar bg */
#define SC_BORDER         lv_color_hex(0x30363D)  /* Subtle borders */

/* ── Accent colors ────────────────────────────────────────────── */
#define SC_ACCENT         lv_color_hex(0xE39810)  /* Primary: Hackaday yellow */
#define SC_GREEN          lv_color_hex(0xABC5A0)  /* Sage green (subtle) */
#define SC_GREEN_BRIGHT   lv_color_hex(0x39D353)  /* Bright green (status OK) */
#define SC_CYAN           lv_color_hex(0x58A6FF)  /* Info blue */
#define SC_RED            lv_color_hex(0xFF6B6B)  /* Error/warning red */

/* ── Text colors ──────────────────────────────────────────────── */
#define SC_TEXT           lv_color_hex(0xE6EDF3)  /* Primary text */
#define SC_TEXT_WHITE     lv_color_hex(0xFFFFFF)  /* Bright white */
#define SC_TEXT_DIM       lv_color_hex(0x808080)  /* Dimmed/disabled text */

#endif /* UI_SKINNYCON_THEME_H */
