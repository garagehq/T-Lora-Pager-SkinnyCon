/**
 * @file      ui_nametag.cpp
 * @brief     SkinnyCon Nametag — editable name badge
 * @details   Uses lv_menu like all other working apps.
 *            Main page shows name/subtitle. Subpages for about/coc/badge info.
 *            Click "Edit Name" to enter textarea for keyboard editing.
 */
#include "ui_define.h"
#include <string.h>
#include "ui_skinnycon_theme.h"
#if defined(ARDUINO) && !defined(LVGL_SIMULATOR)
#include <Preferences.h>
static Preferences nametag_prefs;
#define HAS_PREFERENCES 1
#endif

#define SUPERCON_BG       SC_BG
#define SUPERCON_ACCENT   SC_ACCENT
#define SUPERCON_WHITE    SC_TEXT
#define SUPERCON_GREEN    SC_GREEN

#define NAME_MAX_LEN      24
#define SUBTITLE_MAX_LEN  32

LV_FONT_DECLARE(font_alibaba_40);
LV_FONT_DECLARE(font_alibaba_24);
LV_FONT_DECLARE(font_alibaba_12);

static lv_obj_t *menu = NULL;
static lv_obj_t *name_label = NULL;
static lv_obj_t *subtitle_label = NULL;

/* Editable name storage (non-static so idle screen can read) */
char nametag_user_name[NAME_MAX_LEN + 1] = "YOUR NAME";
char nametag_user_subtitle[SUBTITLE_MAX_LEN + 1] = "SkinnyCon 2026";

static void back_event_handler(lv_event_t *e)
{
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
    if (lv_menu_back_btn_is_root(menu, obj)) {
        printf("[NAMETAG] Back button pressed, exiting\n");
        lv_obj_clean(menu);
        lv_obj_del(menu);
        menu = NULL;
        name_label = NULL;
        subtitle_label = NULL;
        menu_show();
    }
}

static void nametag_save_to_flash() {
#ifdef HAS_PREFERENCES
    nametag_prefs.begin("nametag", false);
    nametag_prefs.putString("name", nametag_user_name);
    nametag_prefs.putString("subtitle", nametag_user_subtitle);
    nametag_prefs.end();
    printf("[NAMETAG] Saved to flash: name='%s' subtitle='%s'\n",
           nametag_user_name, nametag_user_subtitle);
#endif
}

static void nametag_load_from_flash() {
#ifdef HAS_PREFERENCES
    nametag_prefs.begin("nametag", true);  /* read-only */
    String name = nametag_prefs.getString("name", "YOUR NAME");
    String sub = nametag_prefs.getString("subtitle", "SkinnyCon 2026");
    nametag_prefs.end();
    strncpy(nametag_user_name, name.c_str(), NAME_MAX_LEN);
    nametag_user_name[NAME_MAX_LEN] = '\0';
    strncpy(nametag_user_subtitle, sub.c_str(), SUBTITLE_MAX_LEN);
    nametag_user_subtitle[SUBTITLE_MAX_LEN] = '\0';
    printf("[NAMETAG] Loaded from flash: name='%s' subtitle='%s'\n",
           nametag_user_name, nametag_user_subtitle);
#endif
}

static void force_uppercase(char *str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') str[i] -= 32;
    }
}

