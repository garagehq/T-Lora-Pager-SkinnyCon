/**
 * @file      ui_schedule.cpp
 * @brief     SkinnyCon 2026 Schedule — May 12-14, Huntsville AL
 * @details   Browse the 3-day conference schedule. Scrollable list with
 *            time, title, speaker, and room info. Includes back button.
 * @license   MIT
 */

#include "lvgl.h"
#include <stdio.h>
#include <string.h>

#ifdef NATIVE_BUILD
#include <ctime>
#include <cstddef>
#else
#include "ui_define.h"
#endif

/* Schedule data - SkinnyCon 2026 */
typedef struct {
    const char *day;
    const char *date;
    const char *time;
    const char *title;
    const char *speaker;
    const char *room;
    const char *type;
} schedule_event_t;

static const schedule_event_t schedule_data[] = {
    // Day 1 - Tuesday, May 12
    {"Day 1", "Tue, May 12", "08:00-09:00", "Registration & Breakfast", "Staff", "Main Hall", "General"},
    {"Day 1", "Tue, May 12", "09:00-09:30", "Opening Keynote: The Future of LoRa", "Dr. Sarah Chen", "Main Stage", "Keynote"},
    {"Day 1", "Tue, May 12", "09:45-10:30", "LoRaWAN 2.0: What's New", "Mike Johnson", "Room A", "Technical"},
    {"Day 1", "Tue, May 12", "10:45-11:30", "Coffee Break & Networking", "All", "Main Hall", "Social"},
    {"Day 1", "Tue, May 12", "11:45-12:30", "Industrial IoT with LoRa", "TechCorp Team", "Room B", "Case Study"},
    {"Day 1", "Tue, May 12", "12:30-13:30", "Lunch Break", "All", "Main Hall", "Break"},
    {"Day 1", "Tue, May 12", "13:30-14:15", "Smart City Applications", "CityTech Inc", "Room A", "Case Study"},
    {"Day 1", "Tue, May 12", "14:30-15:15", "Security in LoRa Networks", "CyberSec Pro", "Room B", "Technical"},
    {"Day 1", "Tue, May 12", "15:30-16:15", "Panel: LoRa Ecosystem Growth", "Industry Leaders", "Main Stage", "Panel"},
    {"Day 1", "Tue, May 12", "16:30-17:00", "Closing Remarks", "Organizers", "Main Stage", "General"},
    
    // Day 2 - Wednesday, May 13
    {"Day 2", "Wed, May 13", "09:00-09:45", "Workshop: Building LoRa Devices", "Hardware Lab", "Room A", "Workshop"},
    {"Day 2", "Wed, May 13", "10:00-10:45", "Workshop: LoRaWAN Network Setup", "Network Team", "Room B", "Workshop"},
    {"Day 2", "Wed, May 13", "11:00-11:45", "Agricultural IoT Solutions", "AgriTech Solutions", "Main Stage", "Case Study"},
    {"Day 2", "Wed, May 13", "12:00-13:00", "Lunch & Demo Booths", "All", "Main Hall", "Social"},
    {"Day 2", "Wed, May 13", "13:00-13:45", "Energy Harvesting with LoRa", "PowerTech", "Room A", "Technical"},
    {"Day 2", "Wed, May 13", "14:00-14:45", "Asset Tracking Best Practices", "TrackIt Corp", "Room B", "Case Study"},
    {"Day 2", "Wed, May 13", "15:00-15:45", "Developer Showcase", "Community", "Main Stage", "Demo"},
    {"Day 2", "Wed, May 13", "16:00-17:00", "Social Hour & Networking", "All", "Rooftop Terrace", "Social"},
    
    // Day 3 - Thursday, May 14
    {"Day 3", "Thu, May 14", "09:00-09:45", "Future of LPWAN Technologies", "Research Team", "Main Stage", "Keynote"},
    {"Day 3", "Thu, May 14", "10:00-10:45", "LoRa for Healthcare", "MedTech Innovations", "Room A", "Case Study"},
    {"Day 3", "Thu, May 14", "11:00-11:45", "Supply Chain Optimization", "LogiChain Pro", "Room B", "Technical"},
    {"Day 3", "Thu, May 14", "12:00-13:00", "Farewell Lunch", "All", "Main Hall", "Break"},
    {"Day 3", "Thu, May 14", "13:00-14:00", "Closing Ceremony & Awards", "Organizers", "Main Stage", "General"},
    {"Day 3", "Thu, May 14", "14:00-15:00", "Conference Adjourns", "All", "Main Hall", "General"},
};

#define SCHED_ROWS (sizeof(schedule_data) / sizeof(schedule_data[0]))

