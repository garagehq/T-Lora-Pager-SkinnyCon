/**
 * @file      ui_schedule.cpp
 * @brief     SkinnyCon 2026 Schedule
 * @details   Day submenus with focusable talk rows that scroll.
 *            When a day is selected, its talk rows get added to the group.
 *            When going back, the group is cleaned and day items re-added.
 */
#include "ui_define.h"
#include "ui_skinnycon_theme.h"

#define SCHED_ACCENT      SC_ACCENT
#define SCHED_WHITE       SC_TEXT
#define SCHED_CYAN        SC_CYAN
#define SCHED_DIM         SC_TEXT_DIM

LV_FONT_DECLARE(font_alibaba_12);

static lv_obj_t *sched_menu = NULL;
static lv_obj_t *day_conts[3] = {NULL, NULL, NULL};
static lv_obj_t *day_pages[3] = {NULL, NULL, NULL};

/* Track which day's rows are in the group */
static int active_day = -1;

/* Store row objects per day so we can add/remove from group */
#define MAX_TALKS 15
static lv_obj_t *day_rows[3][MAX_TALKS];
static int day_row_counts[3] = {0, 0, 0};

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

static const talk_t *days_data[] = { day1_talks, day2_talks, day3_talks };
static const int day_counts[] = {
    sizeof(day1_talks) / sizeof(talk_t),
    sizeof(day2_talks) / sizeof(talk_t),
    sizeof(day3_talks) / sizeof(talk_t),
};
static const char *day_names[] = {"Tue May 12", "Wed May 13", "Thu May 14"};

static void populate_group_for_day(int day);
static void populate_group_for_main(void);

static void sched_back_handler(lv_event_t *e)
{
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
    if (lv_menu_back_btn_is_root(sched_menu, obj)) {
        printf("[SCHED] Back to main menu\n");
        lv_obj_clean(sched_menu);
        lv_obj_del(sched_menu);
        sched_menu = NULL;
        active_day = -1;
        menu_show();
    } else {
        /* Going back from a day subpage to the day list */
        printf("[SCHED] Back from day %d to day list\n", active_day);
        active_day = -1;
        populate_group_for_main();
    }
}

/* Called when a day menu item is clicked */
static void day_selected_cb(lv_event_t *e)
{
    /* Figure out which day was clicked */
    lv_obj_t *target = lv_event_get_target_obj(e);
    for (int d = 0; d < 3; d++) {
        if (target == day_conts[d]) {
            printf("[SCHED] Day %d selected (%s)\n", d, day_names[d]);
            active_day = d;
            populate_group_for_day(d);
            return;
        }
    }
}

static void populate_group_for_main(void)
{
    lv_group_t *g = lv_group_get_default();
    if (!g) return;

    /* Remove all schedule items from group */
    lv_group_remove_all_objs(g);

    /* Add the 3 day menu items */
    for (int d = 0; d < 3; d++) {
        if (day_conts[d]) lv_group_add_obj(g, day_conts[d]);
    }
    printf("[SCHED] Group set to main: %d items\n", (int)lv_group_get_obj_count(g));
}

static void populate_group_for_day(int day)
{
    lv_group_t *g = lv_group_get_default();
    if (!g) return;

    /* Remove all items from group */
    lv_group_remove_all_objs(g);

    /* Add this day's talk rows */
    for (int i = 0; i < day_row_counts[day]; i++) {
        lv_group_add_obj(g, day_rows[day][i]);
    }
    printf("[SCHED] Group set to day %d: %d items\n", day, (int)lv_group_get_obj_count(g));
}

static void sched_setup(lv_obj_t *parent)
{
    printf("[SCHED] Setup starting\n");
    active_day = -1;

    lv_group_t *g = lv_group_get_default();
    sched_menu = create_menu(parent, sched_back_handler);

    lv_obj_t *main_page = lv_menu_page_create(sched_menu, NULL);

    for (int d = 0; d < 3; d++) {
        /* Create day subpage with focusable rows */
        day_pages[d] = lv_menu_page_create(sched_menu, NULL);
        day_row_counts[d] = 0;

        const talk_t *talks = days_data[d];
        int n = day_counts[d];
        for (int i = 0; i < n && i < MAX_TALKS; i++) {
            /* Regular lv_obj — focusable but won't trigger page load */
            lv_obj_t *row = lv_obj_create(day_pages[d]);
            lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
            lv_obj_set_style_bg_opa(row, LV_OPA_0, 0);
            lv_obj_set_style_border_width(row, 0, 0);
            lv_obj_set_style_radius(row, 0, 0);
            lv_obj_set_style_pad_ver(row, 3, 0);
            lv_obj_set_style_pad_hor(row, 4, 0);

            char buf[80];
            snprintf(buf, sizeof(buf), "%s  %s", talks[i].time, talks[i].title);
            lv_obj_t *lbl = lv_label_create(row);
            lv_label_set_text(lbl, buf);
            lv_obj_set_style_text_color(lbl, talks[i].is_break ? SCHED_DIM : SCHED_WHITE, 0);
            lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
            lv_obj_set_width(lbl, LV_PCT(100));

            day_rows[d][i] = row;
            day_row_counts[d]++;
        }

        /* Create the day menu item on the main page */
        day_conts[d] = lv_menu_cont_create(main_page);
        lv_obj_t *day_lbl = lv_label_create(day_conts[d]);
        lv_label_set_text(day_lbl, day_names[d]);
        lv_obj_set_style_text_color(day_lbl, SCHED_ACCENT, 0);

        lv_menu_set_load_page_event(sched_menu, day_conts[d], day_pages[d]);

        /* Listen for click to swap group contents */
        lv_obj_add_event_cb(day_conts[d], day_selected_cb, LV_EVENT_CLICKED, NULL);
    }

    lv_menu_set_page(sched_menu, main_page);

    /* Initially only 3 day items in group */
    populate_group_for_main();

    printf("[SCHED] Setup complete\n");
}

static void sched_exit(lv_obj_t *parent)
{
    printf("[SCHED] Exit\n");
    if (sched_menu) {
        lv_obj_clean(sched_menu);
        lv_obj_del(sched_menu);
        sched_menu = NULL;
    }
    active_day = -1;
    memset(day_conts, 0, sizeof(day_conts));
    memset(day_pages, 0, sizeof(day_pages));
    memset(day_rows, 0, sizeof(day_rows));
    memset(day_row_counts, 0, sizeof(day_row_counts));
}

app_t ui_schedule_main = {sched_setup, sched_exit, NULL};