/* Textarea event handler — copied from LoRa Chat's working pattern */
static void nametag_ta_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target_obj(e);

    lv_indev_t *indev = lv_indev_active();
    if (!indev) return;

    bool edited = lv_obj_has_state(ta, LV_STATE_EDITED);

    printf("[NAMETAG-TA] code=%d edited=%d indev_type=%d\n",
           (int)code, edited, indev ? (int)indev->type : -1);

    /* Force uppercase on every text change */
    if (code == LV_EVENT_VALUE_CHANGED) {
        const char *text = lv_textarea_get_text(ta);
        if (text) {
            char upper[NAME_MAX_LEN + 1];
            int i;
            for (i = 0; text[i] && i < NAME_MAX_LEN; i++) {
                upper[i] = (text[i] >= 'a' && text[i] <= 'z') ? text[i] - 32 : text[i];
            }
            upper[i] = '\0';
            if (strcmp(upper, text) != 0) {
                uint32_t cur = lv_textarea_get_cursor_pos(ta);
                lv_textarea_set_text(ta, upper);
                lv_textarea_set_cursor_pos(ta, cur);
            }
        }
    }

    if (code == LV_EVENT_KEY) {
        lv_key_t key = *(lv_key_t *)lv_event_get_param(e);
        printf("[NAMETAG-TA] key=%d\n", (int)key);
        if (key == LV_KEY_ENTER) {
            /* Enter key = save and go back */
            const char *text = lv_textarea_get_text(ta);
            if (text && strlen(text) > 0) {
                strncpy(nametag_user_name, text, NAME_MAX_LEN);
                nametag_user_name[NAME_MAX_LEN] = '\0';
                force_uppercase(nametag_user_name);
                nametag_save_to_flash();
            }
            if (name_label) lv_label_set_text(name_label, nametag_user_name);
            printf("[NAMETAG] Name saved: '%s'\n", nametag_user_name);
            lv_group_set_editing((lv_group_t *)lv_obj_get_group(ta), false);
            disable_keyboard();
            lv_obj_send_event(lv_menu_get_main_header_back_button(menu), LV_EVENT_CLICKED, NULL);
            lv_event_stop_processing(e);
        }
    } else if (code == LV_EVENT_CLICKED) {
        if (indev->type == LV_INDEV_TYPE_ENCODER) {
            if (edited) {
                /* Click while editing = save and exit edit mode */
                const char *text = lv_textarea_get_text(ta);
                if (text && strlen(text) > 0) {
                    strncpy(nametag_user_name, text, NAME_MAX_LEN);
                    nametag_user_name[NAME_MAX_LEN] = '\0';
                force_uppercase(nametag_user_name);
                nametag_save_to_flash();
                }
                if (name_label) lv_label_set_text(name_label, nametag_user_name);
                printf("[NAMETAG] Name saved via click: '%s'\n", nametag_user_name);
                lv_group_set_editing((lv_group_t *)lv_obj_get_group(ta), false);
                disable_keyboard();
            } else {
                /* Click while not editing = enter edit mode */
                printf("[NAMETAG] Entering edit mode\n");
                lv_group_set_editing((lv_group_t *)lv_obj_get_group(ta), true);
            }
        }
    } else if (code == LV_EVENT_FOCUSED) {
        if (edited) {
            printf("[NAMETAG] Textarea focused in edit mode, enabling keyboard\n");
            enable_keyboard();
        }
    } else if (code == LV_EVENT_DEFOCUSED) {
        printf("[NAMETAG] Textarea defocused, disabling keyboard\n");
        disable_keyboard();
        /* Save on defocus */
        const char *text = lv_textarea_get_text(ta);
        if (text && strlen(text) > 0) {
            strncpy(nametag_user_name, text, NAME_MAX_LEN);
            nametag_user_name[NAME_MAX_LEN] = '\0';
                force_uppercase(nametag_user_name);
                nametag_save_to_flash();
        }
        if (name_label) lv_label_set_text(name_label, nametag_user_name);
    }
}

/* (Edit Subtitle removed per user request) */

/* Focusable text row for scrollable subpages — same pattern as Schedule talk rows */
static void scroll_row_focus_cb(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target_obj(e);
    lv_obj_scroll_to_view(target, LV_ANIM_ON);
}

static lv_obj_t *add_text_row(lv_obj_t *page, const char *text, lv_color_t color,
                               const lv_font_t *font, lv_group_t *g)
{
    lv_obj_t *row = lv_obj_create(page);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_ver(row, 2, 0);
    lv_obj_set_style_pad_hor(row, 8, 0);
    lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    /* Focus highlight */
    lv_obj_set_style_bg_color(row, SC_CYAN, LV_STATE_FOCUS_KEY);
    lv_obj_set_style_bg_opa(row, LV_OPA_COVER, LV_STATE_FOCUS_KEY);

    lv_obj_t *lbl = lv_label_create(row);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_color(lbl, color, 0);
    if (font) lv_obj_set_style_text_font(lbl, font, 0);
    lv_obj_set_width(lbl, LV_PCT(100));
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);

    lv_obj_add_event_cb(row, scroll_row_focus_cb, LV_EVENT_FOCUSED, NULL);
    if (g) lv_group_add_obj(g, row);
    return row;
}

