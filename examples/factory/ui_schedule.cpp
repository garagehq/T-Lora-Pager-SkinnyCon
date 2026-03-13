/**
 * @file      ui_schedule.cpp
 * @brief     SkinnyCon 2026 Schedule — May 12-14, Huntsville AL
 * @details   Browse the 3-day conference schedule. Scrollable list with
 *            time, title, and speaker info for each talk.
 */

#include "ui_define.h"
#include "lvgl.h"
#include "ui_icons_v9.h"
#include <stdio.h>

static lv_obj_t *sched_cont = NULL;
static lv_obj_t *sched_list = NULL;
static lv_obj_t *day_label = NULL;
static int current_day = 0;

// Schedule data for SkinnyCon 2026 (May 12-14, Huntsville AL)
typedef struct {
    const char *time;
    const char *title;
    const char *speaker;
    const char *room;
} talk_t;

static const talk_t day0[] = {
    {"08:00", "Doors Open", "All Attendees", "Main Hall"},
    {"09:00", "Opening Keynote: The Future of LoRa", "Dr. Alice Chen", "Main Stage"},
    {"10:00", "LoRaWAN 2.0: What's New?", "Bob Martinez", "Room A"},
    {"11:00", "Building Long-Range IoT Networks", "Carol Davis", "Room B"},
    {"12:00", "Lunch Break", "All Attendees", "Cafeteria"},
    {"13:00", "LoRa in Agriculture", "David Kim", "Room A"},
    {"14:00", "Smart City Applications", "Eva Johnson", "Room B"},
    {"15:00", "Afternoon Break", "All Attendees", "Main Hall"},
    {"15:30", "Panel: LoRa vs WiFi vs BLE", "Moderator: Frank Lee", "Main Stage"},
    {"16:30", "Closing Remarks", "Organizing Committee", "Main Stage"},
};

static const talk_t day1[] = {
    {"09:00", "Day 2 Opening", "All Attendees", "Main Stage"},
    {"09:30", "Workshop: LoRa Network Design", "Dr. Alice Chen", "Room A"},
    {"11:00", "Workshop: LoRa Network Design (cont.)", "Dr. Alice Chen", "Room A"},
    {"12:00", "Lunch Break", "All Attendees", "Cafeteria"},
    {"13:00", "Advanced LoRa Techniques", "George Wang", "Room B"},
    {"14:00", "Security in LoRaWAN", "Helen Brown", "Room A"},
    {"15:00", "Afternoon Break", "All Attendees", "Main Hall"},
    {"15:30", "Demo Session: LoRa Projects", "Various Speakers", "Main Stage"},
    {"16:30", "Day 2 Closing", "All Attendees", "Main Stage"},
};

static const talk_t day2[] = {
    {"09:00", "Day 3 Opening", "All Attendees", "Main Stage"},
    {"09:30", "LoRa for Industrial IoT", "Ivan Petrov", "Room A"},
    {"10:30", "Low Power Design Strategies", "Julia Smith", "Room B"},
    {"11:30", "Coffee Break", "All Attendees", "Main Hall"},
    {"12:00", "Lunch Break", "All Attendees", "Cafeteria"},
    {"13:00", "Final Project Presentations", "Various Speakers", "Main Stage"},
    {"15:00", "Conference Wrap-up", "Organizing Committee", "Main Stage"},
    {"15:30", "Farewell Reception", "All Attendees", "Garden Patio"},
};

static const talk_t *days[] = {day0, day1, day2};
static const int day_counts[] = {10, 8, 8};

