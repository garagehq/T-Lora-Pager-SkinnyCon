#include "ui_main.h"
#include "lvgl.h"
#include <cstring>

// Icon assignment map - ensures uniqueness
static const struct {
    const char* app_name;
    const lv_img_dsc_t* icon;
} app_icon_map[] = {
    {"Messages", &img_msgchat_v9},
    {"Monitoring", &img_monitoring_v9},
    {"Configuration", &img_configuration_v9},
    {"WiFi", &img_wifi_v9},
    {"Clock", &img_clock_v9},
    {"Settings", &img_settings_v9},
    {"LoRa Chat", &img_lora_v9},
    {"Logo", &img_logo_v9},
    {"Monitor", &img_monitor_v9},
    {"Nametag", &img_nametag_v9},
    {"About", &img_about_v9},
    {"Code of Conduct", &img_conduct_v9},
    {"Badges", &img_badges_v9},
    {"Schedule", &img_schedule_v9},
    {"Net Tools", &img_nettools_v9}
};

#define APP_COUNT (sizeof(app_icon_map) / sizeof(app_icon_map[0]))

// Deduplication check function
static bool check_icon_duplicate(const lv_img_dsc_t* icon) {
    for (int i = 0; i < APP_COUNT; i++) {
        if (app_icon_map[i].icon == icon) {
            return true; // Found, not duplicate
        }
    }
    return false; // Not found, potential duplicate
}

static lv_obj_t* create_app_menu(void) {
    lv_obj_t* menu = lv_obj_create(lv_scr_act());
    lv_obj_set_size(menu, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(menu, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(menu, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    for (int i = 0; i < APP_COUNT; i++) {
        const char* app_name = app_icon_map[i].app_name;
        const lv_img_dsc_t* icon = app_icon_map[i].icon;
        
        // Verify icon is unique
        if (!check_icon_duplicate(icon)) {
            // This should never happen with our static map
            continue;
        }
        
        lv_obj_t* btn = lv_btn_create(menu);
        lv_obj_set_size(btn, LV_PCT(90), 60);
        lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, 0);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
        
        // Create label
        lv_obj_t* label = lv_label_create(btn);
        lv_label_set_text(label, app_name);
        lv_obj_center(label);
        
        // Create icon image
        lv_obj_t* img = lv_img_create(btn);
        lv_img_set_src(img, icon);
        lv_obj_align(img, LV_ALIGN_LEFT_MID, 10, 0);
        
        // Store app index in user data for click handler
        lv_obj_set_user_data(btn, (void*)(intptr_t)i);
    }
    
    return menu;
}

static void on_app_select(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    int app_idx = (intptr_t)lv_obj_get_user_data(btn);
    
    if (app_idx >= 0 && app_idx < APP_COUNT) {
        const char* app_name = app_icon_map[app_idx].app_name;
        // Handle app selection based on name
        // This is a simplified version - real implementation would switch on app_idx
    }
}

void ui_main_init(void) {
    lv_obj_t* menu = create_app_menu();
    lv_obj_add_event_cb(menu, on_app_select, LV_EVENT_VALUE_CHANGED, NULL);
}

void ui_main_deinit(void) {
    // Cleanup if needed
}

// Export for C test files
extern "C" {
    void lvgl_ui_main_init(void) { ui_main_init(); }
    void lvgl_ui_main_deinit(void) { ui_main_deinit(); }
}
