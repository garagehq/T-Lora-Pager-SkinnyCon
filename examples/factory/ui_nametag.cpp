/**
 * @file      ui_nametag.cpp
 * @brief     Nametag screen — inspired by Supercon 2025 badge
 * @details   Large name display for conference badges. Shows name in large font
 *            with optional subtitle. Rotary/keyboard switches between display modes.
 *            Ported from Supercon 2025 badge nametag app design.
 */
#include "ui_define.h"

/* Supercon-inspired color palette */
#define SUPERCON_BG       lv_color_hex(0x1A1A1A)  /* Hackaday grey */
#define SUPERCON_ACCENT   lv_color_hex(0xE39810)  /* Hackaday yellow */
#define SUPERCON_WHITE    lv_color_hex(0xFFFFFF)
#define SUPERCON_GREEN    lv_color_hex(0xABC5A0)  /* Sage green */

LV_FONT_DECLARE(font_alibaba_40);
LV_FONT_DECLARE(font_alibaba_24);
LV_FONT_DECLARE(font_alibaba_100);

static lv_obj_t *nametag_cont = NULL;
static lv_obj_t *name_label = NULL;
static lv_obj_t *subtitle_label = NULL;
static lv_obj_t *badge_id_label = NULL;
static lv_obj_t *mode_label = NULL;
static uint8_t display_mode = 0;  /* 0=name, 1=fullscreen name, 2=badge ID */

static const char *default_name = "SkinnyCon";
static const char *default_subtitle = "T-LoRa-Pager";

