#include "ui_define.h"
#include "lvgl.h"
#include <string.h>

static lv_obj_t *tools_list;
static lv_obj_t *tools_back_btn;
static bool tools_back_pressed = false;

static void tools_back_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        tools_back_pressed = true;
    }
}

static void tools_create_back_button(void) {
    tools_back_btn = lv_btn_create(lv_scr_act());
    lv_obj_set_pos(tools_back_btn, 10, 10);
    lv_obj_set_size(tools_back_btn, 100, 30);
    lv_obj_add_flag(tools_back_btn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_add_event_cb(tools_back_btn, tools_back_event_cb, LV_EVENT_ALL, NULL);
    
    lv_obj_t *label = lv_label_create(tools_back_btn);
    lv_label_set_text(label, "Back");
    lv_obj_center(label);
}

static void tools_setup(void) {
    tools_create_back_button();
    tools_list = lv_list_create(lv_scr_act());
    lv_obj_set_size(tools_list, LV_PCT(100), LV_PCT(80));
    lv_obj_align(tools_list, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(tools_list, lv_color_hex(0x21262D), 0);
    lv_obj_set_style_bg_opa(tools_list, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(tools_list, 0, 0);
    
    lv_obj_t *header = lv_label_create(tools_list);
    lv_label_set_text(header, "Net Tools");
    lv_obj_set_style_text_font(header, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(header, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x21262D), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_size(header, LV_PCT(100), 28);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    
    const char *tools[] = {"Ping", "Traceroute", "DNS Lookup", "Whois", "Network Scan"};
    for (int i = 0; i < 5; i++) {
        lv_obj_t *row = lv_obj_create(tools_list);
        lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(row, lv_color_hex(0x21262D), 0);
        lv_obj_set_style_border_width(row, 0, 0);
        
        lv_obj_t *tool_label = lv_label_create(row);
        lv_label_set_text(tool_label, tools[i]);
        lv_obj_set_style_text_font(tool_label, &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_color(tool_label, lv_color_hex(0xFFFFFF), 0);
        lv_obj_align(tool_label, LV_ALIGN_LEFT_MID, 10, 0);
    }
}

void ui_tools_init(void) {
    lv_scr_load(lv_obj_create(NULL));
    tools_setup();
}

bool ui_tools_update(void) {
    if (tools_back_pressed) {
        tools_back_pressed = false;
        return true;
    }
    return false;
}

void ui_tools_cleanup(void) {
    if (tools_back_btn) {
        lv_obj_del(tools_back_btn);
        tools_back_btn = NULL;
    }
    if (tools_list) {
        lv_obj_del(tools_list);
        tools_list = NULL;
    }
}