static lv_obj_t *schedule_list = NULL;
static lv_obj_t *day_label = NULL;
static lv_obj_t *date_label = NULL;
static lv_obj_t *time_label = NULL;
static lv_obj_t *title_label = NULL;
static lv_obj_t *speaker_label = NULL;
static lv_obj_t *room_label = NULL;
static lv_obj_t *type_label = NULL;
static int current_row = 0;

static const lv_color_t SCHED_ACCENT = {0x21, 0x96, 0xF3};
static const lv_color_t SCHED_BG = {0xFA, 0xFA, 0xFA};
static const lv_color_t SCHED_TEXT = {0x33, 0x33, 0x33};
static const lv_color_t SCHED_TYPE_KEYNOTE = {0x9C, 0x27, 0xB0};
static const lv_color_t SCHED_TYPE_TECHNICAL = {0x00, 0x96, 0x88};
static const lv_color_t SCHED_TYPE_CASE = {0xFF, 0x98, 0x00};
static const lv_color_t SCHED_TYPE_SOCIAL = {0x7B, 0x1FA8};
static const lv_color_t SCHED_TYPE_WORKSHOP = {0xE9, 0x1E, 0x63};
static const lv_color_t SCHED_TYPE_GENERAL = {0x60, 0x7D, 0x8B};

static void schedule_update_display(void) {
    if (current_row >= SCHED_ROWS) return;
    
    const schedule_event_t *evt = &schedule_data[current_row];
    
    lv_label_set_text(day_label, evt->day);
    lv_label_set_text(date_label, evt->date);
    lv_label_set_text(time_label, evt->time);
    lv_label_set_text(title_label, evt->title);
    lv_label_set_text(speaker_label, evt->speaker);
    lv_label_set_text(room_label, evt->room);
    lv_label_set_text(type_label, evt->type);
    
    /* Set type color */
    lv_color_t type_color = SCHED_TYPE_GENERAL;
    if (strcmp(evt->type, "Keynote") == 0) type_color = SCHED_TYPE_KEYNOTE;
    else if (strcmp(evt->type, "Technical") == 0) type_color = SCHED_TYPE_TECHNICAL;
    else if (strcmp(evt->type, "Case Study") == 0) type_color = SCHED_TYPE_CASE;
    else if (strcmp(evt->type, "Social") == 0) type_color = SCHED_TYPE_SOCIAL;
    else if (strcmp(evt->type, "Workshop") == 0) type_color = SCHED_TYPE_WORKSHOP;
    
    lv_obj_set_style_text_color(type_label, type_color, 0);
}

