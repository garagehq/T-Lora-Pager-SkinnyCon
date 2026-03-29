/**
 * @file      ui_schedule.cpp
 * @brief     SkinnyCon 2026 Schedule — May 12-14, Huntsville AL
 * @details   Browse the 3-day conference schedule using LVGL menu widget.
 *            Scroll through talks, left/right to switch days.
 *            Back button returns to main menu.
 */
#include "ui_define.h"
#include "ui_skinnycon_theme.h"

#define SCHED_BG          SC_BG
#define SCHED_ACCENT      SC_ACCENT
#define SCHED_WHITE       SC_TEXT
#define SCHED_GREEN       SC_GREEN
#define SCHED_CYAN        SC_CYAN
#define SCHED_DIM         SC_TEXT_DIM

LV_FONT_DECLARE(font_alibaba_12);
LV_FONT_DECLARE(font_alibaba_24);

static lv_obj_t *menu = NULL;
static lv_obj_t *sched_list = NULL;
static lv_obj_t *day_label = NULL;
static int selected_talk = 0;
static int current_day = 0;  /* 0=Tue, 1=Wed, 2=Thu */

typedef struct {
    const char *time;
    const char *title;
    const char *speaker;
    bool is_break;
} talk_t;

/* SkinnyCon 2026 — Tuesday, May 12 */
static const talk_t day1_talks[] = {
    {"0800", "Check-in/Breakfast/Vendor Setup",          "",                 true},
    {"0900", "Welcome",                                  "",                 false},
    {"0915", "How to CTF",                               "",                 false},
    {"0930", "Tech Ops Case Files: Real Stories",        "",                 false},
    {"1030", "Break",                                    "",                 true},
    {"1100", "Intro to Reverse Engineering",             "",                 false},
    {"1150", "Lunch",                                    "",                 true},
    {"1300", "Training 1",                               "",                 false},
    {"1300", "Upping IQ on I&Q: GNU Radio",              "",                 false},
    {"1500", "Training 2",                               "",                 false},
    {"1500", "RF Situational Awareness in IPMS",         "",                 false},
    {"1500", "Behind the Waterfall: SDRs",               "",                 false},
};

/* SkinnyCon 2026 — Wednesday, May 13 */
static const talk_t day2_talks[] = {
    {"0800", "Check-in/Breakfast/Vendor Setup",          "",                 true},
    {"0900", "Welcome",                                  "",                 false},
    {"0910", "H&E Field Ultrasonic+Magnetic Sensor",     "",                 false},
    {"0940", "Electronic Sniffing K-9s",                 "",                 false},
    {"1030", "Break",                                    "",                 true},
    {"1100", "Reverse Engineering Medical Devices",      "",                 false},
    {"1150", "Lunch",                                    "",                 true},
    {"1330", "BYOD Upgrades and Updates",                "",                 false},
    {"1405", "Training 3",                               "",                 false},
    {"1405", "Next-Gen Portable X-Ray in Action",        "",                 false},
    {"1600", "TSCM Workforce Survey and Report",         "",                 false},
};

/* SkinnyCon 2026 — Thursday, May 14 */
static const talk_t day3_talks[] = {
    {"0800", "Check-in/Breakfast/Vendor Chats",          "",                 true},
    {"0900", "Welcome",                                  "",                 false},
    {"0910", "Supertooth",                               "",                 false},
    {"0945", "State of TSCM Education",                  "",                 false},
    {"1035", "Break",                                    "",                 true},
    {"1100", "Converge and Cringe",                      "",                 false},
    {"1150", "Lunch",                                    "",                 true},
    {"1300", "Commercial TSCM Panel",                    "",                 false},
    {"1350", "Break",                                    "",                 true},
    {"1420", "TBD",                                      "",                 false},
    {"1520", "Closing Remarks",                          "",                 false},
};

static const talk_t *days[] = { day1_talks, day2_talks, day3_talks };
static const int day_counts[] = {
    sizeof(day1_talks) / sizeof(day1_talks[0]),
    sizeof(day2_talks) / sizeof(day2_talks[0]),
    sizeof(day3_talks) / sizeof(day3_talks[0]),
};
static const char *day_names[] = {
    "Tue May 12",
    "Wed May 13",
    "Thu May 14",
};

static void sched_build_list(void);

