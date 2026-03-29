/**
 * @file      ui_nametag.cpp
 * @brief     SkinnyCon Nametag — editable name badge with conference info
 * @details   Conference badge with 5 display modes cycled via rotary encoder:
 *            0 = Name + subtitle (editable via lv_textarea)
 *            1 = Fullscreen giant name
 *            2 = About SkinnyCon
 *            3 = Code of Conduct
 *            4 = Badge info (hardware specs)
 *            Encoder click = go back to menu (or confirm edit).
 *            Encoder rotate = cycle modes (when not editing).
 *            Physical keyboard types into lv_textarea (mode 0).
 */
#include "ui_define.h"
#include <string.h>
#include "ui_skinnycon_theme.h"

#define SUPERCON_BG       SC_BG
#define SUPERCON_ACCENT   SC_ACCENT
#define SUPERCON_WHITE    SC_TEXT
#define SUPERCON_GREEN    SC_GREEN
#define SUPERCON_PANEL    SC_PANEL

#define NAME_MAX_LEN      24
#define SUBTITLE_MAX_LEN  32
#define NUM_MODES         5

LV_FONT_DECLARE(font_alibaba_40);
LV_FONT_DECLARE(font_alibaba_24);
LV_FONT_DECLARE(font_alibaba_12);

static lv_obj_t *nametag_cont = NULL;
static lv_obj_t *name_label = NULL;
static lv_obj_t *subtitle_label = NULL;
static lv_obj_t *mode_label = NULL;
static lv_obj_t *name_textarea = NULL;  /* lv_textarea for editing */
static uint8_t display_mode = 0;
static bool is_editing = false;

/* Editable name storage (non-static so idle screen can read) */
char nametag_user_name[NAME_MAX_LEN + 1] = "YOUR NAME";
char nametag_user_subtitle[SUBTITLE_MAX_LEN + 1] = "SkinnyCon 2026";
#define user_name nametag_user_name
#define user_subtitle nametag_user_subtitle

static void nametag_rebuild(void);

