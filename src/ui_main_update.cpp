#include "ui_main.h"
#include "ui_nametag.h"
#include "ui_clock.h"
#include "lvgl.h"
#include <cstring>

#define DISPLAY_MODE_NAMETAG 0
#define DISPLAY_MODE_CLOCK   1

static int current_display_mode = DISPLAY_MODE_NAMETAG;
static lv_obj_t *settings_label = nullptr;
static lv_obj_t *mode_button = nullptr;

static void on_mode_button_click(lv_event_t *e) {
    (void)e;
    if (current_display_mode == DISPLAY_MODE_NAMETAG) {
        current_display_mode = DISPLAY_MODE_CLOCK;
        lv_label_set_text(settings_label, "Switch to Clock");
        ui_clock_init();
    } else {
        current_display_mode = DISPLAY_MODE_NAMETAG;
        lv_label_set_text(settings_label, "Switch to Nametag");
        ui_nametag_init();
    }
}

void ui_main_init(void) {
    lv_obj_t *screen = lv_scr_act();
    
    settings_label = lv_label_create(screen);
    lv_label_set_text(settings_label, "Switch to Clock");
    lv_obj_set_style_text_color(settings_label, lv_color_hex(0xaaaaaa), 0);
    lv_obj_set_style_text_font(settings_label, &lv_font_montserrat_16, 0);
    lv_obj_align_to(settings_label, screen, LV_ALIGN_BOTTOM_MID, 0, -20);
    
    mode_button = lv_btn_create(screen);
    lv_obj_align_to(mode_button, settings_label, LV_ALIGN_OUT_LEFT_MID, -10, 0);
    lv_obj_set_size(mode_button, 120, 40);
    
    lv_obj_t *btn_label = lv_label_create(mode_button);
    lv_label_set_text(btn_label, "Change Mode");
    lv_obj_center(btn_label);
    
    lv_event_register(mode_button, LV_EVENT_CLICKED, on_mode_button_click, nullptr);
    
    ui_nametag_init();
}

void ui_main_update(void) {
    if (current_display_mode == DISPLAY_MODE_NAMETAG) {
        ui_nametag_update();
    } else {
        ui_clock_update();
    }
}
