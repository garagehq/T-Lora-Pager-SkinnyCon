/**
 * @file      ui_schedule.cpp
 * @brief     SkinnyCon 2026 Schedule
 * @details   Exact Settings pattern: all items added to group at setup.
 *            lv_menu handles subpage navigation. Encoder focuses visible items.
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

static const talk_t *days_data[] = { day1_talks, day2_talks, day3_talks };
static const int day_counts[] = {
    sizeof(day1_talks) / sizeof(talk_t),
    sizeof(day2_talks) / sizeof(talk_t),
    sizeof(day3_talks) / sizeof(talk_t),
};
static const char *day_names[] = {"Tue May 12", "Wed May 13", "Thu May 14"};

/* Store row refs per day for deferred group swap */
#define MAX_TALKS 15
static lv_obj_t *day_rows[3][MAX_TALKS];
static int day_row_counts[3] = {0, 0, 0};
static lv_obj_t *day_conts[3] = {NULL, NULL, NULL};
static int pending_day = -1;

static void deferred_group_swap(lv_timer_t *t)
{
    lv_timer_del(t);
    lv_group_t *g = lv_group_get_default();
    if (!g) return;

    if (pending_day >= 0 && pending_day < 3) {
        /* Entering a day subpage — back button first, then that day's rows */
        printf("[SCHED] Deferred: switching to day %d, removing all from group\n", pending_day);
        lv_group_remove_all_objs(g);
        lv_obj_t *back_btn = lv_menu_get_main_header_back_button(sched_menu);
        if (back_btn) {
            lv_group_add_obj(g, back_btn);
            printf("[SCHED]   added back_btn=%p to group\n", (void *)back_btn);
        }
        for (int i = 0; i < day_row_counts[pending_day]; i++) {
            printf("[SCHED]   adding row[%d][%d]=%p to group\n", pending_day, i, (void *)day_rows[pending_day][i]);
            lv_group_add_obj(g, day_rows[pending_day][i]);
        }
        printf("[SCHED] Deferred: group now has %d items, focused=%p\n",
               (int)lv_group_get_obj_count(g), (void *)lv_group_get_focused(g));
    } else {
        /* Returning to main page — show only day items */
        printf("[SCHED] Deferred: switching to main page, removing all from group\n");
        lv_group_remove_all_objs(g);
        for (int d = 0; d < 3; d++) {
            if (day_conts[d]) {
                printf("[SCHED]   adding day_conts[%d]=%p to group\n", d, (void *)day_conts[d]);
                lv_group_add_obj(g, day_conts[d]);
            }
        }
        printf("[SCHED] Deferred: group now has %d items, focused=%p\n",
               (int)lv_group_get_obj_count(g), (void *)lv_group_get_focused(g));
    }
    pending_day = -1;
}

static void sched_back_handler(lv_event_t *e)
{
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
    printf("[SCHED] Back handler: obj=%p is_root=%d\n",
           (void *)obj, lv_menu_back_btn_is_root(sched_menu, obj));
    if (lv_menu_back_btn_is_root(sched_menu, obj)) {
        printf("[SCHED] Back to main menu\n");
        lv_obj_clean(sched_menu);
        lv_obj_del(sched_menu);
        sched_menu = NULL;
        menu_show();
    } else {
        /* Going back from day subpage to main */
        printf("[SCHED] Back to day list (was on day %d)\n", pending_day);
        pending_day = -1;  /* -1 means main page */
        lv_timer_create(deferred_group_swap, 50, NULL);
    }
}

/* When a talk row gains focus, scroll it into view */
static void talk_row_focus_cb(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target_obj(e);
    lv_obj_t *parent = lv_obj_get_parent(target);
    printf("[SCHED] Focus on obj=%p parent=%p\n", (void *)target, (void *)parent);
    if (parent) {
        lv_obj_scroll_to_view(target, LV_ANIM_ON);
        printf("[SCHED]   scrolled to view\n");
    }
}

static void day_clicked_cb(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target_obj(e);
    for (int d = 0; d < 3; d++) {
        if (target == day_conts[d]) {
            printf("[SCHED] Day %d clicked\n", d);
            pending_day = d;
            lv_timer_create(deferred_group_swap, 50, NULL);
            return;
        }
    }
}

static void sched_setup(lv_obj_t *parent)
{
    printf("[SCHED] Setup starting\n");

    lv_group_t *g = lv_group_get_default();
    sched_menu = create_menu(parent, sched_back_handler);

    lv_obj_t *main_page = lv_menu_page_create(sched_menu, NULL);

    memset(day_rows, 0, sizeof(day_rows));
    memset(day_row_counts, 0, sizeof(day_row_counts));
    memset(day_conts, 0, sizeof(day_conts));

    for (int d = 0; d < 3; d++) {
        lv_obj_t *sub_page = lv_menu_page_create(sched_menu, NULL);
        day_row_counts[d] = 0;

        const talk_t *talks = days_data[d];
        int n = day_counts[d];
        for (int i = 0; i < n && i < MAX_TALKS; i++) {
            /* Plain lv_obj instead of lv_menu_cont — lv_menu won't try to
               navigate these on click, preventing the freeze */
            lv_obj_t *cont = lv_obj_create(sub_page);
            lv_obj_remove_style_all(cont);
            lv_obj_set_size(cont, LV_PCT(100), LV_SIZE_CONTENT);
            lv_obj_set_style_pad_ver(cont, 4, 0);
            lv_obj_set_style_pad_hor(cont, 8, 0);
            lv_obj_remove_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
            /* Focus highlight since remove_style_all strips theme focus styling */
            lv_obj_set_style_bg_color(cont, SCHED_CYAN, LV_STATE_FOCUS_KEY);
            lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, LV_STATE_FOCUS_KEY);

            char buf[80];
            snprintf(buf, sizeof(buf), "%s  %s", talks[i].time, talks[i].title);
            lv_obj_t *lbl = lv_label_create(cont);
            lv_label_set_text(lbl, buf);
            lv_obj_set_style_text_color(lbl, talks[i].is_break ? SCHED_DIM : SCHED_WHITE, 0);
            lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
            lv_obj_set_width(lbl, LV_PCT(100));
            /* Scroll into view on focus */
            lv_obj_add_event_cb(cont, talk_row_focus_cb, LV_EVENT_FOCUSED, NULL);

            /* Store ref but do NOT add to group yet */
            day_rows[d][i] = cont;
            day_row_counts[d]++;
            printf("[SCHED]   row[%d][%d] = %p '%s'\n", d, i, (void *)cont, buf);
        }

        /* Main page entry */
        day_conts[d] = lv_menu_cont_create(main_page);
        lv_obj_t *day_lbl = lv_label_create(day_conts[d]);
        lv_label_set_text(day_lbl, day_names[d]);
        lv_obj_set_style_text_color(day_lbl, SCHED_WHITE, 0);
        lv_menu_set_load_page_event(sched_menu, day_conts[d], sub_page);
        lv_obj_add_event_cb(day_conts[d], day_clicked_cb, LV_EVENT_CLICKED, NULL);
        if (g) lv_group_add_obj(g, day_conts[d]);
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
    memset(day_rows, 0, sizeof(day_rows));
    memset(day_row_counts, 0, sizeof(day_row_counts));
    memset(day_conts, 0, sizeof(day_conts));
    pending_day = -1;
}

app_t ui_schedule_main = {sched_setup, sched_exit, NULL};
