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
LV_FONT_DECLARE(GoogleSansCodeMono_12);

static lv_obj_t *sched_menu = NULL;

typedef struct {
    const char *time;
    const char *title;
    bool is_break;
    bool is_child;
} talk_t;

static const talk_t day1_talks[] = {
    {"0800-0900", "Check-in/Breakfast/Vendor Chats/Vendor Setup", true, false},
    {"0900-0915", "Welcome", false, false},
    {"0915-0930", "How to Do a CTF", false, false},
    {"0930-1020", "An Introduction to Reverse Engineering", false, false},
    {"1020-1050", "Break", true, false},
    {"1100-1150", "Tech Ops Case Files: Real Stories", false, false},
    {"1150-1300", "Lunch", true, false},
    {"1300-1500", "Training Session 1", false, false},
    {"", "1300-1400 RF Situational Awareness in an IPMS", false, true},
    {"", "1300-1400 Next-Generation Portable X-Ray in Action", false, true},
    {"", "1300-1500 Upping Your IQ on I&Q (GNU Radio)", false, true},
    {"", "1405-1435 EMI Awareness", false, true},
    {"", "1405-1500 The Rise of Modern Signaling Threats", false, true},
    {"1500-1700", "Training Session 2", false, false},
    {"", "1505-1535 Controlling Mobile Sensors and Signals", false, true},
    {"", "1505-1600 The Rise of Modern Signaling Threats", false, true},
    {"", "1505-1700 Intro to Reverse Engineering Lab", false, true},
    {"", "1540-1610 EMI Awareness", false, true},
};

static const talk_t day2_talks[] = {
    {"0800-0900", "Check-in/Breakfast/Vendor Chats/Vendor Setup", true, false},
    {"0900-0910", "Welcome", false, false},
    {"0910-0940", "H&E Field Ultrasonic + Magnetic Sensor", false, false},
    {"0940-1030", "Electronic Sniffing K-9s", false, false},
    {"1030-1100", "Break", true, false},
    {"1100-1150", "Reverse Engineering Medical Devices", false, false},
    {"1150-1330", "Lunch", true, false},
    {"1330-1400", "BYOD Upgrades and Updates", false, false},
    {"1405-1600", "Training Session 3", false, false},
    {"", "1405-1435 Controlling Mobile Sensors and Signals", false, true},
    {"", "1405-1505 Next-Generation Portable X-Ray in Action", false, true},
    {"", "1405-1600 Behind the Waterfall: Spectral Ops with SDRs", false, true},
    {"", "1440-1540 RF Situational Awareness in an IPMS", false, true},
    {"1600-1650", "TEMPEST Emissions and Mitigations", false, false},
};

static const talk_t day3_talks[] = {
    {"0800-0900", "Check-in/Breakfast/Vendor Chats", true, false},
    {"0900-0910", "Welcome", false, false},
    {"0910-0940", "Supertooth", false, false},
    {"0940-1030", "State of TSCM Education Panel", false, false},
    {"1030-1100", "Break", true, false},
    {"1100-1150", "Converge and Cringe", false, false},
    {"1150-1300", "Lunch", true, false},
    {"1300-1350", "Commercial TSCM Panel", false, false},
    {"1350-1420", "Break", true, false},
    {"1420-1510", "TSCM Workforce Survey", false, false},
    {"1529-1600", "Closing Remarks", false, false},
};

static const talk_t *days_data[] = { day1_talks, day2_talks, day3_talks };
static const int day_counts[] = {
    sizeof(day1_talks) / sizeof(talk_t),
    sizeof(day2_talks) / sizeof(talk_t),
    sizeof(day3_talks) / sizeof(talk_t),
};
static const char *day_names[] = {"Tue May 12", "Wed May 13", "Thu May 14"};

/* Store row refs per day for deferred group swap */
#define MAX_TALKS 24
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
        /* Returning to main page — back button first, then day items */
        printf("[SCHED] Deferred: switching to main page, removing all from group\n");
        lv_group_remove_all_objs(g);
        lv_obj_t *back_btn = lv_menu_get_main_header_back_button(sched_menu);
        if (back_btn) {
            lv_group_add_obj(g, back_btn);
            printf("[SCHED]   added back_btn=%p to group\n", (void *)back_btn);
        }
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

    /* Add back button to group first so encoder can reach it */
    if (g) {
        lv_obj_t *back_btn = lv_menu_get_main_header_back_button(sched_menu);
        if (back_btn) lv_group_add_obj(g, back_btn);
    }

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

            char buf[128];
            if (talks[i].is_child) {
                snprintf(buf, sizeof(buf), "  |-- %s", talks[i].title);
            } else {
                snprintf(buf, sizeof(buf), "%-10s %s", talks[i].time, talks[i].title);
            }
            lv_obj_t *lbl = lv_label_create(cont);
            lv_label_set_text(lbl, buf);
            lv_obj_set_style_text_font(lbl, &GoogleSansCodeMono_12, 0);
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
        lv_obj_set_style_text_font(day_lbl, &GoogleSansCodeMono_12, 0);
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
