/**
 * @file      ui_schedule.cpp
 * @brief     SkinnyCon 2026 Schedule — May 12-14, Huntsville AL
 * @details   Browse the 3-day conference schedule. Scrollable list with
 *            time, title, and speaker. Rotary/keyboard navigates items.
 *            Left/Right arrows switch between days.
 */
#include "ui_define.h"

#define SCHED_BG          lv_color_hex(0x1A1A1A)  /* Hackaday grey */
#define SCHED_ACCENT      lv_color_hex(0xE39810)  /* Hackaday yellow */
#define SCHED_WHITE       lv_color_hex(0xE6EDF3)
#define SCHED_GREEN       lv_color_hex(0xABC5A0)  /* Sage green */
#define SCHED_CYAN        lv_color_hex(0x58A6FF)
#define SCHED_DIM         lv_color_hex(0x808080)

LV_FONT_DECLARE(font_alibaba_12);
LV_FONT_DECLARE(font_alibaba_24);

static lv_obj_t *sched_cont = NULL;
static lv_obj_t *sched_list = NULL;
static lv_obj_t *day_label = NULL;
static lv_obj_t *back_btn = NULL;
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
    {"0800", "Check-in / Breakfast / Vendor Setup",     "",                 true},
    {"0900", "Welcome",                                  "",                 false},
    {"0915", "How to CTF",                               "",                 false},
    {"0930", "Tech Ops Case Files: Real Stories",        "",                 false},
    {"1030", "Break",                                    "",                 true},
    {"1100", "Intro to Reverse Engineering",             "",                 false},
    {"1150", "Lunch",                                    "",                 true},
    {"1300", "Training 1: SDR + GNU Radio",              "",                 false},
    {"1500", "Training 2: RF Situational Awareness",     "",                 false},
    {"1500", "Spectral Ops with SDRs",                   "",                 false},
};

/* SkinnyCon 2026 — Wednesday, May 13 */
static const talk_t day2_talks[] = {
    {"0800", "Check-in / Breakfast / Vendor Setup",     "",                 true},
    {"0900", "Welcome",                                  "",                 false},
    {"0910", "Ultrasonic + Magnetic Sensor",             "",                 false},
    {"0940", "Electronic Sniffing K-9s",                 "",                 false},
    {"1030", "Break",                                    "",                 true},
    {"1100", "Reverse Engineering Medical Devices",      "",                 false},
    {"1150", "Lunch",                                    "",                 true},
    {"1335", "BYOD Upgrades and Updates",                "",                 false},
    {"1405", "Training 3: Portable X-Ray",               "",                 false},
    {"1600", "TSCM Workforce Survey",                    "",                 false},
};

/* SkinnyCon 2026 — Thursday, May 14 */
static const talk_t day3_talks[] = {
    {"0800", "Check-in / Breakfast / Vendor Chats",     "",                 true},
    {"0900", "Welcome",                                  "",                 false},
    {"0910", "Supertooth",                               "",                 false},
    {"0945", "State of TSCM Education",                  "",                 false},
    {"1035", "Break",                                    "",                 true},
    {"1100", "TBD",                                      "",                 false},
    {"1150", "Lunch",                                    "",                 true},
    {"1300", "Commercial TSCM Panel",                    "",                 false},
    {"1350", "Break",                                    "",                 true},
    {"1420", "Converge and Cringe",                      "",                 false},
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
            lv_obj_set_style_bg_color(row, lv_color_hex(0x30363D), 0);
            lv_obj_set_style_border_side(row, LV_BORDER_SIDE_LEFT, 0);
            lv_obj_set_style_border_width(row, 3, 0);
            lv_obj_set_style_border_color(row, SCHED_ACCENT, 0);
        } else {
            lv_obj_set_style_bg_color(row, lv_color_hex(0x1A1A1A), 0);
            lv_obj_set_style_border_side(row, LV_BORDER_SIDE_NONE, 0);
            lv_obj_set_style_border_width(row, 0, 0);
        }
    }
}

static void sched_update_day_label(void)
{
    if (day_label) {
        lv_label_set_text(day_label, day_names[current_day]);
    }
}

static void sched_update_selected_text(void)
{
    const int n = day_counts[current_day];
    if (selected_talk >= n) selected_talk = n - 1;
    if (selected_talk < 0) selected_talk = 0;
    
    const talk_t *talk = &days[current_day][selected_talk];
    
    if (talk->is_break) {
        lv_obj_t *first_child = lv_obj_get_child(sched_list, selected_talk);
        if (first_child) {
            lv_obj_t *time_label = lv_obj_get_child(first_child, 0);
            lv_obj_t *title_label = lv_obj_get_child(first_child, 1);
            if (time_label) lv_label_set_text(time_label, "");
            if (title_label) lv_label_set_text(title_label, talk->title);
        }
    } else {
        lv_obj_t *first_child = lv_obj_get_child(sched_list, selected_talk);
        if (first_child) {
            lv_obj_t *time_label = lv_obj_get_child(first_child, 0);
            lv_obj_t *title_label = lv_obj_get_child(first_child, 1);
            if (time_label) lv_label_set_text(time_label, talk->time);
            if (title_label) lv_label_set_text(title_label, talk->title);
        }
    }
}

