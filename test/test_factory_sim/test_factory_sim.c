/**
 * @file test_factory_sim.c
 * Factory app simulation tests for T-LoRa-Pager (480x222 RGB565).
 *
 * Renders actual factory app screens using real assets (fonts, icons)
 * from examples/factory/src/. Screenshots are saved as PPM files for
 * visual regression testing in CI.
 *
 * This test validates that the factory UI renders correctly without
 * needing ESP32 hardware, using the same LVGL widgets and layouts
 * as the real application.
 */

#include <unity.h>
#include "../simulator/sim_main.c"
#include <string.h>
#include <stdio.h>

/* ---- Display constants ---- */
#define EXPECTED_HOR_RES  480
#define EXPECTED_VER_RES  222

/* ---- Include real factory assets (pure C arrays, no ESP32 deps) ---- */

/* Fonts are compiled as separate .c files in this directory to avoid
 * static name conflicts (glyph_bitmap, font_dsc, etc.).
 * They are: font_12.c, font_24.c, font_40.c, font_100.c
 * We just need the extern declarations here: */
LV_FONT_DECLARE(font_alibaba_12);
LV_FONT_DECLARE(font_alibaba_24);
LV_FONT_DECLARE(font_alibaba_40);
LV_FONT_DECLARE(font_alibaba_100);

/* Icons - v9 variants (RGB565A8, 70x70) */
#include "../../examples/factory/src/img_configuration_v9.c"
#include "../../examples/factory/src/img_radio_v9.c"
#include "../../examples/factory/src/img_msgchat_v9.c"
#include "../../examples/factory/src/img_music_v9.c"
#include "../../examples/factory/src/img_monitoring_v9.c"
#include "../../examples/factory/src/img_keyboard_v9.c"
#include "../../examples/factory/src/img_bluetooth_v9.c"
#include "../../examples/factory/src/img_wifi_v9.c"
#include "../../examples/factory/src/img_test_v9.c"

/* ---- Test setup/teardown ---- */

void setUp(void)
{
    lvgl_sim_init();
}

void tearDown(void)
{
    lvgl_sim_deinit();
}

/* ================================================================
 *  HELPER: Create an app icon button (mirrors factory create_app)
 * ================================================================ */

