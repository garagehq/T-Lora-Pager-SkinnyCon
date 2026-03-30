/**
 * @file      ui_schedule.cpp
 * @brief     SkinnyCon 2026 Schedule — Settings-style submenus per day
 * @details   Copies the exact Settings app pattern:
 *            - Main page has 3 lv_menu_cont items (one per day)
 *            - Each day opens a subpage with talk labels (NOT in group)
 *            - Back button on subpage returns to day list
 *            - Back button on day list returns to main menu
 */
#include "ui_define.h"
#include "ui_skinnycon_theme.h"

#define SCHED_ACCENT      SC_ACCENT
#define SCHED_WHITE       SC_TEXT
#define SCHED_CYAN        SC_CYAN
#define SCHED_DIM         SC_TEXT_DIM

LV_FONT_DECLARE(font_alibaba_12);

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
static const char *day_names[] = {
    "Tue May 12",
    "Wed May 13",
    "Thu May 14"
};

static void sched_back_handler(lv_event_t *e)
{
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
    if (lv_menu_back_btn_is_root(sched_menu, obj)) {
        printf("[SCHED] Back to main menu\n");
        lv_obj_clean(sched_menu);
        lv_obj_del(sched_menu);
        sched_menu = NULL;
        menu_show();
    }
}

/*
 * Create a subpage for one day — just labels, NOT added to group.
 * The lv_menu page scrolls automatically when content overflows.
 * Back button in the menu header navigates back to day list.
 */
static lv_obj_t *create_day_subpage(const talk_t *talks, int n)
{
    lv_obj_t *page = lv_menu_page_create(sched_menu, NULL);

    for (int i = 0; i < n; i++) {
        /* Use lv_menu_cont so the menu knows about this content,
         * but do NOT add to group and do NOT set load_page_event */
        lv_obj_t *cont = lv_menu_cont_create(page);

        char buf[80];
        snprintf(buf, sizeof(buf), "%s  %s", talks[i].time, talks[i].title);
        lv_obj_t *lbl = lv_label_create(cont);
        lv_label_set_text(lbl, buf);
        lv_obj_set_style_text_color(lbl, talks[i].is_break ? SCHED_DIM : SCHED_WHITE, 0);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(lbl, LV_PCT(100));
    }

    printf("[SCHED] Created day subpage with %d talks (NOT in group)\n", n);
    return page;
}

static void sched_setup(lv_obj_t *parent)
{
    printf("[SCHED] Setup starting\n");

    lv_group_t *g = lv_group_get_default();
    sched_menu = create_menu(parent, sched_back_handler);

    lv_obj_t *main_page = lv_menu_page_create(sched_menu, NULL);

    /*
     * Exactly like Settings: create subpages, then create
     * lv_menu_cont items on main_page that link to them.
     * Only the main page menu_cont items go in the group.
     */
    for (int d = 0; d < 3; d++) {
        /* Create the day's subpage (talks inside, not in group) */
        lv_obj_t *day_page = create_day_subpage(days[d], day_counts[d]);

        /* Create the main page menu item that links to it */
        lv_obj_t *day_cont = lv_menu_cont_create(main_page);
        lv_obj_t *day_lbl = lv_label_create(day_cont);
        lv_label_set_text(day_lbl, day_names[d]);
        lv_obj_set_style_text_color(day_lbl, SCHED_ACCENT, 0);

        /* This is what makes clicking the item load the subpage */
        lv_menu_set_load_page_event(sched_menu, day_cont, day_page);

        /* Only the day items go in the group (3 total) */
        if (g) lv_group_add_obj(g, day_cont);
    }

    lv_menu_set_page(sched_menu, main_page);

    printf("[SCHED] Setup complete. %d items in group (should be 3 days)\n",
           g ? (int)lv_group_get_obj_count(g) : -1);
}

static void sched_exit(lv_obj_t *parent)
{
    lv_group_t *g = lv_group_get_default();
    printf("[SCHED] Exit — group has %d objs\n",
           g ? (int)lv_group_get_obj_count(g) : -1);
    if (sched_menu) {
        lv_obj_clean(sched_menu);
        lv_obj_del(sched_menu);
        sched_menu = NULL;
    }
}

app_t ui_schedule_main = {sched_setup, sched_exit, NULL};
