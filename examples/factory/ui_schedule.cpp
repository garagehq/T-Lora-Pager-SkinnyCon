/**
 * @file      ui_schedule.cpp
 * @brief     Conference Schedule — inspired by Supercon 2025 badge
 * @details   Browse conference talks and events. Scrollable list with
 *            time, title, and speaker. Rotary/keyboard navigates items.
 *            Ported from Supercon 2025 badge schedule browser concept.
 */
#include "ui_define.h"

#define SCHED_BG          lv_color_hex(0x1A1A1A)  /* Hackaday grey */
#define SCHED_ACCENT      lv_color_hex(0xE39810)  /* Hackaday yellow */
#define SCHED_WHITE       lv_color_hex(0xE6EDF3)
#define SCHED_GREEN       lv_color_hex(0xABC5A0)  /* Sage green */
#define SCHED_CYAN        lv_color_hex(0x58A6FF)
#define SCHED_DIM         lv_color_hex(0x808080)
#define MAX_TALKS         20

LV_FONT_DECLARE(font_alibaba_12);
LV_FONT_DECLARE(font_alibaba_24);

static lv_obj_t *sched_cont = NULL;
static int selected_talk = 0;

typedef struct {
    const char *time;
    const char *title;
    const char *speaker;
    bool is_break;
} talk_t;

/* Sample schedule — can be replaced with OTA data */
static const talk_t talks[] = {
    {"09:00", "Opening Ceremony",          "Organizers",       false},
    {"09:30", "Badge Hacking 101",         "Sprite_tm",        false},
    {"10:00", "LoRa Mesh Networks",        "Travis Goodspeed", false},
    {"10:30", "Coffee Break",              "",                 true},
    {"11:00", "Reverse Engineering SoCs",  "Bunnie Huang",     false},
    {"11:30", "FPGA Badge Design",         "esden",            false},
    {"12:00", "Lunch",                     "",                 true},
    {"13:30", "Soldering Workshop",        "Badge Team",       false},
    {"14:00", "RF Hacking with SDR",       "Mike Ossmann",     false},
    {"14:30", "Lightning Talks",           "Community",        false},
    {"15:00", "Badge Assembly Contest",    "Everyone",         false},
    {"15:30", "Closing & Awards",          "Organizers",       false},
};
static const int num_talks = sizeof(talks) / sizeof(talks[0]);

static void sched_update_highlight(lv_obj_t *list)
{
    for (int i = 0; i < (int)lv_obj_get_child_count(list) && i < num_talks; i++) {
        lv_obj_t *row = lv_obj_get_child(list, i);
        if (i == selected_talk) {
            lv_obj_set_style_bg_color(row, lv_color_hex(0x30363D), 0);
            lv_obj_set_style_border_side(row, LV_BORDER_SIDE_LEFT, 0);
            lv_obj_set_style_border_width(row, 3, 0);
            lv_obj_set_style_border_color(row, SCHED_ACCENT, 0);
        } else {
            lv_obj_set_style_bg_color(row, (i % 2) ? lv_color_hex(0x161B22) : SCHED_BG, 0);
            lv_obj_set_style_border_width(row, 0, 0);
        }
    }
}

static void sched_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_KEY) {
        uint32_t key = lv_event_get_key(e);
        lv_obj_t *list = (lv_obj_t *)lv_event_get_user_data(e);
        if (key == LV_KEY_DOWN || key == LV_KEY_RIGHT) {
            selected_talk = (selected_talk + 1) % num_talks;
            sched_update_highlight(list);
            /* Scroll to keep selection visible */
            lv_obj_t *sel = lv_obj_get_child(list, selected_talk);
            if (sel) lv_obj_scroll_to_view(sel, LV_ANIM_ON);
        } else if (key == LV_KEY_UP || key == LV_KEY_LEFT) {
            selected_talk = (selected_talk + num_talks - 1) % num_talks;
            sched_update_highlight(list);
            lv_obj_t *sel = lv_obj_get_child(list, selected_talk);
            if (sel) lv_obj_scroll_to_view(sel, LV_ANIM_ON);
        }
    }
}