/* ── Mode 0: Name + Subtitle ──────────────────────────────────── */
static void nametag_build_name_mode(void)
{
    /* Top accent bar */
    lv_obj_t *bar = lv_obj_create(nametag_cont);
    lv_obj_remove_style_all(bar);
    lv_obj_set_size(bar, LV_PCT(100), 3);
    lv_obj_set_style_bg_color(bar, SC_TEAL, 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);

    if (is_editing) {
        /* Show textarea for editing */
        name_textarea = lv_textarea_create(nametag_cont);
        lv_obj_set_size(name_textarea, LV_PCT(90), 40);
        lv_textarea_set_max_length(name_textarea, NAME_MAX_LEN);
        lv_textarea_set_text(name_textarea, user_name);
        lv_textarea_set_one_line(name_textarea, true);
        lv_obj_set_style_text_font(name_textarea, &font_alibaba_24, 0);
        lv_obj_set_style_text_color(name_textarea, SUPERCON_WHITE, 0);
        lv_obj_set_style_text_align(name_textarea, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_border_color(name_textarea, SC_ACCENT, 0);
        lv_obj_set_style_border_width(name_textarea, 2, 0);

        /* Add textarea to group so keyboard input reaches it */
        lv_group_t *g = lv_group_get_default();
        if (g) lv_group_add_obj(g, name_textarea);
        lv_group_focus_obj(name_textarea);

        printf("[NAMETAG] Editing mode: textarea created and focused\n");

        mode_label = lv_label_create(nametag_cont);
        lv_label_set_text(mode_label, "Type name  Click=done");
        lv_obj_set_style_text_color(mode_label, SUPERCON_ACCENT, 0);
        lv_obj_set_style_text_font(mode_label, &font_alibaba_12, 0);
        lv_obj_set_style_text_align(mode_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_width(mode_label, LV_PCT(100));
    } else {
        /* Display mode — show name and subtitle as labels */
        name_label = lv_label_create(nametag_cont);
        lv_label_set_text(name_label, user_name);
        lv_obj_set_style_text_font(name_label, &font_alibaba_40, 0);
        lv_obj_set_style_text_color(name_label, SUPERCON_WHITE, 0);
        lv_obj_set_style_text_align(name_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_width(name_label, LV_PCT(100));
        lv_label_set_long_mode(name_label, LV_LABEL_LONG_SCROLL_CIRCULAR);

        subtitle_label = lv_label_create(nametag_cont);
        lv_label_set_text(subtitle_label, user_subtitle);
        lv_obj_set_style_text_font(subtitle_label, &font_alibaba_24, 0);
        lv_obj_set_style_text_color(subtitle_label, SUPERCON_GREEN, 0);
        lv_obj_set_style_text_align(subtitle_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_width(subtitle_label, LV_PCT(100));

        /* Bottom accent bar */
        lv_obj_t *bot = lv_obj_create(nametag_cont);
        lv_obj_remove_style_all(bot);
        lv_obj_set_size(bot, LV_PCT(100), 3);
        lv_obj_set_style_bg_color(bot, SC_TEAL, 0);
        lv_obj_set_style_bg_opa(bot, LV_OPA_COVER, 0);

        mode_label = lv_label_create(nametag_cont);
        lv_label_set_text(mode_label, "Click=edit  Rotate=mode");
        lv_obj_set_style_text_color(mode_label, SC_TEXT_DIM, 0);
        lv_obj_set_style_text_font(mode_label, &font_alibaba_12, 0);
        lv_obj_set_style_text_align(mode_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_width(mode_label, LV_PCT(100));
    }
}

/* ── Mode 1: Fullscreen giant name ─────────────────────────────── */
static void nametag_build_fullscreen_mode(void)
{
    name_label = lv_label_create(nametag_cont);
    lv_label_set_text(name_label, user_name);
    lv_obj_set_style_text_font(name_label, &font_alibaba_40, 0);
    lv_obj_set_style_text_color(name_label, SUPERCON_ACCENT, 0);
    lv_obj_set_style_text_align(name_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(name_label, LV_PCT(100));
    lv_label_set_long_mode(name_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_center(name_label);
}

/* ── Mode 2: About SkinnyCon ───────────────────────────────────── */
static void nametag_build_about_mode(void)
{
    lv_obj_t *title = lv_label_create(nametag_cont);
    lv_label_set_text(title, "SKINNYCON 2026");
    lv_obj_set_style_text_font(title, &font_alibaba_12, 0);
    lv_obj_set_style_text_color(title, SUPERCON_ACCENT, 0);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(title, LV_PCT(100));

    lv_obj_t *info = lv_label_create(nametag_cont);
    lv_label_set_text(info,
        "May 12-14, 2026\n"
        "Huntsville, Alabama\n"
        "I2C Invention to Innovation Center\n"
        "UAH Campus\n\n"
        "The best (and only) conference for\n"
        "federal government TSCM professionals.\n\n"
        "Speakers, classes, workshops,\n"
        "sponsors, swag, and good snacks.\n\n"
        "Hosted by Skinny R&D\n"
        "Jason Baird, President"
    );
    lv_obj_set_style_text_font(info, &font_alibaba_12, 0);
    lv_obj_set_style_text_color(info, SUPERCON_WHITE, 0);
    lv_obj_set_width(info, LV_PCT(100));
    lv_label_set_long_mode(info, LV_LABEL_LONG_WRAP);

    mode_label = lv_label_create(nametag_cont);
    lv_label_set_text(mode_label, "Click=back  Rotate=mode");
    lv_obj_set_style_text_font(mode_label, &font_alibaba_12, 0);
    lv_obj_set_style_text_color(mode_label, SC_TEXT_DIM, 0);
    lv_obj_set_style_text_align(mode_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(mode_label, LV_PCT(100));
}

/* ── Mode 3: Code of Conduct ───────────────────────────────────── */
static void nametag_build_coc_mode(void)
{
    lv_obj_t *title = lv_label_create(nametag_cont);
    lv_label_set_text(title, "(!) CODE OF CONDUCT");
    lv_obj_set_style_text_font(title, &font_alibaba_12, 0);
    lv_obj_set_style_text_color(title, SUPERCON_ACCENT, 0);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(title, LV_PCT(100));

    lv_obj_t *coc = lv_label_create(nametag_cont);
    lv_label_set_text(coc,
        "- Be kind, considerate, respectful\n"
        "- Behave professionally\n"
        "- Respect differing viewpoints\n"
        "- Be mindful of personal space\n"
        "- Obey venue rules\n\n"
        "I2C / UAH campus - shared space.\n"
        "Do not explore beyond con areas.\n"
        "Do not play with door locks!\n"
        "UAH is 100% tobacco free."
    );
    lv_obj_set_style_text_font(coc, &font_alibaba_12, 0);
    lv_obj_set_style_text_color(coc, SUPERCON_WHITE, 0);
    lv_obj_set_width(coc, LV_PCT(100));
    lv_label_set_long_mode(coc, LV_LABEL_LONG_WRAP);

    mode_label = lv_label_create(nametag_cont);
    lv_label_set_text(mode_label, "Click=back  Rotate=mode");
    lv_obj_set_style_text_font(mode_label, &font_alibaba_12, 0);
    lv_obj_set_style_text_color(mode_label, SC_TEXT_DIM, 0);
    lv_obj_set_style_text_align(mode_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(mode_label, LV_PCT(100));
}

/* ── Mode 4: Badge hardware info ──────────────────────────────── */
static void nametag_build_badge_info_mode(void)
{
    lv_obj_t *title = lv_label_create(nametag_cont);
    lv_label_set_text(title, "BADGE INFO");
    lv_obj_set_style_text_font(title, &font_alibaba_12, 0);
    lv_obj_set_style_text_color(title, SUPERCON_ACCENT, 0);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(title, LV_PCT(100));

    lv_obj_t *info = lv_label_create(nametag_cont);
    lv_label_set_text(info,
        "Device: T-LoRa-Pager\n"
        "MCU: ESP32-S3 240MHz\n"
        "Display: 480x222 IPS\n"
        "Radio: SX1262 LoRa 915MHz\n"
        "GPS: u-blox MIA-M10Q\n"
        "NFC: ST25R3911B\n\n"
        "Created by: Cyril Engmann\n"
        "Garage Agency LLC"
    );
    lv_obj_set_style_text_font(info, &font_alibaba_12, 0);
    lv_obj_set_style_text_color(info, SUPERCON_WHITE, 0);
    lv_obj_set_width(info, LV_PCT(100));
    lv_label_set_long_mode(info, LV_LABEL_LONG_WRAP);

    mode_label = lv_label_create(nametag_cont);
    lv_label_set_text(mode_label, "Click=back  Rotate=mode");
    lv_obj_set_style_text_font(mode_label, &font_alibaba_12, 0);
    lv_obj_set_style_text_color(mode_label, SC_TEXT_DIM, 0);
    lv_obj_set_style_text_align(mode_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(mode_label, LV_PCT(100));
}

/* ── Mode switching ────────────────────────────────────────────── */
static void nametag_rebuild(void)
{
    printf("[NAMETAG] rebuild: mode=%d editing=%d\n", display_mode, is_editing);

    /* Save textarea content before cleaning */
    if (name_textarea) {
        const char *text = lv_textarea_get_text(name_textarea);
        if (text && strlen(text) > 0) {
            strncpy(user_name, text, NAME_MAX_LEN);
            user_name[NAME_MAX_LEN] = '\0';
            printf("[NAMETAG] Saved name from textarea: '%s'\n", user_name);
        }
        name_textarea = NULL;
    }

    lv_obj_clean(nametag_cont);
    name_label = NULL;
    subtitle_label = NULL;
    mode_label = NULL;

    /* Center short content, top-align long content */
    if (display_mode <= 1 && !is_editing) {
        lv_obj_set_flex_align(nametag_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    } else {
        lv_obj_set_flex_align(nametag_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    }
    lv_obj_scroll_to_y(nametag_cont, 0, LV_ANIM_OFF);

    switch (display_mode) {
    case 0: nametag_build_name_mode(); break;
    case 1: nametag_build_fullscreen_mode(); break;
    case 2: nametag_build_about_mode(); break;
    case 3: nametag_build_coc_mode(); break;
    case 4: nametag_build_badge_info_mode(); break;
    }

    /* Re-focus nametag_cont for non-editing modes so encoder works */
    if (!is_editing) {
        lv_group_t *g = lv_group_get_default();
        if (g) {
            /* Remove any stale objects, add nametag_cont */
            lv_group_remove_all_objs(g);
            lv_group_add_obj(g, nametag_cont);
            lv_group_focus_obj(nametag_cont);
        }
    }
}

static void nametag_exit(lv_obj_t *parent);

static void nametag_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    printf("[NAMETAG-EVT] code=%d mode=%d editing=%d\n",
           (int)code, display_mode, is_editing);

    if (code == LV_EVENT_CLICKED) {
        if (is_editing) {
            /* Confirm edit and return to display mode */
            is_editing = false;
            printf("[NAMETAG] Edit confirmed, rebuilding display mode\n");
            nametag_rebuild();
        } else if (display_mode == 0) {
            /* Enter edit mode */
            is_editing = true;
            printf("[NAMETAG] Entering edit mode\n");
            nametag_rebuild();
        } else {
            /* Exit to menu from info modes */
            printf("[NAMETAG] Exiting to menu\n");
            nametag_exit(NULL);
            menu_show();
        }
        return;
    }

    if (code != LV_EVENT_KEY) return;

    uint32_t key = lv_event_get_key(e);
    printf("[NAMETAG-KEY] key=%lu (RIGHT=%d LEFT=%d)\n",
           (unsigned long)key, LV_KEY_RIGHT, LV_KEY_LEFT);

    if (key == LV_KEY_ESC) {
        nametag_exit(NULL);
        menu_show();
        return;
    }

    /* Don't switch modes while editing */
    if (is_editing) return;

    if (key == LV_KEY_RIGHT || key == LV_KEY_DOWN) {
        display_mode = (display_mode + 1) % NUM_MODES;
        nametag_rebuild();
    } else if (key == LV_KEY_LEFT || key == LV_KEY_UP) {
        display_mode = (display_mode + NUM_MODES - 1) % NUM_MODES;
        nametag_rebuild();
    }
}

static void nametag_setup(lv_obj_t *parent)
{
    printf("[NAMETAG] Setup starting\n");

    nametag_cont = lv_obj_create(parent);
    lv_obj_remove_style_all(nametag_cont);
    lv_obj_set_size(nametag_cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(nametag_cont, SUPERCON_BG, 0);
    lv_obj_set_style_bg_opa(nametag_cont, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(nametag_cont, 4, 0);
    lv_obj_set_flex_flow(nametag_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(nametag_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(nametag_cont, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_scroll_dir(nametag_cont, LV_DIR_VER);

    lv_obj_add_event_cb(nametag_cont, nametag_event_cb, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(nametag_cont, nametag_event_cb, LV_EVENT_CLICKED, NULL);

    /* Add to group and focus so encoder works immediately */
    lv_group_t *g = lv_group_get_default();
    if (g) {
        lv_group_add_obj(g, nametag_cont);
        lv_group_focus_obj(nametag_cont);
        printf("[NAMETAG] Added to group and focused, group has %d objs\n",
               (int)lv_group_get_obj_count(g));
    }

    display_mode = 0;
    is_editing = false;
    name_textarea = NULL;
    nametag_build_name_mode();

    printf("[NAMETAG] Setup complete. Mode=%d\n", display_mode);
}

static void nametag_exit(lv_obj_t *parent)
{
    printf("[NAMETAG] Exit: cleaning up\n");

    /* Save textarea content if still editing */
    if (name_textarea) {
        const char *text = lv_textarea_get_text(name_textarea);
        if (text && strlen(text) > 0) {
            strncpy(user_name, text, NAME_MAX_LEN);
            user_name[NAME_MAX_LEN] = '\0';
        }
        name_textarea = NULL;
    }

    /* Clean group before deleting objects */
    lv_group_t *g = lv_group_get_default();
    if (g) {
        lv_group_remove_all_objs(g);
        printf("[NAMETAG] Cleared group, now has %d objs\n",
               (int)lv_group_get_obj_count(g));
    }

    if (nametag_cont) {
        lv_obj_del(nametag_cont);
        nametag_cont = NULL;
    }
    name_label = NULL;
    subtitle_label = NULL;
    mode_label = NULL;
    is_editing = false;
}

app_t ui_nametag_main = {nametag_setup, nametag_exit, NULL};
