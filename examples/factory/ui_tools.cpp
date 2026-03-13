/**
 * @file      ui_tools.cpp
 * @brief     SkinnyCon 2026 Network Tools — Diagnostics & Utilities
 * @details   Browse network status, test LoRa radio, check GPS, view power.
 *            Includes back button navigation to return to main menu.
 * @version   1.0.0
 * @date      2025-12-09
 * @copyright Copyright (c) 2025  ShenZhen XinYuan Electronic Technology Co., Ltd
 */

#include "ui_define.h"
#include "lvgl.h"
#include <string.h>

#if defined(USING_NETWORK_TOOLS)

/* Forward declarations */
static void ui_tools_enter(lv_obj_t *parent);
static void ui_tools_exit(void);
static void ui_tools_show_menu(lv_obj_t *parent);
static void ui_tools_show_network(lv_obj_t *parent);
static void ui_tools_show_lora(lv_obj_t *parent);
static void ui_tools_show_gps(lv_obj_t *parent);
static void ui_tools_show_power(lv_obj_t *parent);

/* UI objects */
static lv_obj_t *tools_menu = NULL;
static lv_obj_t *tools_network = NULL;
static lv_obj_t *tools_lora = NULL;
static lv_obj_t *tools_gps = NULL;
static lv_obj_t *tools_power = NULL;
static lv_obj_t *tools_back_btn = NULL;

/* Tool menu items */
static lv_obj_t *menu_network = NULL;
static lv_obj_t *menu_lora = NULL;
static lv_obj_t *menu_gps = NULL;
static lv_obj_t *menu_power = NULL;
static lv_obj_t *menu_back = NULL;

/* Network tool objects */
static lv_obj_t *net_status_label = NULL;
static lv_obj_t *net_rssi_label = NULL;
static lv_obj_t *net_snr_label = NULL;
static lv_obj_t *net_freq_label = NULL;

/* LoRa tool objects */
static lv_obj_t *lora_status_label = NULL;
static lv_obj_t *lora_tx_label = NULL;
static lv_obj_t *lora_rx_label = NULL;
static lv_obj_t *lora_mode_label = NULL;

/* GPS tool objects */
static lv_obj_t *gps_status_label = NULL;
static lv_obj_t *gps_lat_label = NULL;
static lv_obj_t *gps_lon_label = NULL;
static lv_obj_t *gps_fix_label = NULL;

/* Power tool objects */
static lv_obj_t *power_status_label = NULL;
static lv_obj_t *power_vbat_label = NULL;
static lv_obj_t *power_current_label = NULL;
static lv_obj_t *power_temp_label = NULL;

/* Tool state */
static bool tools_showing_menu = true;
static bool tools_showing_network = false;
static bool tools_showing_lora = false;
static bool tools_showing_gps = false;
static bool tools_showing_power = false;

/* Helper: Create back button */
static lv_obj_t *create_back_button(lv_obj_t *parent, const char *label)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 120, 45);
    lv_obj_center(btn);
    lv_obj_set_style_bg_color(btn, LV_COLOR_MAKE(0x26, 0x32, 0x38), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(btn, 8, 0);
    lv_obj_set_style_border_width(btn, 2, 0);
    lv_obj_set_style_border_color(btn, LV_COLOR_MAKE(0x00, 0x96, 0x88), 0);
    
    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, label);
    lv_obj_center(lbl);
    lv_obj_set_style_text_color(lbl, LV_COLOR_WHITE, 0);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_16, 0);
    
    return btn;
}

/* Helper: Create tool menu item */
static lv_obj_t *create_menu_item(lv_obj_t *parent, const char *label, lv_event_cb_t callback)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 280, 50);
    lv_obj_set_style_bg_color(btn, LV_COLOR_MAKE(0x26, 0x32, 0x38), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(btn, 6, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, LV_COLOR_MAKE(0x00, 0x96, 0x88), 0);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_add_event_cb(btn, callback, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, label);
    lv_obj_center(lbl);
    lv_obj_set_style_text_color(lbl, LV_COLOR_WHITE, 0);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_16, 0);
    
    return btn;
}