static void nametag_build_name_mode(lv_obj_t *parent)
{
    /* Top accent bar */
    lv_obj_t *accent_bar = lv_obj_create(parent);
    lv_obj_set_size(accent_bar, LV_PCT(100), 4);
    lv_obj_set_style_bg_color(accent_bar, SUPERCON_ACCENT, 0);
    lv_obj_set_style_bg_opa(accent_bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(accent_bar, 0, 0);
    lv_obj_set_style_radius(accent_bar, 0, 0);
    lv_obj_set_style_pad_all(accent_bar, 0, 0);

    /* Name in large font */
    name_label = lv_label_create(parent);
    lv_label_set_text(name_label, default_name);
    lv_obj_set_style_text_font(name_label, &font_alibaba_40, 0);
    lv_obj_set_style_text_color(name_label, SUPERCON_WHITE, 0);
    lv_obj_set_style_text_align(name_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(name_label, LV_PCT(100));

    /* Subtitle */
    subtitle_label = lv_label_create(parent);
    lv_label_set_text(subtitle_label, default_subtitle);
    lv_obj_set_style_text_font(subtitle_label, &font_alibaba_24, 0);
    lv_obj_set_style_text_color(subtitle_label, SUPERCON_GREEN, 0);
    lv_obj_set_style_text_align(subtitle_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(subtitle_label, LV_PCT(100));

    /* Bottom accent bar */
    lv_obj_t *bottom_bar = lv_obj_create(parent);
    lv_obj_set_size(bottom_bar, LV_PCT(100), 4);
    lv_obj_set_style_bg_color(bottom_bar, SUPERCON_ACCENT, 0);
    lv_obj_set_style_bg_opa(bottom_bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(bottom_bar, 0, 0);
    lv_obj_set_style_radius(bottom_bar, 0, 0);
    lv_obj_set_style_pad_all(bottom_bar, 0, 0);

    /* Mode hint */
    mode_label = lv_label_create(parent);
    lv_label_set_text(mode_label, LV_SYMBOL_REFRESH " Rotate to switch mode");
    lv_obj_set_style_text_color(mode_label, lv_color_hex(0x808080), 0);
    lv_obj_set_style_text_align(mode_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(mode_label, LV_PCT(100));
}

static void nametag_build_fullscreen_mode(lv_obj_t *parent)
{
    /* Giant name across the ultra-wide display */
    name_label = lv_label_create(parent);
    lv_label_set_text(name_label, default_name);
    lv_obj_set_style_text_font(name_label, &font_alibaba_100, 0);
    lv_obj_set_style_text_color(name_label, SUPERCON_ACCENT, 0);
    lv_obj_set_style_text_align(name_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(name_label, LV_PCT(100));
    lv_obj_center(name_label);
}

static void nametag_build_badge_id_mode(lv_obj_t *parent)
{
    /* Badge info screen */
    lv_obj_t *title = lv_label_create(parent);
    lv_label_set_text(title, "BADGE INFO");
    lv_obj_set_style_text_font(title, &font_alibaba_24, 0);
    lv_obj_set_style_text_color(title, SUPERCON_ACCENT, 0);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(title, LV_PCT(100));

    /* Two-column info panel */
    lv_obj_t *info_panel = lv_obj_create(parent);
    lv_obj_set_size(info_panel, LV_PCT(90), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(info_panel, lv_color_hex(0x2A2A2A), 0);
    lv_obj_set_style_bg_opa(info_panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(info_panel, SUPERCON_ACCENT, 0);
    lv_obj_set_style_border_width(info_panel, 1, 0);
    lv_obj_set_style_radius(info_panel, 8, 0);
    lv_obj_set_style_pad_all(info_panel, 10, 0);
    lv_obj_set_flex_flow(info_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_center(info_panel);

    badge_id_label = lv_label_create(info_panel);
    lv_label_set_text_fmt(badge_id_label,
        "Device: T-LoRa-Pager\n"
        "MCU: ESP32-S3\n"
        "Display: 480x222\n"
        "Radio: SX1262 LoRa\n"
        "Freq: 868 MHz"
    );
    lv_obj_set_style_text_color(badge_id_label, SUPERCON_WHITE, 0);

    mode_label = lv_label_create(parent);
    lv_label_set_text(mode_label, LV_SYMBOL_REFRESH " Rotate to switch mode");
    lv_obj_set_style_text_color(mode_label, lv_color_hex(0x808080), 0);
    lv_obj_set_style_text_align(mode_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(mode_label, LV_PCT(100));
}

static void nametag_rebuild(lv_obj_t *parent)
{
    /* Clear and rebuild */
    lv_obj_clean(nametag_cont);

    switch (display_mode) {
    case 0: nametag_build_name_mode(nametag_cont); break;
    case 1: nametag_build_fullscreen_mode(nametag_cont); break;
    case 2: nametag_build_badge_id_mode(nametag_cont); break;
    }
}

static void nametag_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED || code == LV_EVENT_KEY) {
        if (code == LV_EVENT_KEY) {
            uint32_t key = lv_event_get_key(e);
            if (key == LV_KEY_RIGHT || key == LV_KEY_DOWN) {
                display_mode = (display_mode + 1) % 3;
                nametag_rebuild(nametag_cont);
            } else if (key == LV_KEY_LEFT || key == LV_KEY_UP) {
                display_mode = (display_mode + 2) % 3;
                nametag_rebuild(nametag_cont);
            }
        }
    }
}

static void nametag_setup(lv_obj_t *parent)
{
    nametag_cont = lv_obj_create(parent);
    lv_obj_set_size(nametag_cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(nametag_cont, SUPERCON_BG, 0);
    lv_obj_set_style_bg_opa(nametag_cont, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(nametag_cont, 0, 0);
    lv_obj_set_style_radius(nametag_cont, 0, 0);
    lv_obj_set_style_pad_all(nametag_cont, 8, 0);
    lv_obj_set_flex_flow(nametag_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(nametag_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(nametag_cont, LV_SCROLLBAR_MODE_OFF);

    lv_obj_add_event_cb(nametag_cont, nametag_event_cb, LV_EVENT_KEY, NULL);
    lv_group_t *g = lv_group_get_default();
    if (g) lv_group_add_obj(g, nametag_cont);

    display_mode = 0;
    nametag_build_name_mode(nametag_cont);
}

static void nametag_exit(lv_obj_t *parent)
{
    if (nametag_cont) {
        lv_obj_del(nametag_cont);
        nametag_cont = NULL;
    }
    name_label = NULL;
    subtitle_label = NULL;
    badge_id_label = NULL;
    mode_label = NULL;
}

app_t ui_nametag_main = {nametag_setup, nametag_exit, NULL};