static void create_app_icon(lv_obj_t *parent, const char *name,
                            const lv_image_dsc_t *img)
{
    lv_obj_t *btn = lv_button_create(parent);
    lv_obj_set_size(btn, 150, LV_PCT(100));
    lv_obj_set_style_bg_opa(btn, LV_OPA_0, 0);
    lv_obj_set_style_shadow_width(btn, 30, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(btn, lv_color_black(), LV_PART_MAIN);

    if (img != NULL) {
        lv_obj_t *icon = lv_image_create(btn);
        lv_image_set_src(icon, img);
        lv_obj_center(icon);
    }

    lv_obj_set_user_data(btn, (void *)name);
}

/* ================================================================
 *  TEST: Main menu screen with real app icons
 * ================================================================ */

void test_factory_main_menu(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_radius(scr, 0, 0);

    /* Create tileview (same as factory) */
    lv_obj_t *tileview = lv_tileview_create(scr);
    lv_obj_set_size(tileview, LV_PCT(100), LV_PCT(100));
    lv_obj_set_scrollbar_mode(tileview, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(tileview, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *menu_tile = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_HOR);
    lv_tileview_add_tile(tileview, 0, 1, LV_DIR_HOR);

    /* Create icon panel (70% height, horizontal scroll) */
    static lv_style_t style_panel;
    lv_style_init(&style_panel);
    lv_style_set_radius(&style_panel, 0);
    lv_style_set_border_width(&style_panel, 0);
    lv_style_set_bg_color(&style_panel, lv_color_white());
    lv_style_set_shadow_width(&style_panel, 55);
    lv_style_set_shadow_color(&style_panel, lv_color_black());

    lv_obj_t *panel = lv_obj_create(menu_tile);
    lv_obj_set_scrollbar_mode(panel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_size(panel, LV_PCT(100), LV_PCT(70));
    lv_obj_set_scroll_snap_x(panel, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_ROW);
    lv_obj_align(panel, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_style(panel, &style_panel, 0);

    /* Add real app icons */
    create_app_icon(panel, "Setting",    &img_configuration);
    create_app_icon(panel, "Wireless",   &img_wifi);
    create_app_icon(panel, "Music",      &img_music);
    create_app_icon(panel, "LoRa",       &img_radio);
    create_app_icon(panel, "LoRa Chat",  &img_msgchat);
    create_app_icon(panel, "Monitor",    &img_monitoring);
    create_app_icon(panel, "Keyboard",   &img_keyboard);
    create_app_icon(panel, "Bluetooth",  &img_bluetooth);
    create_app_icon(panel, "Screen Test", &img_test);

    /* Description label (bottom 30%) with real font */
    lv_obj_t *desc = lv_label_create(menu_tile);
    lv_obj_set_width(desc, LV_PCT(100));
    lv_obj_align(desc, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_text_align(desc, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(desc, &font_alibaba_40, 0);
    lv_label_set_text(desc, "Setting");
    lv_label_set_long_mode(desc, LV_LABEL_LONG_SCROLL_CIRCULAR);

    lvgl_test_run(200);

    /* Verify rendering */
    uint16_t *fb = lvgl_sim_get_framebuffer();
    int non_zero = 0;
    for (int i = 0; i < EXPECTED_HOR_RES * EXPECTED_VER_RES; i++) {
        if (fb[i] != 0) non_zero++;
    }
    /* With icons and text, significant portion should be non-zero */
    TEST_ASSERT_TRUE(non_zero > 1000);

    int result = lvgl_test_save_ppm("factory_main_menu.ppm");
    TEST_ASSERT_EQUAL_INT(0, result);
}

/* ================================================================
 *  TEST: Clock screen with real fonts (low-power mode)
 * ================================================================ */

void test_factory_clock_screen(void)
{
    lv_obj_t *scr = lv_screen_active();

    /* Dark background */
    lv_obj_t *page = lv_obj_create(scr);
    lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));
    lv_obj_remove_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_radius(page, 0, 0);
    lv_obj_set_style_bg_color(page, lv_color_hex(0x1a1a2e), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, LV_PART_MAIN);

    /* Hour container (left) */
    lv_obj_t *hour_cont = lv_obj_create(page);
    lv_obj_set_size(hour_cont, LV_PCT(35), LV_PCT(70));
    lv_obj_align(hour_cont, LV_ALIGN_LEFT_MID, 35, -20);
    lv_obj_set_style_bg_opa(hour_cont, LV_OPA_20, LV_PART_MAIN);
    lv_obj_set_style_border_opa(hour_cont, LV_OPA_60, LV_PART_MAIN);
    lv_obj_remove_flag(hour_cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *hour_label = lv_label_create(hour_cont);
    lv_obj_set_style_text_font(hour_label, &font_alibaba_100, LV_PART_MAIN);
    lv_label_set_text(hour_label, "12");
    lv_obj_set_style_text_color(hour_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_center(hour_label);

    /* Colon separator */
    lv_obj_t *colon = lv_label_create(page);
    lv_obj_align(colon, LV_ALIGN_CENTER, 0, -30);
    lv_obj_set_style_text_font(colon, &font_alibaba_100, LV_PART_MAIN);
    lv_label_set_text(colon, ":");
    lv_obj_set_style_text_color(colon, lv_color_white(), LV_PART_MAIN);

    /* Minute container (right) */
    lv_obj_t *min_cont = lv_obj_create(page);
    lv_obj_set_size(min_cont, LV_PCT(35), LV_PCT(70));
    lv_obj_align(min_cont, LV_ALIGN_RIGHT_MID, -35, -20);
    lv_obj_set_style_bg_opa(min_cont, LV_OPA_20, LV_PART_MAIN);
    lv_obj_set_style_border_opa(min_cont, LV_OPA_60, LV_PART_MAIN);
    lv_obj_remove_flag(min_cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *min_label = lv_label_create(min_cont);
    lv_obj_set_style_text_font(min_label, &font_alibaba_100, LV_PART_MAIN);
    lv_label_set_text(min_label, "34");
    lv_obj_set_style_text_color(min_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_center(min_label);

    /* Date line */
    lv_obj_t *date = lv_label_create(page);
    lv_obj_set_style_text_font(date, &font_alibaba_24, LV_PART_MAIN);
    lv_label_set_text(date, "03-09 Sun");
    lv_obj_set_style_text_color(date, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(date, LV_ALIGN_BOTTOM_MID, 0, -5);

    /* Battery indicator (using bar widget, img_battery is v8-only) */
    lv_obj_t *bat_cont = lv_obj_create(page);
    lv_obj_set_size(bat_cont, 40, 18);
    lv_obj_align(bat_cont, LV_ALIGN_BOTTOM_RIGHT, -60, -5);
    lv_obj_set_style_bg_opa(bat_cont, LV_OPA_0, 0);
    lv_obj_set_style_border_color(bat_cont, lv_color_white(), 0);
    lv_obj_set_style_border_width(bat_cont, 1, 0);
    lv_obj_set_style_radius(bat_cont, 2, 0);
    lv_obj_set_style_pad_all(bat_cont, 2, 0);

    lv_obj_t *bar = lv_bar_create(bat_cont);
    lv_obj_set_size(bar, 32, 10);
    lv_bar_set_value(bar, 75, LV_ANIM_OFF);
    lv_obj_set_style_radius(bar, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(bar, 0, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(bar, lv_color_make(0, 255, 0), LV_PART_INDICATOR);
    lv_obj_center(bar);

    lv_obj_t *bat_pct = lv_label_create(page);
    lv_obj_set_style_text_font(bat_pct, &font_alibaba_12, LV_PART_MAIN);
    lv_label_set_text(bat_pct, "75%");
    lv_obj_set_style_text_color(bat_pct, lv_color_white(), LV_PART_MAIN);
    lv_obj_align_to(bat_pct, bat_cont, LV_ALIGN_OUT_LEFT_MID, -5, 0);

    lvgl_test_run(200);

    /* Verify clock elements rendered */
    uint16_t *fb = lvgl_sim_get_framebuffer();
    int non_zero = 0;
    for (int i = 0; i < EXPECTED_HOR_RES * EXPECTED_VER_RES; i++) {
        if (fb[i] != 0) non_zero++;
    }
    TEST_ASSERT_TRUE(non_zero > 5000);

    int result = lvgl_test_save_ppm("factory_clock_screen.ppm");
    TEST_ASSERT_EQUAL_INT(0, result);
}

/* ================================================================
 *  TEST: Settings-style menu screen
 * ================================================================ */

void test_factory_settings_screen(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x1a1a2e), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* Title bar */
    lv_obj_t *title = lv_label_create(scr);
    lv_obj_set_style_text_font(title, &font_alibaba_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text(title, "Settings");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);

    /* Settings container */
    lv_obj_t *cont = lv_obj_create(scr);
    lv_obj_set_size(cont, LV_PCT(95), LV_PCT(80));
    lv_obj_align(cont, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(cont, 8, 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x16213e), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_radius(cont, 10, 0);

    /* Brightness slider row */
    lv_obj_t *row1 = lv_obj_create(cont);
    lv_obj_set_size(row1, LV_PCT(100), 40);
    lv_obj_set_flex_flow(row1, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_opa(row1, LV_OPA_0, 0);
    lv_obj_set_style_border_width(row1, 0, 0);
    lv_obj_set_style_pad_all(row1, 0, 0);

    lv_obj_t *lbl1 = lv_label_create(row1);
    lv_obj_set_style_text_font(lbl1, &font_alibaba_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(lbl1, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text(lbl1, "Brightness");
    lv_obj_set_width(lbl1, 100);

    lv_obj_t *slider = lv_slider_create(row1);
    lv_obj_set_flex_grow(slider, 1);
    lv_slider_set_range(slider, 0, 16);
    lv_slider_set_value(slider, 10, LV_ANIM_OFF);

    /* WiFi switch row */
    lv_obj_t *row2 = lv_obj_create(cont);
    lv_obj_set_size(row2, LV_PCT(100), 40);
    lv_obj_set_flex_flow(row2, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_opa(row2, LV_OPA_0, 0);
    lv_obj_set_style_border_width(row2, 0, 0);
    lv_obj_set_style_pad_all(row2, 0, 0);

    lv_obj_t *lbl2 = lv_label_create(row2);
    lv_obj_set_style_text_font(lbl2, &font_alibaba_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(lbl2, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text(lbl2, "WiFi");
    lv_obj_set_width(lbl2, 100);

    lv_obj_t *sw = lv_switch_create(row2);
    lv_obj_add_state(sw, LV_STATE_CHECKED);

    /* Charger current row */
    lv_obj_t *row3 = lv_obj_create(cont);
    lv_obj_set_size(row3, LV_PCT(100), 40);
    lv_obj_set_flex_flow(row3, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_opa(row3, LV_OPA_0, 0);
    lv_obj_set_style_border_width(row3, 0, 0);
    lv_obj_set_style_pad_all(row3, 0, 0);

    lv_obj_t *lbl3 = lv_label_create(row3);
    lv_obj_set_style_text_font(lbl3, &font_alibaba_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(lbl3, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text(lbl3, "Charge Current");
    lv_obj_set_width(lbl3, 100);

    lv_obj_t *slider2 = lv_slider_create(row3);
    lv_obj_set_flex_grow(slider2, 1);
    lv_slider_set_range(slider2, 128, 2048);
    lv_slider_set_value(slider2, 512, LV_ANIM_OFF);

    lvgl_test_run(200);

    uint16_t *fb = lvgl_sim_get_framebuffer();
    int non_zero = 0;
    for (int i = 0; i < EXPECTED_HOR_RES * EXPECTED_VER_RES; i++) {
        if (fb[i] != 0) non_zero++;
    }
    TEST_ASSERT_TRUE(non_zero > 5000);

    int result = lvgl_test_save_ppm("factory_settings.ppm");
    TEST_ASSERT_EQUAL_INT(0, result);
}

/* ================================================================
 *  TEST: LoRa messaging screen
 * ================================================================ */

void test_factory_lora_chat_screen(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x0a0a1a), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* Header with icon */
    lv_obj_t *header = lv_obj_create(scr);
    lv_obj_set_size(header, LV_PCT(100), 35);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x16213e), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_radius(header, 0, 0);

    lv_obj_t *chat_icon = lv_image_create(header);
    lv_image_set_src(chat_icon, &img_msgchat);
    lv_image_set_scale(chat_icon, 64);  /* Scale down to ~28px */
    lv_obj_align(chat_icon, LV_ALIGN_LEFT_MID, 5, 0);

    lv_obj_t *htitle = lv_label_create(header);
    lv_obj_set_style_text_font(htitle, &font_alibaba_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(htitle, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text(htitle, "LoRa Chat");
    lv_obj_align(htitle, LV_ALIGN_LEFT_MID, 45, 0);

    /* RSSI indicator */
    lv_obj_t *rssi = lv_label_create(header);
    lv_obj_set_style_text_font(rssi, &font_alibaba_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(rssi, lv_color_hex(0x00ff00), LV_PART_MAIN);
    lv_label_set_text(rssi, "RSSI: -45 dBm");
    lv_obj_align(rssi, LV_ALIGN_RIGHT_MID, -10, 0);

    /* Message area */
    lv_obj_t *msg_area = lv_obj_create(scr);
    lv_obj_set_size(msg_area, LV_PCT(95), 140);
    lv_obj_align(msg_area, LV_ALIGN_TOP_MID, 0, 38);
    lv_obj_set_flex_flow(msg_area, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(msg_area, 5, 0);
    lv_obj_set_style_bg_color(msg_area, lv_color_hex(0x0f0f23), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(msg_area, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(msg_area, 1, 0);
    lv_obj_set_style_border_color(msg_area, lv_color_hex(0x333366), 0);
    lv_obj_set_style_radius(msg_area, 5, 0);

    /* Sample messages */
    const char *msgs[] = {
        "TX: Hello from Pager 1",
        "RX: Copy that, signal strong",
        "TX: Testing 868 MHz link",
        "RX: Received, RSSI -42 dBm",
    };
    lv_color_t colors[] = {
        lv_color_hex(0x4488ff),
        lv_color_hex(0x44ff88),
        lv_color_hex(0x4488ff),
        lv_color_hex(0x44ff88),
    };

    for (int i = 0; i < 4; i++) {
        lv_obj_t *msg = lv_label_create(msg_area);
        lv_obj_set_style_text_font(msg, &font_alibaba_12, LV_PART_MAIN);
        lv_obj_set_style_text_color(msg, colors[i], LV_PART_MAIN);
        lv_label_set_text(msg, msgs[i]);
    }

    /* Input area at bottom */
    lv_obj_t *input_bar = lv_obj_create(scr);
    lv_obj_set_size(input_bar, LV_PCT(95), 35);
    lv_obj_align(input_bar, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_set_style_bg_color(input_bar, lv_color_hex(0x16213e), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(input_bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(input_bar, 1, 0);
    lv_obj_set_style_border_color(input_bar, lv_color_hex(0x333366), 0);
    lv_obj_set_style_radius(input_bar, 5, 0);

    lv_obj_t *input_text = lv_label_create(input_bar);
    lv_obj_set_style_text_font(input_text, &font_alibaba_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(input_text, lv_color_hex(0x666688), LV_PART_MAIN);
    lv_label_set_text(input_text, "Type message...");
    lv_obj_align(input_text, LV_ALIGN_LEFT_MID, 10, 0);

    lvgl_test_run(200);

    uint16_t *fb = lvgl_sim_get_framebuffer();
    int non_zero = 0;
    for (int i = 0; i < EXPECTED_HOR_RES * EXPECTED_VER_RES; i++) {
        if (fb[i] != 0) non_zero++;
    }
    TEST_ASSERT_TRUE(non_zero > 5000);

    int result = lvgl_test_save_ppm("factory_lora_chat.ppm");
    TEST_ASSERT_EQUAL_INT(0, result);
}

/* ================================================================
 *  TEST: Startup logo screen (as seen on boot)
 * ================================================================ */

void test_factory_logo_screen(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(scr, 0, 0);

    lv_obj_t *logo = lv_label_create(scr);
    lv_label_set_text(logo, "LilyGo");
    lv_obj_set_style_text_font(logo, &font_alibaba_40, LV_PART_MAIN);
    lv_obj_set_style_text_color(logo, lv_color_white(), LV_PART_MAIN);
    lv_obj_center(logo);

    lvgl_test_run(100);

    uint16_t *fb = lvgl_sim_get_framebuffer();
    int non_zero = 0;
    for (int i = 0; i < EXPECTED_HOR_RES * EXPECTED_VER_RES; i++) {
        if (fb[i] != 0) non_zero++;
    }
    TEST_ASSERT_TRUE(non_zero > 100);

    int result = lvgl_test_save_ppm("factory_logo.ppm");
    TEST_ASSERT_EQUAL_INT(0, result);
}

/* ================================================================
 *  TEST: Monitor/status screen with real fonts
 * ================================================================ */

void test_factory_monitor_screen(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x0a0a1a), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* Title */
    lv_obj_t *title = lv_label_create(scr);
    lv_obj_set_style_text_font(title, &font_alibaba_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text(title, "System Monitor");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);

    /* Two-column layout */
    lv_obj_t *left = lv_obj_create(scr);
    lv_obj_set_size(left, LV_PCT(48), LV_PCT(75));
    lv_obj_align(left, LV_ALIGN_BOTTOM_LEFT, 5, -5);
    lv_obj_set_flex_flow(left, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(left, 4, 0);
    lv_obj_set_style_bg_color(left, lv_color_hex(0x16213e), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(left, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(left, 0, 0);
    lv_obj_set_style_radius(left, 8, 0);

    const char *left_items[] = {
        "Battery: 3850 mV",
        "USB: 5120 mV",
        "Charge: Active",
        "Temp: 32.5 C",
    };
    for (int i = 0; i < 4; i++) {
        lv_obj_t *lbl = lv_label_create(left);
        lv_obj_set_style_text_font(lbl, &font_alibaba_12, LV_PART_MAIN);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xccccff), LV_PART_MAIN);
        lv_label_set_text(lbl, left_items[i]);
    }

    lv_obj_t *right = lv_obj_create(scr);
    lv_obj_set_size(right, LV_PCT(48), LV_PCT(75));
    lv_obj_align(right, LV_ALIGN_BOTTOM_RIGHT, -5, -5);
    lv_obj_set_flex_flow(right, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(right, 4, 0);
    lv_obj_set_style_bg_color(right, lv_color_hex(0x16213e), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(right, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(right, 0, 0);
    lv_obj_set_style_radius(right, 8, 0);

    const char *right_items[] = {
        "Capacity: 82%",
        "Current: 245 mA",
        "Remaining: 1850 mAh",
        "Time to Full: 45 min",
    };
    for (int i = 0; i < 4; i++) {
        lv_obj_t *lbl = lv_label_create(right);
        lv_obj_set_style_text_font(lbl, &font_alibaba_12, LV_PART_MAIN);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xccccff), LV_PART_MAIN);
        lv_label_set_text(lbl, right_items[i]);
    }

    lvgl_test_run(200);

    int result = lvgl_test_save_ppm("factory_monitor.ppm");
    TEST_ASSERT_EQUAL_INT(0, result);
}

/* ================================================================
 *  TEST: Verify font rendering quality
 * ================================================================ */

void test_font_rendering_quality(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* Show all font sizes on one screen */
    lv_obj_t *l12 = lv_label_create(scr);
    lv_obj_set_style_text_font(l12, &font_alibaba_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(l12, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text(l12, "Alibaba 12px: T-LoRa-Pager 0123456789");
    lv_obj_align(l12, LV_ALIGN_TOP_LEFT, 10, 10);

    lv_obj_t *l24 = lv_label_create(scr);
    lv_obj_set_style_text_font(l24, &font_alibaba_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(l24, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text(l24, "Alibaba 24px: Settings");
    lv_obj_align(l24, LV_ALIGN_TOP_LEFT, 10, 30);

    lv_obj_t *l40 = lv_label_create(scr);
    lv_obj_set_style_text_font(l40, &font_alibaba_40, LV_PART_MAIN);
    lv_obj_set_style_text_color(l40, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text(l40, "Alibaba 40px: Menu");
    lv_obj_align(l40, LV_ALIGN_TOP_LEFT, 10, 60);

    lv_obj_t *l100 = lv_label_create(scr);
    lv_obj_set_style_text_font(l100, &font_alibaba_100, LV_PART_MAIN);
    lv_obj_set_style_text_color(l100, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text(l100, "12:34");
    lv_obj_align(l100, LV_ALIGN_BOTTOM_MID, 0, -10);

    lvgl_test_run(200);

    int result = lvgl_test_save_ppm("factory_fonts.ppm");
    TEST_ASSERT_EQUAL_INT(0, result);

    /* Verify significant rendering happened */
    uint16_t *fb = lvgl_sim_get_framebuffer();
    int non_zero = 0;
    for (int i = 0; i < EXPECTED_HOR_RES * EXPECTED_VER_RES; i++) {
        if (fb[i] != 0) non_zero++;
    }
    TEST_ASSERT_TRUE(non_zero > 2000);
}

/* ================================================================
 *  TEST: Icon rendering verification
 * ================================================================ */

void test_icon_rendering(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x1a1a2e), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* Grid of all included icons with labels */
    const struct {
        const lv_image_dsc_t *img;
        const char *name;
    } icons[] = {
        {&img_configuration, "Settings"},
        {&img_radio,         "Radio"},
        {&img_msgchat,       "Chat"},
        {&img_music,         "Music"},
        {&img_monitoring,    "Monitor"},
        {&img_keyboard,      "Keyboard"},
        {&img_bluetooth,     "BT"},
        {&img_wifi,          "WiFi"},
        {&img_test,          "Test"},
    };

    int cols = 5;
    int icon_w = EXPECTED_HOR_RES / cols;
    int icon_h = EXPECTED_VER_RES / 2;

    for (int i = 0; i < 9; i++) {
        int col = i % cols;
        int row = i / cols;

        lv_obj_t *img_obj = lv_image_create(scr);
        lv_image_set_src(img_obj, icons[i].img);
        lv_image_set_scale(img_obj, 180);  /* Scale to ~50px */
        lv_obj_set_pos(img_obj, col * icon_w + (icon_w - 50) / 2, row * icon_h + 10);

        lv_obj_t *label = lv_label_create(scr);
        lv_obj_set_style_text_font(label, &font_alibaba_12, LV_PART_MAIN);
        lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
        lv_label_set_text(label, icons[i].name);
        lv_obj_set_pos(label, col * icon_w + 10, row * icon_h + 70);
    }

    lvgl_test_run(200);

    int result = lvgl_test_save_ppm("factory_icons.ppm");
    TEST_ASSERT_EQUAL_INT(0, result);
}

/* ================================================================
 *  TEST: Screenshot file sizes are valid
 * ================================================================ */

void test_screenshot_sizes(void)
{
    /* Render something */
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x003366), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    lv_obj_t *label = lv_label_create(scr);
    lv_obj_set_style_text_font(label, &font_alibaba_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text(label, "T-LoRa-Pager Screenshot Test");
    lv_obj_center(label);

    lvgl_test_run(100);

    int result = lvgl_test_save_ppm("factory_screenshot_test.ppm");
    TEST_ASSERT_EQUAL_INT(0, result);

    /* PPM size: header + 480*222*3 = ~319,680+ bytes */
    FILE *fp = fopen("factory_screenshot_test.ppm", "rb");
    TEST_ASSERT_NOT_NULL(fp);
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fclose(fp);
    TEST_ASSERT_TRUE(size > 300000);
}

/* ================================================================
 *  MAIN
 * ================================================================ */

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    UNITY_BEGIN();

    /* Factory app screens */
    RUN_TEST(test_factory_main_menu);
    RUN_TEST(test_factory_clock_screen);
    RUN_TEST(test_factory_settings_screen);
    RUN_TEST(test_factory_lora_chat_screen);
    RUN_TEST(test_factory_logo_screen);
    RUN_TEST(test_factory_monitor_screen);

    /* Asset quality verification */
    RUN_TEST(test_font_rendering_quality);
    RUN_TEST(test_icon_rendering);
    RUN_TEST(test_screenshot_sizes);

    return UNITY_END();
}