static void sched_update_highlight(void)
{
    const int n = day_counts[current_day];
    for (int i = 0; i < (int)lv_obj_get_child_count(sched_list) && i < n; i++) {
        lv_obj_t *row = lv_obj_get_child(sched_list, i);
        if (i == selected_talk) {
            lv_obj_set_style_bg_color(row, SC_BORDER, 0);
            lv_obj_set_style_border_side(row, LV_BORDER_SIDE_LEFT, 0);
            lv_obj_set_style_border_width(row, 3, 0);
            lv_obj_set_style_border_color(row, SCHED_ACCENT, 0);
        } else {
            lv_obj_set_style_bg_color(row, (i % 2) ? SC_PANEL_ALT : SCHED_BG, 0);
            lv_obj_set_style_border_width(row, 0, 0);
        }
    }
}

static void back_event_handler(lv_event_t *e)
{
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
    if (lv_menu_back_btn_is_root(menu, obj)) {
        lv_obj_clean(menu);
        lv_obj_del(menu);
        menu = NULL;
        sched_list = NULL;
        day_label = NULL;
        menu_show();
    }
}

static void sched_key_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_KEY) return;
    uint32_t key = lv_event_get_key(e);
    const int n = day_counts[current_day];

    if (key == LV_KEY_RIGHT) {
        current_day = (current_day + 1) % 3;
        selected_talk = 0;
        sched_build_list();
    } else if (key == LV_KEY_LEFT) {
        current_day = (current_day + 2) % 3;
        selected_talk = 0;
        sched_build_list();
    } else if (key == LV_KEY_DOWN) {
        selected_talk = (selected_talk + 1) % n;
        sched_update_highlight();
        lv_obj_t *sel = lv_obj_get_child(sched_list, selected_talk);
        if (sel) lv_obj_scroll_to_view(sel, LV_ANIM_ON);
    } else if (key == LV_KEY_UP) {
        selected_talk = (selected_talk + n - 1) % n;
        sched_update_highlight();
        lv_obj_t *sel = lv_obj_get_child(sched_list, selected_talk);
        if (sel) lv_obj_scroll_to_view(sel, LV_ANIM_ON);
    }
}

static void sched_build_list(void)
{
    if (day_label) {
        lv_label_set_text_fmt(day_label, LV_SYMBOL_LEFT " %s " LV_SYMBOL_RIGHT, day_names[current_day]);
    }

    if (!sched_list) return;
    lv_obj_clean(sched_list);

    const talk_t *talks = days[current_day];
    const int n = day_counts[current_day];

    for (int i = 0; i < n; i++) {
        lv_obj_t *row = lv_obj_create(sched_list);
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
        lv_obj_set_style_min_width(time_lbl, 40, 0);

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
    }

    sched_update_highlight();
}

static void sched_setup(lv_obj_t *parent)
{
    selected_talk = 0;
    current_day = 0;

    menu = create_menu(parent, back_event_handler);

    lv_obj_t *main_page = lv_menu_page_create(menu, NULL);

    /* Header */
    lv_obj_t *header = lv_obj_create(main_page);
    lv_obj_set_size(header, LV_PCT(100), 28);
    lv_obj_set_style_bg_color(header, SC_HEADER, 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_pad_hor(header, 8, 0);

    lv_obj_t *title = lv_label_create(header);
    lv_label_set_text(title, "SkinnyCon 2026");
    lv_obj_set_style_text_color(title, SCHED_ACCENT, 0);
    lv_obj_set_style_text_font(title, &font_alibaba_12, 0);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 0, 0);

    day_label = lv_label_create(header);
    lv_obj_set_style_text_color(day_label, SCHED_WHITE, 0);
    lv_obj_align(day_label, LV_ALIGN_RIGHT_MID, 0, 0);

    /* Talk list */
    sched_list = lv_obj_create(main_page);
    lv_obj_set_size(sched_list, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(sched_list, 1);
    lv_obj_set_style_bg_color(sched_list, SCHED_BG, 0);
    lv_obj_set_style_bg_opa(sched_list, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(sched_list, 0, 0);
    lv_obj_set_style_radius(sched_list, 0, 0);
    lv_obj_set_style_pad_all(sched_list, 0, 0);
    lv_obj_set_flex_flow(sched_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(sched_list, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_scroll_dir(sched_list, LV_DIR_VER);

    sched_build_list();

    lv_menu_set_page(menu, main_page);

    /* Key handler for day/talk navigation */
    lv_obj_add_event_cb(menu, sched_key_cb, LV_EVENT_KEY, NULL);
}

static void sched_exit(lv_obj_t *parent)
{
    if (menu) {
        lv_obj_clean(menu);
        lv_obj_del(menu);
        menu = NULL;
    }
    sched_list = NULL;
    day_label = NULL;
}

app_t ui_schedule_main = {sched_setup, sched_exit, NULL};