static void nametag_setup(lv_obj_t *parent)
{
    printf("[NAMETAG] Setup starting\n");
    nametag_load_from_flash();

    lv_group_t *g = lv_group_get_default();
    menu = create_menu(parent, back_event_handler);

    /* ── Main page: name display + menu items ────────────────────── */
    lv_obj_t *main_page = lv_menu_page_create(menu, NULL);

    /* Name display (big, centered) */
    lv_obj_t *name_cont = lv_obj_create(main_page);
    lv_obj_set_size(name_cont, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(name_cont, LV_OPA_0, 0);
    lv_obj_set_style_border_width(name_cont, 0, 0);
    lv_obj_set_style_pad_ver(name_cont, 10, 0);
    lv_obj_set_flex_flow(name_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(name_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* Teal accent bar */
    lv_obj_t *bar = lv_obj_create(name_cont);
    lv_obj_remove_style_all(bar);
    lv_obj_set_size(bar, LV_PCT(100), 3);
    lv_obj_set_style_bg_color(bar, SC_TEAL, 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);

    name_label = lv_label_create(name_cont);
    lv_label_set_text(name_label, nametag_user_name);
    lv_obj_set_style_text_font(name_label, &font_alibaba_40, 0);
    lv_obj_set_style_text_color(name_label, SUPERCON_WHITE, 0);
    lv_obj_set_style_text_align(name_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(name_label, LV_PCT(100));

    subtitle_label = lv_label_create(name_cont);
    lv_label_set_text(subtitle_label, nametag_user_subtitle);
    lv_obj_set_style_text_font(subtitle_label, &font_alibaba_24, 0);
    lv_obj_set_style_text_color(subtitle_label, SUPERCON_GREEN, 0);
    lv_obj_set_style_text_align(subtitle_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(subtitle_label, LV_PCT(100));

    lv_obj_t *bar2 = lv_obj_create(name_cont);
    lv_obj_remove_style_all(bar2);
    lv_obj_set_size(bar2, LV_PCT(100), 3);
    lv_obj_set_style_bg_color(bar2, SC_TEAL, 0);
    lv_obj_set_style_bg_opa(bar2, LV_OPA_COVER, 0);

    /* ── Menu items (scrollable via encoder) ─────────────────────── */

    /* Edit Name — textarea in a menu subpage */
    lv_obj_t *edit_name_page = lv_menu_page_create(menu, NULL);

    lv_obj_t *ta_label = lv_label_create(edit_name_page);
    lv_label_set_text(ta_label, "Type your name:");
    lv_obj_set_style_text_font(ta_label, &font_alibaba_12, 0);
    lv_obj_set_style_text_color(ta_label, SC_TEXT_DIM, 0);

    lv_obj_t *name_ta = lv_textarea_create(edit_name_page);
    lv_obj_set_size(name_ta, LV_PCT(90), 40);
    lv_textarea_set_max_length(name_ta, NAME_MAX_LEN);
    lv_textarea_set_text(name_ta, nametag_user_name);
    lv_textarea_set_one_line(name_ta, true);
    lv_obj_set_style_text_font(name_ta, &font_alibaba_24, 0);
    lv_obj_add_event_cb(name_ta, nametag_ta_cb, LV_EVENT_ALL, NULL);
    /* Textarea needs to be in group for keyboard input */
    if (g) lv_group_add_obj(g, name_ta);

    lv_obj_t *edit_name_cont = lv_menu_cont_create(main_page);
    lv_obj_t *edit_name_lbl = lv_label_create(edit_name_cont);
    lv_label_set_text(edit_name_lbl, "Edit Name");
    lv_obj_set_style_text_color(edit_name_lbl, SUPERCON_ACCENT, 0);
    lv_menu_set_load_page_event(menu, edit_name_cont, edit_name_page);
    if (g) lv_group_add_obj(g, edit_name_cont);

    /* About SkinnyCon */
    lv_obj_t *about_page = lv_menu_page_create(menu, NULL);
    add_text_row(about_page, "SKINNYCON 2026", SUPERCON_ACCENT, &font_alibaba_24, g);
    add_text_row(about_page, "May 12-14, 2026", SUPERCON_WHITE, NULL, g);
    add_text_row(about_page, "Huntsville, Alabama", SUPERCON_WHITE, NULL, g);
    add_text_row(about_page, "I2C Invention to Innovation Center\nUAH Campus", SUPERCON_WHITE, NULL, g);
    add_text_row(about_page, "The best (and only) conference for federal government TSCM professionals.", SUPERCON_WHITE, NULL, g);
    add_text_row(about_page, "Speakers, classes, workshops, sponsors, swag, and good snacks.", SUPERCON_WHITE, NULL, g);
    add_text_row(about_page, "Hosted by Skinny R&D\nJason Baird, President", SC_TEXT_DIM, NULL, g);

    lv_obj_t *about_cont = lv_menu_cont_create(main_page);
    lv_obj_t *about_lbl = lv_label_create(about_cont);
    lv_label_set_text(about_lbl, "About SkinnyCon");
    lv_menu_set_load_page_event(menu, about_cont, about_page);
    if (g) lv_group_add_obj(g, about_cont);

    /* Code of Conduct */
    lv_obj_t *coc_page = lv_menu_page_create(menu, NULL);
    add_text_row(coc_page, "CODE OF CONDUCT", SUPERCON_ACCENT, &font_alibaba_24, g);
    add_text_row(coc_page, "Be kind, considerate, respectful", SUPERCON_WHITE, NULL, g);
    add_text_row(coc_page, "Behave professionally", SUPERCON_WHITE, NULL, g);
    add_text_row(coc_page, "Respect differing viewpoints", SUPERCON_WHITE, NULL, g);
    add_text_row(coc_page, "Be mindful of personal space", SUPERCON_WHITE, NULL, g);
    add_text_row(coc_page, "Obey venue rules", SUPERCON_WHITE, NULL, g);
    add_text_row(coc_page, "I2C / UAH campus is shared space. Do not explore beyond con areas.", SUPERCON_WHITE, NULL, g);
    add_text_row(coc_page, "Do not play with door locks!", SC_RED, NULL, g);
    add_text_row(coc_page, "UAH is 100% tobacco free.", SC_TEXT_DIM, NULL, g);

    lv_obj_t *coc_cont = lv_menu_cont_create(main_page);
    lv_obj_t *coc_lbl = lv_label_create(coc_cont);
    lv_label_set_text(coc_lbl, "Code of Conduct");
    lv_menu_set_load_page_event(menu, coc_cont, coc_page);
    if (g) lv_group_add_obj(g, coc_cont);

    /* Badge Info */
    lv_obj_t *badge_page = lv_menu_page_create(menu, NULL);
    add_text_row(badge_page, "BADGE INFO", SUPERCON_ACCENT, &font_alibaba_24, g);
    add_text_row(badge_page, "Device: T-LoRa-Pager", SUPERCON_WHITE, NULL, g);
    add_text_row(badge_page, "MCU: ESP32-S3 240MHz", SUPERCON_WHITE, NULL, g);
    add_text_row(badge_page, "Display: 480x222 IPS", SUPERCON_WHITE, NULL, g);
    add_text_row(badge_page, "Radio: SX1262 LoRa 915MHz", SUPERCON_WHITE, NULL, g);
    add_text_row(badge_page, "GPS: u-blox MIA-M10Q", SUPERCON_WHITE, NULL, g);
    add_text_row(badge_page, "NFC: ST25R3911B", SUPERCON_WHITE, NULL, g);
    add_text_row(badge_page, "Created by: Cyril Engmann\nGarage Agency LLC", SC_TEXT_DIM, NULL, g);

    lv_obj_t *badge_cont = lv_menu_cont_create(main_page);
    lv_obj_t *badge_lbl = lv_label_create(badge_cont);
    lv_label_set_text(badge_lbl, "Badge Info");
    lv_menu_set_load_page_event(menu, badge_cont, badge_page);
    if (g) lv_group_add_obj(g, badge_cont);

    lv_menu_set_page(menu, main_page);

    printf("[NAMETAG] Setup complete. %d items in group\n",
           g ? (int)lv_group_get_obj_count(g) : -1);
}

static void nametag_exit(lv_obj_t *parent)
{
    printf("[NAMETAG] Exit — cleaning up\n");
    /* Save name from textarea if still on edit page */
    lv_group_t *g = lv_group_get_default();
    if (g) {
        printf("[NAMETAG] Group had %d objs before cleanup\n",
               (int)lv_group_get_obj_count(g));
    }
    if (menu) {
        lv_obj_clean(menu);
        lv_obj_del(menu);
        menu = NULL;
    }
    if (g) {
        printf("[NAMETAG] Group has %d objs after menu delete\n",
               (int)lv_group_get_obj_count(g));
    }
    name_label = NULL;
    subtitle_label = NULL;
}

app_t ui_nametag_main = {nametag_setup, nametag_exit, NULL};
