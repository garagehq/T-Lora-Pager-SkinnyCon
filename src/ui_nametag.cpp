#include "ui_nametag.h"
#include "lvgl.h"
#include <cstring>

static lv_obj_t *screen_label = nullptr;
static char nametag_text[64] = "Default Name";

void ui_nametag_init(void) {
    lv_obj_t *screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x1a1a2e), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    
    screen_label = lv_label_create(screen);
    lv_label_set_text(screen_label, nametag_text);
    lv_obj_set_style_text_color(screen_label, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(screen_label, &lv_font_montserrat_48, 0);
    lv_obj_align(screen_label, LV_ALIGN_CENTER, 0, 0);
}

void ui_nametag_set_text(const char *text) {
    if (text && strlen(text) < sizeof(nametag_text)) {
        strncpy(nametag_text, text, sizeof(nametag_text) - 1);
        nametag_text[sizeof(nametag_text) - 1] = '\0';
        if (screen_label) {
            lv_label_set_text(screen_label, nametag_text);
        }
    }
}

void ui_nametag_update(void) {
    if (screen_label) {
        lv_label_set_text(screen_label, nametag_text);
    }
}