static void sched_setup(lv_obj_t *parent)
{
    selected_talk = 0;

    sched_cont = lv_obj_create(parent);
    lv_obj_set_size(sched_cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(sched_cont, SCHED_BG, 0);
    lv_obj_set_style_bg_opa(sched_cont, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(sched_cont, 0, 0);
    lv_obj_set_style_radius(sched_cont, 0, 0);
    lv_obj_set_style_pad_all(sched_cont, 0, 0);
    lv_obj_set_flex_flow(sched_cont, LV_FLEX_FLOW_COLUMN);

    /* Header */
    lv_obj_t *header = lv_obj_create(sched_cont);
    lv_obj_set_size(header, LV_PCT(100), 28);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x21262D), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_pad_hor(header, 8, 0);

    lv_obj_t *title = lv_label_create(header);
    lv_label_set_text(title, LV_SYMBOL_LIST " Schedule");
    lv_obj_set_style_text_color(title, SCHED_ACCENT, 0);
    lv_obj_set_style_text_font(title, &font_alibaba_12, 0);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t *day_lbl = lv_label_create(header);
    lv_label_set_text(day_lbl, "Day 1 - Saturday");
    lv_obj_set_style_text_color(day_lbl, SCHED_WHITE, 0);
    lv_obj_align(day_lbl, LV_ALIGN_RIGHT_MID, 0, 0);

    /* Talk list */
    lv_obj_t *list = lv_obj_create(sched_cont);
    lv_obj_set_size(list, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(list, 1);
    lv_obj_set_style_bg_color(list, SCHED_BG, 0);
    lv_obj_set_style_bg_opa(list, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_style_radius(list, 0, 0);
    lv_obj_set_style_pad_all(list, 0, 0);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_scroll_dir(list, LV_DIR_VER);

    for (int i = 0; i < num_talks; i++) {
        lv_obj_t *row = lv_obj_create(list);
        lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_radius(row, 0, 0);
        lv_obj_set_style_pad_ver(row, 3, 0);
        lv_obj_set_style_pad_hor(row, 8, 0);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);

        /* Time */
        lv_obj_t *time_lbl = lv_label_create(row);
        lv_label_set_text(time_lbl, talks[i].time);
        lv_obj_set_style_text_color(time_lbl, SCHED_CYAN, 0);
        lv_obj_set_style_min_width(time_lbl, 45, 0);

        /* Title */
        lv_obj_t *title_lbl = lv_label_create(row);
        lv_label_set_text(title_lbl, talks[i].title);
        if (talks[i].is_break) {
            lv_obj_set_style_text_color(title_lbl, SCHED_DIM, 0);
        } else {
            lv_obj_set_style_text_color(title_lbl, SCHED_WHITE, 0);
        }
        lv_obj_set_flex_grow(title_lbl, 1);
        lv_label_set_long_mode(title_lbl, LV_LABEL_LONG_CLIP);

        /* Speaker */
        if (talks[i].speaker[0] != '\0') {
            lv_obj_t *spk_lbl = lv_label_create(row);
            lv_label_set_text(spk_lbl, talks[i].speaker);
            lv_obj_set_style_text_color(spk_lbl, SCHED_GREEN, 0);
            lv_obj_set_style_min_width(spk_lbl, 100, 0);
            lv_obj_set_style_text_align(spk_lbl, LV_TEXT_ALIGN_RIGHT, 0);
        }
    }

    sched_update_highlight(list);

    /* Register input */
    lv_obj_add_event_cb(sched_cont, sched_event_cb, LV_EVENT_KEY, list);
    lv_group_t *g = lv_group_get_default();
    if (g) lv_group_add_obj(g, sched_cont);
}

static void sched_exit(lv_obj_t *parent)
{
    if (sched_cont) {
        lv_obj_del(sched_cont);
        sched_cont = NULL;
    }
}

app_t ui_schedule_main = {sched_setup, sched_exit, NULL};