static void sched_update_all(void)
{
    sched_update_highlight();
    sched_update_day_label();
    sched_update_selected_text();
}

static void sched_next_talk(void)
{
    const int n = day_counts[current_day];
    if (selected_talk < n - 1) {
        selected_talk++;
        sched_update_all();
        lv_obj_scroll_to_y(sched_list, selected_talk * 50, LV_ANIM_ON);
    }
}

static void sched_prev_talk(void)
{
    if (selected_talk > 0) {
        selected_talk--;
        sched_update_all();
        lv_obj_scroll_to_y(sched_list, selected_talk * 50, LV_ANIM_ON);
    }
}

static void sched_next_day(void)
{
    current_day = (current_day + 1) % 3;
    selected_talk = 0;
    sched_build_list();
    sched_update_all();
}

static void sched_prev_day(void)
{
    current_day = (current_day + 2) % 3;
    selected_talk = 0;
    sched_build_list();
    sched_update_all();
}

static void sched_back_event_handler(lv_event_t *e)
{
    (void)e;
    app_set_current(APP_MAIN);
}

static void sched_build_list(void)
{
    if (sched_list) {
        lv_obj_del(sched_list);
        sched_list = NULL;
    }
    
    sched_list = lv_list_create(sched_cont);
    lv_obj_set_size(sched_list, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(sched_list, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(sched_list, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(sched_list, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_align(sched_list, LV_ALIGN_TOP_MID, 0, 50);
    
    const int n = day_counts[current_day];
    for (int i = 0; i < n; i++) {
        const talk_t *talk = &days[current_day][i];
        lv_obj_t *row = lv_obj_create(sched_list);
        lv_obj_set_size(row, lv_pct(100), 50);
        lv_obj_set_style_pad_row(row, 0, LV_PART_MAIN);
        lv_obj_set_style_bg_color(row, SCHED_BG, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_pad_all(row, 10, 0);
        lv_obj_set_layout(row, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        
        lv_obj_t *time_label = lv_label_create(row);
        lv_label_set_text(time_label, talk->time);
        lv_obj_set_style_text_color(time_label, talk->is_break ? SCHED_CYAN : SCHED_GREEN, 0);
        lv_obj_set_style_text_font(time_label, &font_alibaba_12, 0);
        
        lv_obj_t *title_label = lv_label_create(row);
        lv_label_set_text(title_label, talk->title);
        lv_obj_set_style_text_color(title_label, SCHED_WHITE, 0);
        lv_obj_set_style_text_font(title_label, &font_alibaba_12, 0);
        
        if (talk->speaker[0] != '\0') {
            lv_obj_t *speaker_label = lv_label_create(row);
            lv_label_set_text(speaker_label, talk->speaker);
            lv_obj_set_style_text_color(speaker_label, SCHED_DIM, 0);
            lv_obj_set_style_text_font(speaker_label, &font_alibaba_12, 0);
        }
    }
}

static void sched_create_back_button(lv_obj_t *parent)
{
    back_btn = lv_btn_create(parent);
    lv_obj_set_size(back_btn, 60, 40);
    lv_obj_align(back_btn, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_set_style_bg_color(back_btn, lv_color_hex(0x30363D), 0);
    lv_obj_set_style_border_width(back_btn, 0, 0);
    lv_obj_set_style_pad_all(back_btn, 5, 0);
    
    lv_obj_t *label = lv_label_create(back_btn);
    lv_label_set_text(label, "←");
    lv_obj_set_style_text_color(label, SCHED_WHITE, 0);
    lv_obj_set_style_text_font(label, &font_alibaba_12, 0);
    
    lv_obj_add_event_cb(back_btn, sched_back_event_handler, LV_EVENT_CLICKED, NULL);
}

static void ui_schedule_enter(lv_obj_t *parent)
{
    sched_cont = lv_obj_create(parent);
    lv_obj_set_size(sched_cont, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(sched_cont, SCHED_BG, 0);
    lv_obj_set_style_pad_all(sched_cont, 0, 0);
    lv_obj_align(sched_cont, LV_ALIGN_TOP_MID, 0, 0);
    
    sched_create_back_button(parent);
    
    day_label = lv_label_create(parent);
    lv_label_set_text(day_label, day_names[current_day]);
    lv_obj_set_style_text_color(day_label, SCHED_WHITE, 0);
    lv_obj_set_style_text_font(day_label, &font_alibaba_24, 0);
    lv_obj_align(day_label, LV_ALIGN_TOP_LEFT, 80, 10);
    
    sched_build_list();
    sched_update_all();
}

static void ui_schedule_exit(lv_obj_t *parent)
{
    (void)parent;
    if (sched_list) {
        lv_obj_del(sched_list);
        sched_list = NULL;
    }
    if (back_btn) {
        lv_obj_del(back_btn);
        back_btn = NULL;
    }
    if (day_label) {
        lv_obj_del(day_label);
        day_label = NULL;
    }
}

app_t ui_schedule_main = {
    .setup_func_cb = ui_schedule_enter,
    .exit_func_cb = ui_schedule_exit,
    .user_data = nullptr,
};