static void update_schedule_day(void)
{
    if (!day_label || !sched_list) return;
    
    const char *day_names[] = {"Tuesday, May 12", "Wednesday, May 13", "Thursday, May 14"};
    lv_label_set_text(day_label, day_names[current_day]);
    
    lv_obj_clean(sched_list);
    
    const talk_t *talks = days[current_day];
    const int n = day_counts[current_day];
    
    for (int i = 0; i < n; i++) {
        lv_obj_t *row = lv_obj_create(sched_list);
        lv_obj_set_size(row, LV_PCT(100), 80);
        lv_obj_set_style_bg_color(row, lv_color_hex(0x2D3142), 0);
        lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_pad_all(row, 10, 0);
        lv_obj_set_style_radius(row, 8, 0);
        
        lv_obj_t *time_label = lv_label_create(row);
        lv_label_set_text(time_label, talks[i].time);
        lv_obj_set_style_text_font(time_label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(time_label, lv_color_hex(0x4DADCC), 0);
        lv_obj_align(time_label, LV_ALIGN_LEFT_MID, 10, 0);
        
        lv_obj_t *title_label = lv_label_create(row);
        lv_label_set_text(title_label, talks[i].title);
        lv_obj_set_style_text_font(title_label, &lv_font_montserrat_18, 0);
        lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
        lv_obj_align(title_label, LV_ALIGN_LEFT_MID, 70, -10);
        
        lv_obj_t *speaker_label = lv_label_create(row);
        lv_label_set_text(speaker_label, talks[i].speaker);
        lv_obj_set_style_text_font(speaker_label, &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_color(speaker_label, lv_color_hex(0xA0A0A0), 0);
        lv_obj_align(speaker_label, LV_ALIGN_LEFT_MID, 70, 10);
        
        lv_obj_t *room_label = lv_label_create(row);
        lv_label_set_text(room_label, talks[i].room);
        lv_obj_set_style_text_font(room_label, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(room_label, lv_color_hex(0x666666), 0);
        lv_obj_align(room_label, LV_ALIGN_RIGHT_MID, -10, 0);
    }
}

static void day_button_cb(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    
    if (btn == lv_obj_get_child(sched_list, 0)) {
        current_day = 0;
    } else if (btn == lv_obj_get_child(sched_list, 1)) {
        current_day = 1;
    } else {
        current_day = 2;
    }
    
    update_schedule_day();
}

static void sched_exit(void)
{
    if (sched_cont) {
        lv_obj_del(sched_cont);
        sched_cont = NULL;
    }
    sched_list = NULL;
    day_label = NULL;
}

static void back_button_cb(lv_event_t *e)
{
    (void)e;
    sched_exit();
    ui_main_show();
}

static void create_back_button(lv_obj_t *parent)
{
    lv_obj_t *back_btn = lv_btn_create(parent);
    lv_obj_set_size(back_btn, 40, 40);
    lv_obj_align(back_btn, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_style_bg_color(back_btn, lv_color_hex(0x21262D), 0);
    lv_obj_set_style_bg_opa(back_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(back_btn, 0, 0);
    lv_obj_set_style_pad_all(back_btn, 0, 0);
    
    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "←");
    lv_obj_center(back_label);
    lv_obj_set_style_text_color(back_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(back_label, &lv_font_montserrat_16, 0);
    
    lv_event_add_click_cb(back_btn, back_button_cb);
}

static void sched_setup(void)
{
    sched_cont = lv_obj_create(NULL);
    lv_obj_set_size(sched_cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(sched_cont, lv_color_hex(0x21262D), 0);
    lv_obj_set_style_bg_opa(sched_cont, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(sched_cont, 0, 0);
    lv_obj_set_style_pad_all(sched_cont, 0, 0);
    
    create_back_button(sched_cont);
    
    lv_obj_t *header = lv_obj_create(sched_cont);
    lv_obj_set_size(header, LV_PCT(100), 60);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x21262D), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, 0);
    
    lv_obj_t *title = lv_label_create(header);
    lv_label_set_text(title, "Schedule");
    lv_obj_center(title);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    
    sched_list = lv_obj_create(sched_cont);
    lv_obj_set_size(sched_list, LV_PCT(100), LV_PCT(100) - 60);
    lv_obj_align(sched_list, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(sched_list, lv_color_hex(0x21262D), 0);
    lv_obj_set_style_bg_opa(sched_list, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(sched_list, 0, 0);
    lv_obj_set_style_pad_all(sched_list, 0, 0);
    lv_obj_set_style_pad_hor(sched_list, 10, 0);
    lv_obj_set_style_pad_ver(sched_list, 10, 0);
    lv_obj_set_style_scroll_dir(sched_list, LV_DIR_VER, 0);
    
    day_label = lv_label_create(sched_list);
    lv_obj_set_style_text_font(day_label, &lv_font_montserrat_20, 0);
    lv_label_set_text(day_label, "Loading...");
    
    current_day = 0;
    update_schedule_day();
}

app_t ui_schedule_main = {sched_setup, sched_exit, NULL};