static void schedule_create_row(lv_obj_t *parent, int idx) {
    if (idx >= SCHED_ROWS) return;
    
    const schedule_event_t *evt = &schedule_data[idx];
    
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_set_size(row, LV_PCT(100), 140);
    lv_obj_set_style_bg_color(row, lv_color_white(), 0);
    lv_obj_set_style_radius(row, 12, 0);
    lv_obj_set_style_border_width(row, 1, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_color(row, lv_color_hex(0xE0E0E0), LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_shadow_width(row, 8, 0);
    lv_obj_set_style_shadow_opa(row, LV_OPA_20, 0);
    lv_obj_set_style_shadow_offset_y(row, 2, 0);
    lv_obj_add_flag(row, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_style_pad_all(row, 16, 0);
    
    /* Day label */
    lv_obj_t *day_lbl = lv_label_create(row);
    lv_label_set_text(day_lbl, evt->day);
    lv_obj_set_style_text_font(day_lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(day_lbl, SCHED_ACCENT, 0);
    lv_obj_align(day_lbl, LV_ALIGN_TOP_LEFT, 12, 8);
    
    /* Date label */
    lv_obj_t *date_lbl = lv_label_create(row);
    lv_label_set_text(date_lbl, evt->date);
    lv_obj_set_style_text_font(date_lbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(date_lbl, lv_color_hex(0x757575), 0);
    lv_obj_align_to(date_lbl, day_lbl, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    
    /* Time label */
    lv_obj_t *time_lbl = lv_label_create(row);
    lv_label_set_text(time_lbl, evt->time);
    lv_obj_set_style_text_font(time_lbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(time_lbl, lv_color_hex(0x757575), 0);
    lv_obj_align_to(time_lbl, date_lbl, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    
    /* Title label */
    lv_obj_t *title_lbl = lv_label_create(row);
    lv_label_set_text(title_lbl, evt->title);
    lv_obj_set_style_text_font(title_lbl, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(title_lbl, lv_color_black(), 0);
    lv_obj_align_to(title_lbl, day_lbl, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_obj_set_width(title_lbl, LV_PCT(85));
    lv_label_set_long_mode(title_lbl, LV_LABEL_LONG_SCROLL_CIRCULAR);
    
    /* Speaker label */
    lv_obj_t *speaker_lbl = lv_label_create(row);
    lv_label_set_text(speaker_lbl, evt->speaker);
    lv_obj_set_style_text_font(speaker_lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(speaker_lbl, lv_color_hex(0x555555), 0);
    lv_obj_align_to(speaker_lbl, title_lbl, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 6);
    
    /* Room label */
    lv_obj_t *room_lbl = lv_label_create(row);
    lv_label_set_text(room_lbl, evt->room);
    lv_obj_set_style_text_font(room_lbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(room_lbl, lv_color_hex(0x757575), 0);
    lv_obj_align_to(room_lbl, speaker_lbl, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    
    /* Type badge */
    lv_obj_t *type_box = lv_obj_create(row);
    lv_obj_set_size(type_box, 80, 28);
    lv_obj_set_style_bg_color(type_box, type_color, 0);
    lv_obj_set_style_bg_opa(type_box, LV_OPA_80, 0);
    lv_obj_set_style_radius(type_box, 14, 0);
    lv_obj_align(type_box, LV_ALIGN_BOTTOM_RIGHT, -12, -12);
    
    lv_obj_t *type_lbl2 = lv_label_create(type_box);
    lv_label_set_text(type_lbl2, evt->type);
    lv_obj_set_style_text_font(type_lbl2, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(type_lbl2, lv_color_white(), 0);
    lv_obj_center(type_lbl2);
}

static void schedule_list_create(lv_obj_t *parent) {
    schedule_list = lv_list_create(parent);
    lv_obj_set_size(schedule_list, LV_PCT(100), LV_PCT(100));
    lv_obj_align(schedule_list, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(schedule_list, lv_color_white(), 0);
    lv_obj_set_style_radius(schedule_list, 0, 0);
    lv_obj_set_style_pad_all(schedule_list, 0, 0);
    lv_obj_set_style_border_width(schedule_list, 0, 0);
    
    /* Create all rows */
    for (int i = 0; i < SCHED_ROWS; i++) {
        schedule_create_row(schedule_list, i);
    }
    
    /* Set initial row */
    current_row = 0;
    schedule_update_display();
}

static void schedule_back_button(lv_obj_t *parent) {
    lv_obj_t *back_btn = lv_btn_create(parent);
    lv_obj_set_size(back_btn, 60, 40);
    lv_obj_align(back_btn, LV_ALIGN_TOP_LEFT, 12, 12);
    lv_obj_set_style_bg_color(back_btn, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_100, 0);
    lv_obj_set_style_radius(back_btn, 8, 0);
    lv_obj_set_style_border_width(back_btn, 1, 0);
    lv_obj_set_style_border_color(back_btn, SCHED_ACCENT, 0);
    lv_obj_set_style_shadow_width(back_btn, 4, 0);
    lv_obj_set_style_shadow_opa(back_btn, LV_OPA_20, 0);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    
    lv_obj_t *back_icon = lv_img_create(back_btn);
    lv_img_set_src(back_icon, &lv_icon_back_20px);
    lv_obj_align(back_icon, LV_ALIGN_LEFT_MID, 8, 0);
    
    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "Back");
    lv_obj_set_style_text_font(back_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(back_label, SCHED_ACCENT, 0);
    lv_obj_align_to(back_label, back_icon, LV_ALIGN_OUT_RIGHT_MID, 6, 0);
    
    lv_obj_add_event_cb(back_btn, [](lv_event_t *e) {
        lv_obj_t *btn = lv_event_get_target(e);
        if (btn) {
            lv_obj_del(btn);
            lv_obj_del(schedule_list);
            lv_obj_clean(lv_scr_act());
            
            /* Return to main menu */
            extern void ui_main_menu_create(lv_obj_t *parent);
            ui_main_menu_create(lv_scr_act());
        }
    }, LV_EVENT_CLICKED, NULL);
}

static void ui_schedule_enter(lv_obj_t *parent) {
    /* Create back button */
    schedule_back_button(parent);
    
    /* Create schedule list */
    schedule_list_create(parent);
    
    /* Create info label */
    lv_obj_t *info_lbl = lv_label_create(parent);
    lv_label_set_text(info_lbl, "Tap any event to view details");
    lv_obj_set_style_text_font(info_lbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(info_lbl, lv_color_hex(0x757575), 0);
    lv_obj_align(info_lbl, LV_ALIGN_BOTTOM_MID, 0, -12);
}

static void ui_schedule_exit(lv_obj_t *parent) {
    if (schedule_list) {
        lv_obj_del(schedule_list);
        schedule_list = NULL;
    }
    current_row = 0;
}

app_t ui_schedule_main = {
    .setup_func_cb = ui_schedule_enter,
    .exit_func_cb = ui_schedule_exit,
    .user_data = nullptr,
};