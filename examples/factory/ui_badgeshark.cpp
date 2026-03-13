#include "ui_define.h"
#include "lvgl.h"
#include <string.h>

static lv_obj_t *badge_list;
static lv_obj_t *badge_back_btn;
static bool badge_back_pressed = false;

static void badge_back_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        badge_back_pressed = true;
    }
}

static void badge_create_back_button(void) {
    badge_back_btn = lv_btn_create(lv_scr_act());
    lv_obj_set_pos(badge_back_btn, 10, 10);
    lv_obj_set_size(badge_back_btn, 100, 30);
    lv_obj_add_flag(badge_back_btn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_add_event_cb(badge_back_btn, badge_back_event_cb, LV_EVENT_ALL, NULL);
    
    lv_obj_t *label = lv_label_create(badge_back_btn);
    lv_label_set_text(label, "Back");
    lv_obj_center(label);
}

static void badge_setup(void) {
    badge_create_back_button();
    badge_list = lv_list_create(lv_scr_act());
    lv_obj_set_size(badge_list, LV_PCT(100), LV_PCT(80));
    lv_obj_align(badge_list, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(badge_list, lv_color_hex(0x21262D), 0);
    lv_obj_set_style_bg_opa(badge_list, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(badge_list, 0, 0);
    
    lv_obj_t *header = lv_label_create(badge_list);
    lv_label_set_text(header, "BadgeShark");
    lv_obj_set_style_text_font(header, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(header, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x21262D), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_size(header, LV_PCT(100), 28);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    
    const char *badges[] = {"Badge 1", "Badge 2", "Badge 3", "Badge 4", "Badge 5"};
    for (int i = 0; i < 5; i++) {
        lv_obj_t *row = lv_obj_create(badge_list);
        lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(row, lv_color_hex(0x21262D), 0);
        lv_obj_set_style_border_width(row, 0, 0);
        
        lv_obj_t *badge_label = lv_label_create(row);
        lv_label_set_text(badge_label, badges[i]);
        lv_obj_set_style_text_font(badge_label, &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_color(badge_label, lv_color_hex(0xFFFFFF), 0);
        lv_obj_align(badge_label, LV_ALIGN_LEFT_MID, 10, 0);
    }
}

void ui_badgeshark_init(void) {
    lv_scr_load(lv_obj_create(NULL));
    badge_setup();
}

bool ui_badgeshark_update(void) {
    if (badge_back_pressed) {
        badge_back_pressed = false;
        return true;
    }
    return false;
}

void ui_badgeshark_cleanup(void) {
    if (badge_back_btn) {
        lv_obj_del(badge_back_btn);
        badge_back_btn = NULL;
    }
    if (badge_list) {
        lv_obj_del(badge_list);
        badge_list = NULL;
    }
}
