/**
 * @file      ui_schedule.cpp
 * @brief     SkinnyCon 2026 Schedule — uses lv_menu like Settings
 * @details   3-day schedule as sched_menu items. Encoder scrolls through talks,
 *            back button returns to main sched_menu. Each day is a subpage.
 */
#include "ui_define.h"
#include "ui_skinnycon_theme.h"

#define SCHED_BG          SC_BG
#define SCHED_ACCENT      SC_ACCENT
#define SCHED_WHITE       SC_TEXT
#define SCHED_CYAN        SC_CYAN
#define SCHED_DIM         SC_TEXT_DIM

LV_FONT_DECLARE(font_alibaba_12);
LV_FONT_DECLARE(font_alibaba_24);

static lv_obj_t *sched_menu = NULL;

typedef struct {
    const char *time;
    const char *title;
    bool is_break;
} talk_t;

static const talk_t day1_talks[] = {
    {"0800", "Check-in/Breakfast/Vendor Setup", true},
    {"0900", "Welcome", false},
    {"0915", "How to CTF", false},
    {"0930", "Tech Ops Case Files: Real Stories", false},
    {"1030", "Break", true},
    {"1100", "Intro to Reverse Engineering", false},
    {"1150", "Lunch", true},
    {"1300", "Training 1", false},
    {"1300", "Upping IQ on I&Q: GNU Radio", false},
    {"1500", "Training 2", false},
    {"1500", "RF Situational Awareness in IPMS", false},
    {"1500", "Behind the Waterfall: SDRs", false},
};

static const talk_t day2_talks[] = {
    {"0800", "Check-in/Breakfast/Vendor Setup", true},
    {"0900", "Welcome", false},
    {"0910", "H&E Field Ultrasonic+Magnetic Sensor", false},
    {"0940", "Electronic Sniffing K-9s", false},
    {"1030", "Break", true},
    {"1100", "Reverse Engineering Medical Devices", false},
    {"1150", "Lunch", true},
    {"1330", "BYOD Upgrades and Updates", false},
    {"1405", "Training 3", false},
    {"1405", "Next-Gen Portable X-Ray in Action", false},
    {"1600", "TSCM Workforce Survey and Report", false},
};

static const talk_t day3_talks[] = {
    {"0800", "Check-in/Breakfast/Vendor Chats", true},
    {"0900", "Welcome", false},
    {"0910", "Supertooth", false},
    {"0945", "State of TSCM Education", false},
    {"1035", "Break", true},
    {"1100", "Converge and Cringe", false},
    {"1150", "Lunch", true},
    {"1300", "Commercial TSCM Panel", false},
    {"1350", "Break", true},
    {"1420", "TBD", false},
    {"1520", "Closing Remarks", false},
};

static const talk_t *days[] = { day1_talks, day2_talks, day3_talks };
static const int day_counts[] = {
    sizeof(day1_talks) / sizeof(talk_t),
    sizeof(day2_talks) / sizeof(talk_t),
    sizeof(day3_talks) / sizeof(talk_t),
};
static const char *day_names[] = {"Tue May 12", "Wed May 13", "Thu May 14"};

static void sched_back_handler(lv_event_t *e)
{
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
    if (lv_menu_back_btn_is_root(sched_menu, obj)) {
        printf("[SCHED] Back button, exiting\n");
        lv_obj_clean(sched_menu);
        lv_obj_del(sched_menu);
        sched_menu = NULL;
        menu_show();
    }
}

/* Build a subpage for one day's talks */
static lv_obj_t *create_day_page(lv_obj_t *m, const talk_t *talks, int n, lv_group_t *g)
{
    lv_obj_t *page = lv_menu_page_create(m, NULL);

    for (int i = 0; i < n; i++) {
        /* Use lv_obj (not lv_menu_cont) so clicking doesn't try to load a subpage */
        lv_obj_t *row = lv_obj_create(page);
        lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(row, LV_OPA_0, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_radius(row, 0, 0);
        lv_obj_set_style_pad_ver(row, 3, 0);
        lv_obj_set_style_pad_hor(row, 8, 0);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);

        lv_obj_t *time_lbl = lv_label_create(row);
        lv_label_set_text(time_lbl, talks[i].time);
        lv_obj_set_style_text_color(time_lbl, SCHED_CYAN, 0);
        lv_obj_set_style_min_width(time_lbl, 45, 0);

        lv_obj_t *title_lbl = lv_label_create(row);
        lv_label_set_text(title_lbl, talks[i].title);
        lv_obj_set_style_text_color(title_lbl, talks[i].is_break ? SCHED_DIM : SCHED_WHITE, 0);
        lv_obj_set_flex_grow(title_lbl, 1);
        lv_label_set_long_mode(title_lbl, LV_LABEL_LONG_CLIP);

        /* Add to group so encoder can scroll through talks */
        if (g) lv_group_add_obj(g, row);
    }

    return page;
}

static void sched_setup(lv_obj_t *parent)
{
    printf("[SCHED] Setup starting\n");

    lv_group_t *g = lv_group_get_default();
    sched_menu = create_menu(parent, sched_back_handler);

    lv_obj_t *main_page = lv_menu_page_create(sched_menu, NULL);

    /* Header */
    lv_obj_t *header_cont = lv_obj_create(main_page);
    lv_obj_set_size(header_cont, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(header_cont, LV_OPA_0, 0);
    lv_obj_set_style_border_width(header_cont, 0, 0);
    lv_obj_set_style_pad_all(header_cont, 4, 0);
    lv_obj_t *header_lbl = lv_label_create(header_cont);
    lv_label_set_text(header_lbl, "SkinnyCon 2026 Schedule");
    lv_obj_set_style_text_color(header_lbl, SCHED_ACCENT, 0);
    lv_obj_set_style_text_font(header_lbl, &font_alibaba_12, 0);

    /* Create subpages for each day */
    for (int d = 0; d < 3; d++) {
        lv_obj_t *day_page = create_day_page(sched_menu, days[d], day_counts[d], g);

        lv_obj_t *day_cont = lv_menu_cont_create(main_page);
        lv_obj_t *day_lbl = lv_label_create(day_cont);
        lv_label_set_text(day_lbl, day_names[d]);
        lv_obj_set_style_text_color(day_lbl, SCHED_ACCENT, 0);
        lv_menu_set_load_page_event(sched_menu, day_cont, day_page);
        if (g) lv_group_add_obj(g, day_cont);
    }

    lv_menu_set_page(sched_menu, main_page);

    printf("[SCHED] Setup complete. %d items in group\n",
           g ? (int)lv_group_get_obj_count(g) : -1);
}

static void sched_exit(lv_obj_t *parent)
{
    printf("[SCHED] Exit\n");
    if (sched_menu) {
        lv_obj_clean(sched_menu);
        lv_obj_del(sched_menu);
        sched_menu = NULL;
    }
}

app_t ui_schedule_main = {sched_setup, sched_exit, NULL};