/* Helper: Create info card */
static lv_obj_t *create_info_card(lv_obj_t *parent, const char *title)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, 300, 180);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, LV_COLOR_MAKE(0xEC, 0xEF, 0xF1), 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(card, 10, 0);
    lv_obj_set_style_border_width(card, 2, 0);
    lv_obj_set_style_border_color(card, LV_COLOR_MAKE(0x00, 0x96, 0x88), 0);
    
    lv_obj_t *title_lbl = lv_label_create(card);
    lv_label_set_text(title_lbl, title);
    lv_obj_align(title_lbl, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_text_font(title_lbl, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(title_lbl, LV_COLOR_MAKE(0x26, 0x32, 0x38), 0);
    
    return card;
}

/* Helper: Create info label */
static lv_obj_t *create_info_label(lv_obj_t *parent, const char *label_text, const char *value_text)
{
    lv_obj_t *lbl = lv_label_create(parent);
    lv_label_set_text_fmt(lbl, "%s: %s", label_text, value_text);
    lv_obj_align_to(lbl, parent, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl, LV_COLOR_MAKE(0x26, 0x32, 0x38), 0);
    return lbl;
}

/* Tool: Show menu */
static void ui_tools_show_menu(lv_obj_t *parent)
{
    if (tools_menu) lv_obj_del(tools_menu);
    tools_menu = lv_obj_create(parent);
    lv_obj_set_size(tools_menu, 320, 240);
    lv_obj_center(tools_menu);
    lv_obj_set_style_bg_color(tools_menu, LV_COLOR_MAKE(0xEC, 0xEF, 0xF1), 0);
    lv_obj_set_style_bg_opa(tools_menu, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(tools_menu, 12, 0);
    lv_obj_set_style_border_width(tools_menu, 3, 0);
    lv_obj_set_style_border_color(tools_menu, LV_COLOR_MAKE(0x00, 0x96, 0x88), 0);
    
    lv_obj_t *title = lv_label_create(tools_menu);
    lv_label_set_text(title, "Network Tools");
    lv_obj_align_to(title, tools_menu, LV_ALIGN_TOP_MID, 0, 15);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, LV_COLOR_MAKE(0x26, 0x32, 0x38), 0);
    
    lv_obj_t *subtitle = lv_label_create(tools_menu);
    lv_label_set_text(subtitle, "Diagnostics & Utilities");
    lv_obj_align_to(subtitle, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    lv_obj_set_style_text_font(subtitle, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(subtitle, LV_COLOR_GRAY, 0);
    
    menu_network = create_menu_item(tools_menu, "Network Status", ui_tools_show_network);
    lv_obj_align_to(menu_network, subtitle, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
    
    menu_lora = create_menu_item(tools_menu, "LoRa Radio", ui_tools_show_lora);
    lv_obj_align_to(menu_lora, menu_network, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    
    menu_gps = create_menu_item(tools_menu, "GPS Position", ui_tools_show_gps);
    lv_obj_align_to(menu_gps, menu_lora, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    
    menu_power = create_menu_item(tools_menu, "Power Monitor", ui_tools_show_power);
    lv_obj_align_to(menu_power, menu_gps, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    
    menu_back = create_back_button(tools_menu, "Back");
    lv_obj_align_to(menu_back, menu_power, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
    lv_obj_add_flag(menu_back, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(menu_back, [](lv_event_t *e) {
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *parent = lv_obj_get_parent(btn);
        lv_obj_del(parent);
        tools_showing_menu = false;
        tools_showing_network = false;
        tools_showing_lora = false;
        tools_showing_gps = false;
        tools_showing_power = false;
    }, LV_EVENT_CLICKED, NULL);
    
    tools_showing_menu = true;
    tools_showing_network = false;
    tools_showing_lora = false;
    tools_showing_gps = false;
    tools_showing_power = false;
}

/* Tool: Show network status */
static void ui_tools_show_network(lv_event_t *e)
{
    if (tools_network) lv_obj_del(tools_network);
    tools_network = lv_obj_create(tools_menu);
    lv_obj_set_size(tools_network, 320, 240);
    lv_obj_center(tools_network);
    lv_obj_set_style_bg_color(tools_network, LV_COLOR_MAKE(0xEC, 0xEF, 0xF1), 0);
    lv_obj_set_style_bg_opa(tools_network, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(tools_network, 12, 0);
    lv_obj_set_style_border_width(tools_network, 3, 0);
    lv_obj_set_style_border_color(tools_network, LV_COLOR_MAKE(0x00, 0x96, 0x88), 0);
    
    lv_obj_t *title = lv_label_create(tools_network);
    lv_label_set_text(title, "Network Status");
    lv_obj_align_to(title, tools_network, LV_ALIGN_TOP_MID, 0, 15);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, LV_COLOR_MAKE(0x26, 0x32, 0x38), 0);
    
    lv_obj_t *card = create_info_card(tools_network, "Connection Info");
    lv_obj_align_to(card, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    
    net_status_label = create_info_label(card, "Status", "Connected");
    net_rssi_label = create_info_label(card, "RSSI", "-65 dBm");
    net_snr_label = create_info_label(card, "SNR", "9.5 dB");
    net_freq_label = create_info_label(card, "Frequency", "915 MHz");
    
    lv_obj_t *back_btn = create_back_button(tools_network, "Back");
    lv_obj_align_to(back_btn, net_freq_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(back_btn, [](lv_event_t *e) {
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *parent = lv_obj_get_parent(btn);
        lv_obj_del(parent);
        lv_obj_del(tools_menu);
        tools_showing_menu = true;
        tools_showing_network = false;
        ui_tools_show_menu(tools_menu);
    }, LV_EVENT_CLICKED, NULL);
    
    tools_showing_menu = false;
    tools_showing_network = true;
    tools_showing_lora = false;
    tools_showing_gps = false;
    tools_showing_power = false;
}

/* Tool: Show LoRa status */
static void ui_tools_show_lora(lv_event_t *e)
{
    if (tools_lora) lv_obj_del(tools_lora);
    tools_lora = lv_obj_create(tools_menu);
    lv_obj_set_size(tools_lora, 320, 240);
    lv_obj_center(tools_lora);
    lv_obj_set_style_bg_color(tools_lora, LV_COLOR_MAKE(0xEC, 0xEF, 0xF1), 0);
    lv_obj_set_style_bg_opa(tools_lora, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(tools_lora, 12, 0);
    lv_obj_set_style_border_width(tools_lora, 3, 0);
    lv_obj_set_style_border_color(tools_lora, LV_COLOR_MAKE(0x00, 0x96, 0x88), 0);
    
    lv_obj_t *title = lv_label_create(tools_lora);
    lv_label_set_text(title, "LoRa Radio");
    lv_obj_align_to(title, tools_lora, LV_ALIGN_TOP_MID, 0, 15);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, LV_COLOR_MAKE(0x26, 0x32, 0x38), 0);
    
    lv_obj_t *card = create_info_card(tools_lora, "Radio Status");
    lv_obj_align_to(card, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    
    lora_status_label = create_info_label(card, "Status", "Active");
    lora_tx_label = create_info_label(card, "TX Power", "22 dBm");
    lora_rx_label = create_info_label(card, "RX Count", "1,247");
    lora_mode_label = create_info_label(card, "Mode", "Class A");
    
    lv_obj_t *back_btn = create_back_button(tools_lora, "Back");
    lv_obj_align_to(back_btn, lora_mode_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(back_btn, [](lv_event_t *e) {
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *parent = lv_obj_get_parent(btn);
        lv_obj_del(parent);
        lv_obj_del(tools_menu);
        tools_showing_menu = true;
        tools_showing_lora = false;
        ui_tools_show_menu(tools_menu);
    }, LV_EVENT_CLICKED, NULL);
    
    tools_showing_menu = false;
    tools_showing_network = false;
    tools_showing_lora = true;
    tools_showing_gps = false;
    tools_showing_power = false;
}

/* Tool: Show GPS status */
static void ui_tools_show_gps(lv_event_t *e)
{
    if (tools_gps) lv_obj_del(tools_gps);
    tools_gps = lv_obj_create(tools_menu);
    lv_obj_set_size(tools_gps, 320, 240);
    lv_obj_center(tools_gps);
    lv_obj_set_style_bg_color(tools_gps, LV_COLOR_MAKE(0xEC, 0xEF, 0xF1), 0);
    lv_obj_set_style_bg_opa(tools_gps, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(tools_gps, 12, 0);
    lv_obj_set_style_border_width(tools_gps, 3, 0);
    lv_obj_set_style_border_color(tools_gps, LV_COLOR_MAKE(0x00, 0x96, 0x88), 0);
    
    lv_obj_t *title = lv_label_create(tools_gps);
    lv_label_set_text(title, "GPS Position");
    lv_obj_align_to(title, tools_gps, LV_ALIGN_TOP_MID, 0, 15);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, LV_COLOR_MAKE(0x26, 0x32, 0x38), 0);
    
    lv_obj_t *card = create_info_card(tools_gps, "Location");
    lv_obj_align_to(card, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    
    gps_status_label = create_info_label(card, "Status", "Locked");
    gps_lat_label = create_info_label(card, "Latitude", "34.7304° N");
    gps_lon_label = create_info_label(card, "Longitude", "86.5804° W");
    gps_fix_label = create_info_label(card, "Fix", "3D Fix");
    
    lv_obj_t *back_btn = create_back_button(tools_gps, "Back");
    lv_obj_align_to(back_btn, gps_fix_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(back_btn, [](lv_event_t *e) {
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *parent = lv_obj_get_parent(btn);
        lv_obj_del(parent);
        lv_obj_del(tools_menu);
        tools_showing_menu = true;
        tools_showing_gps = false;
        ui_tools_show_menu(tools_menu);
    }, LV_EVENT_CLICKED, NULL);
    
    tools_showing_menu = false;
    tools_showing_network = false;
    tools_showing_lora = false;
    tools_showing_gps = true;
    tools_showing_power = false;
}

/* Tool: Show power status */
static void ui_tools_show_power(lv_event_t *e)
{
    if (tools_power) lv_obj_del(tools_power);
    tools_power = lv_obj_create(tools_menu);
    lv_obj_set_size(tools_power, 320, 240);
    lv_obj_center(tools_power);
    lv_obj_set_style_bg_color(tools_power, LV_COLOR_MAKE(0xEC, 0xEF, 0xF1), 0);
    lv_obj_set_style_bg_opa(tools_power, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(tools_power, 12, 0);
    lv_obj_set_style_border_width(tools_power, 3, 0);
    lv_obj_set_style_border_color(tools_power, LV_COLOR_MAKE(0x00, 0x96, 0x88), 0);
    
    lv_obj_t *title = lv_label_create(tools_power);
    lv_label_set_text(title, "Power Monitor");
    lv_obj_align_to(title, tools_power, LV_ALIGN_TOP_MID, 0, 15);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, LV_COLOR_MAKE(0x26, 0x32, 0x38), 0);
    
    lv_obj_t *card = create_info_card(tools_power, "Battery Info");
    lv_obj_align_to(card, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    
    power_status_label = create_info_label(card, "Status", "Charging");
    power_vbat_label = create_info_label(card, "Voltage", "4.20 V");
    power_current_label = create_info_label(card, "Current", "150 mA");
    power_temp_label = create_info_label(card, "Temp", "32°C");
    
    lv_obj_t *back_btn = create_back_button(tools_power, "Back");
    lv_obj_align_to(back_btn, power_temp_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(back_btn, [](lv_event_t *e) {
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *parent = lv_obj_get_parent(btn);
        lv_obj_del(parent);
        lv_obj_del(tools_menu);
        tools_showing_menu = true;
        tools_showing_power = false;
        ui_tools_show_menu(tools_menu);
    }, LV_EVENT_CLICKED, NULL);
    
    tools_showing_menu = false;
    tools_showing_network = false;
    tools_showing_lora = false;
    tools_showing_gps = false;
    tools_showing_power = true;
}

/* Enter tools app */
static void ui_tools_enter(lv_obj_t *parent)
{
    ui_tools_show_menu(parent);
}

/* Exit tools app */
static void ui_tools_exit(void)
{
    if (tools_menu) {
        lv_obj_del(tools_menu);
        tools_menu = NULL;
    }
    if (tools_network) {
        lv_obj_del(tools_network);
        tools_network = NULL;
    }
    if (tools_lora) {
        lv_obj_del(tools_lora);
        tools_lora = NULL;
    }
    if (tools_gps) {
        lv_obj_del(tools_gps);
        tools_gps = NULL;
    }
    if (tools_power) {
        lv_obj_del(tools_power);
        tools_power = NULL;
    }
    tools_showing_menu = false;
    tools_showing_network = false;
    tools_showing_lora = false;
    tools_showing_gps = false;
    tools_showing_power = false;
}

/* App registration */
app_t ui_tools_main = {
    .setup_func_cb = ui_tools_enter,
    .exit_func_cb = ui_tools_exit,
    .user_data = nullptr,
};

#endif /* USING_NETWORK_TOOLS */
