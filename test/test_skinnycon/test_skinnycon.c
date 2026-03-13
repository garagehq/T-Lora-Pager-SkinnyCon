/**
 * @file test_skinnycon.c
 * SkinnyCon app simulation tests for T-LoRa-Pager (480x222 RGB565).
 *
 * Renders actual SkinnyCon app screens using real assets (fonts, icons)
 * from application/SkinnyCon/src/. Screenshots are saved as PPM files for
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
#include "../../application/SkinnyCon/src/img_configuration_v9.c"
#include "../../application/SkinnyCon/src/img_radio_v9.c"
#include "../../application/SkinnyCon/src/img_msgchat_v9.c"
#include "../../application/SkinnyCon/src/img_music_v9.c"
#include "../../application/SkinnyCon/src/img_monitoring_v9.c"
#include "../../application/SkinnyCon/src/img_keyboard_v9.c"
#include "../../application/SkinnyCon/src/img_bluetooth_v9.c"
#include "../../application/SkinnyCon/src/img_wifi_v9.c"
#include "../../application/SkinnyCon/src/img_test_v9.c"

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

void test_skinnycon_main_menu(void)
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
 *  TEST: Supercon Nametag screen (standalone C recreation)
 * ================================================================ */

#define SUPERCON_BG_C       lv_color_hex(0x1A1A1A)
#define SUPERCON_ACCENT_C   lv_color_hex(0xE39810)
#define SUPERCON_WHITE_C    lv_color_hex(0xFFFFFF)
#define SUPERCON_GREEN_C    lv_color_hex(0xABC5A0)

void test_supercon_nametag(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, SUPERCON_BG_C, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    lv_obj_t *cont = lv_obj_create(scr);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(cont, SUPERCON_BG_C, 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_radius(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 8, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* Top accent bar */
    lv_obj_t *accent = lv_obj_create(cont);
    lv_obj_set_size(accent, LV_PCT(100), 4);
    lv_obj_set_style_bg_color(accent, SUPERCON_ACCENT_C, 0);
    lv_obj_set_style_bg_opa(accent, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(accent, 0, 0);
    lv_obj_set_style_radius(accent, 0, 0);

    /* Name */
    lv_obj_t *name = lv_label_create(cont);
    lv_label_set_text(name, "SkinnyCon");
    lv_obj_set_style_text_font(name, &font_alibaba_40, 0);
    lv_obj_set_style_text_color(name, SUPERCON_WHITE_C, 0);
    lv_obj_set_style_text_align(name, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(name, LV_PCT(100));

    /* Subtitle */
    lv_obj_t *sub = lv_label_create(cont);
    lv_label_set_text(sub, "T-LoRa-Pager");
    lv_obj_set_style_text_font(sub, &font_alibaba_24, 0);
    lv_obj_set_style_text_color(sub, SUPERCON_GREEN_C, 0);
    lv_obj_set_style_text_align(sub, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(sub, LV_PCT(100));

    /* Bottom accent bar */
    lv_obj_t *bottom = lv_obj_create(cont);
    lv_obj_set_size(bottom, LV_PCT(100), 4);
    lv_obj_set_style_bg_color(bottom, SUPERCON_ACCENT_C, 0);
    lv_obj_set_style_bg_opa(bottom, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(bottom, 0, 0);
    lv_obj_set_style_radius(bottom, 0, 0);

    /* Mode hint */
    lv_obj_t *hint = lv_label_create(cont);
    lv_label_set_text(hint, LV_SYMBOL_REFRESH " Rotate=mode  " LV_SYMBOL_KEYBOARD " Type=edit name");
    lv_obj_set_style_text_color(hint, lv_color_hex(0x808080), 0);
    lv_obj_set_style_text_align(hint, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(hint, LV_PCT(100));

    lvgl_test_run(200);

    uint16_t *fb = lvgl_sim_get_framebuffer();
    int non_zero = 0;
    for (int i = 0; i < EXPECTED_HOR_RES * EXPECTED_VER_RES; i++) {
        if (fb[i] != 0) non_zero++;
    }
    TEST_ASSERT_TRUE(non_zero > 1000);

    int result = lvgl_test_save_ppm("factory_nametag.ppm");
    TEST_ASSERT_EQUAL_INT(0, result);
}

/* ================================================================
 *  TEST: About SkinnyCon screen (standalone C recreation)
 * ================================================================ */

void test_supercon_about(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, SUPERCON_BG_C, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    lv_obj_t *cont = lv_obj_create(scr);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(cont, SUPERCON_BG_C, 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_radius(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 8, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *title = lv_label_create(cont);
    lv_label_set_text(title, "SKINNYCON 2026");
    lv_obj_set_style_text_font(title, &font_alibaba_24, 0);
    lv_obj_set_style_text_color(title, SUPERCON_ACCENT_C, 0);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(title, LV_PCT(100));

    lv_obj_t *panel = lv_obj_create(cont);
    lv_obj_set_size(panel, LV_PCT(95), LV_PCT(100));
    lv_obj_set_flex_grow(panel, 1);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x2A2A2A), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(panel, SUPERCON_ACCENT_C, 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_radius(panel, 6, 0);
    lv_obj_set_style_pad_all(panel, 8, 0);

    lv_obj_t *info = lv_label_create(panel);
    lv_label_set_text(info,
        "May 12-14, 2026\n"
        "Huntsville, Alabama\n"
        "I2C Invention to Innovation Center\n"
        "UAH Campus\n\n"
        "Hosted by Skinny R&D\n"
        "Jason Baird, President"
    );
    lv_obj_set_style_text_color(info, SUPERCON_WHITE_C, 0);
    lv_obj_set_width(info, LV_PCT(100));

    lvgl_test_run(200);

    uint16_t *fb = lvgl_sim_get_framebuffer();
    int non_zero = 0;
    for (int i = 0; i < EXPECTED_HOR_RES * EXPECTED_VER_RES; i++) {
        if (fb[i] != 0) non_zero++;
    }
    TEST_ASSERT_TRUE(non_zero > 1000);

    int result = lvgl_test_save_ppm("factory_about.ppm");
    TEST_ASSERT_EQUAL_INT(0, result);
}

/* ================================================================
 *  TEST: Code of Conduct screen (standalone C recreation)
 * ================================================================ */

void test_supercon_coc(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, SUPERCON_BG_C, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    lv_obj_t *cont = lv_obj_create(scr);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(cont, SUPERCON_BG_C, 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_radius(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 8, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *title = lv_label_create(cont);
    lv_label_set_text(title, LV_SYMBOL_WARNING " CODE OF CONDUCT");
    lv_obj_set_style_text_font(title, &font_alibaba_24, 0);
    lv_obj_set_style_text_color(title, SUPERCON_ACCENT_C, 0);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(title, LV_PCT(100));

    lv_obj_t *panel = lv_obj_create(cont);
    lv_obj_set_size(panel, LV_PCT(95), LV_PCT(100));
    lv_obj_set_flex_grow(panel, 1);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x2A2A2A), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(panel, SUPERCON_ACCENT_C, 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_radius(panel, 6, 0);
    lv_obj_set_style_pad_all(panel, 8, 0);

    lv_obj_t *coc = lv_label_create(panel);
    lv_label_set_text(coc,
        "OUR STANDARDS\n"
        "- Be kind, considerate, respectful\n"
        "- Behave professionally\n"
        "- Respect differing viewpoints\n"
        "- Be mindful of personal space\n\n"
        "UAH is 100% tobacco free."
    );
    lv_obj_set_style_text_color(coc, SUPERCON_WHITE_C, 0);
    lv_obj_set_width(coc, LV_PCT(100));

    lvgl_test_run(200);

    uint16_t *fb = lvgl_sim_get_framebuffer();
    int non_zero = 0;
    for (int i = 0; i < EXPECTED_HOR_RES * EXPECTED_VER_RES; i++) {
        if (fb[i] != 0) non_zero++;
    }
    TEST_ASSERT_TRUE(non_zero > 1000);

    int result = lvgl_test_save_ppm("factory_coc.ppm");
    TEST_ASSERT_EQUAL_INT(0, result);
}

/* ================================================================
 *  TEST: Supercon BadgeShark screen (standalone C recreation)
 * ================================================================ */

#define SHARK_BG_C          lv_color_hex(0x0D1117)
#define SHARK_GREEN_C       lv_color_hex(0x39D353)
#define SHARK_YELLOW_C      lv_color_hex(0xE39810)
#define SHARK_CYAN_C        lv_color_hex(0x58A6FF)
#define SHARK_WHITE_C       lv_color_hex(0xE6EDF3)

void test_supercon_badgeshark(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, SHARK_BG_C, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    lv_obj_t *cont = lv_obj_create(scr);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(cont, SHARK_BG_C, 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_radius(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);

    /* Header */
    lv_obj_t *header = lv_obj_create(cont);
    lv_obj_set_size(header, LV_PCT(100), 28);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x21262D), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_pad_hor(header, 8, 0);

    lv_obj_t *title = lv_label_create(header);
    lv_label_set_text(title, LV_SYMBOL_EYE_OPEN " BadgeShark");
    lv_obj_set_style_text_color(title, SHARK_GREEN_C, 0);
    lv_obj_set_style_text_font(title, &font_alibaba_12, 0);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t *stats = lv_label_create(header);
    lv_label_set_text(stats, "Packets: 5  |  Bytes: 148  |  RSSI: -58 dBm");
    lv_obj_set_style_text_color(stats, SHARK_WHITE_C, 0);
    lv_obj_align(stats, LV_ALIGN_RIGHT_MID, 0, 0);

    /* Column headers */
    lv_obj_t *col_header = lv_obj_create(cont);
    lv_obj_set_size(col_header, LV_PCT(100), 20);
    lv_obj_set_style_bg_color(col_header, lv_color_hex(0x30363D), 0);
    lv_obj_set_style_bg_opa(col_header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(col_header, 0, 0);
    lv_obj_set_style_radius(col_header, 0, 0);
    lv_obj_set_style_pad_hor(col_header, 6, 0);
    lv_obj_set_flex_flow(col_header, LV_FLEX_FLOW_ROW);

    const char *cols[] = {"#", "Len", "RSSI", "Data"};
    int col_widths[] = {50, 40, 65, 0};
    for (int i = 0; i < 4; i++) {
        lv_obj_t *c = lv_label_create(col_header);
        lv_label_set_text(c, cols[i]);
        lv_obj_set_style_text_color(c, SHARK_CYAN_C, 0);
        if (col_widths[i] > 0) lv_obj_set_style_min_width(c, col_widths[i], 0);
        else lv_obj_set_flex_grow(c, 1);
    }

    /* Packet list */
    lv_obj_t *plist = lv_obj_create(cont);
    lv_obj_set_size(plist, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(plist, 1);
    lv_obj_set_style_bg_color(plist, SHARK_BG_C, 0);
    lv_obj_set_style_bg_opa(plist, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(plist, 0, 0);
    lv_obj_set_style_radius(plist, 0, 0);
    lv_obj_set_style_pad_all(plist, 0, 0);
    lv_obj_set_flex_flow(plist, LV_FLEX_FLOW_COLUMN);

    /* Demo packet rows */
    const char *pkt_nums[] = {"#1", "#2", "#3", "#4", "#5"};
    const char *pkt_lens[] = {"32B", "28B", "16B", "48B", "24B"};
    const char *pkt_rssi[] = {"-45dBm", "-72dBm", "-95dBm", "-58dBm", "-83dBm"};
    const char *pkt_data[] = {
        "07 E9 3A 1F FF FF FF FF 00 01",
        "07 E9 7B 2E FF FF FF FF 00 04",
        "07 E9 A1 00 00 01 02 03 00 04",
        "07 E9 12 44 FF FF FF FF 00 0A",
        "07 E9 55 67 FF FF FF FF 00 07",
    };

    for (int i = 0; i < 5; i++) {
        lv_obj_t *row = lv_obj_create(plist);
        lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_color(row, (i % 2) ? lv_color_hex(0x161B22) : SHARK_BG_C, 0);
        lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_radius(row, 0, 0);
        lv_obj_set_style_pad_ver(row, 2, 0);
        lv_obj_set_style_pad_hor(row, 6, 0);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);

        lv_obj_t *num = lv_label_create(row);
        lv_label_set_text(num, pkt_nums[i]);
        lv_obj_set_style_text_color(num, SHARK_CYAN_C, 0);
        lv_obj_set_style_min_width(num, 50, 0);

        lv_obj_t *len = lv_label_create(row);
        lv_label_set_text(len, pkt_lens[i]);
        lv_obj_set_style_text_color(len, SHARK_YELLOW_C, 0);
        lv_obj_set_style_min_width(len, 40, 0);

        lv_obj_t *rssi = lv_label_create(row);
        lv_label_set_text(rssi, pkt_rssi[i]);
        lv_obj_set_style_text_color(rssi, SHARK_GREEN_C, 0);
        lv_obj_set_style_min_width(rssi, 65, 0);

        lv_obj_t *data = lv_label_create(row);
        lv_label_set_text(data, pkt_data[i]);
        lv_obj_set_style_text_color(data, SHARK_WHITE_C, 0);
        lv_obj_set_flex_grow(data, 1);
    }

    lvgl_test_run(200);

    uint16_t *fb = lvgl_sim_get_framebuffer();
    int non_zero = 0;
    for (int i = 0; i < EXPECTED_HOR_RES * EXPECTED_VER_RES; i++) {
        if (fb[i] != 0) non_zero++;
    }
    TEST_ASSERT_TRUE(non_zero > 1000);

    int result = lvgl_test_save_ppm("factory_badgeshark.ppm");
    TEST_ASSERT_EQUAL_INT(0, result);
}

/* ================================================================
 *  TEST: Supercon Schedule screen (standalone C recreation)
 * ================================================================ */

void test_supercon_schedule(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x1A1A1A), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    lv_obj_t *cont = lv_obj_create(scr);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_radius(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);

    /* Header */
    lv_obj_t *header = lv_obj_create(cont);
    lv_obj_set_size(header, LV_PCT(100), 28);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x21262D), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_pad_hor(header, 8, 0);

    lv_obj_t *htitle = lv_label_create(header);
    lv_label_set_text(htitle, LV_SYMBOL_LIST " SkinnyCon 2026");
    lv_obj_set_style_text_color(htitle, SUPERCON_ACCENT_C, 0);
    lv_obj_set_style_text_font(htitle, &font_alibaba_12, 0);
    lv_obj_align(htitle, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t *day = lv_label_create(header);
    lv_label_set_text(day, LV_SYMBOL_LEFT " Tue May 12 " LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_color(day, SHARK_WHITE_C, 0);
    lv_obj_align(day, LV_ALIGN_RIGHT_MID, 0, 0);

    /* Talk list */
    lv_obj_t *list = lv_obj_create(cont);
    lv_obj_set_size(list, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(list, 1);
    lv_obj_set_style_bg_color(list, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_bg_opa(list, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_style_radius(list, 0, 0);
    lv_obj_set_style_pad_all(list, 0, 0);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_AUTO);

    const char *times[] = {"0800", "0900", "0915", "0930", "1030", "1100"};
    const char *titles[] = {"Check-in / Breakfast", "Welcome", "How to CTF",
                            "Tech Ops Case Files", "Break", "Intro to Reverse Engineering"};
    int is_break[] = {1, 0, 0, 0, 1, 0};

    for (int i = 0; i < 6; i++) {
        lv_obj_t *row = lv_obj_create(list);
        lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_color(row, (i == 0) ? lv_color_hex(0x30363D) :
            ((i % 2) ? lv_color_hex(0x161B22) : lv_color_hex(0x1A1A1A)), 0);
        lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
        if (i == 0) {
            lv_obj_set_style_border_side(row, LV_BORDER_SIDE_LEFT, 0);
            lv_obj_set_style_border_width(row, 3, 0);
            lv_obj_set_style_border_color(row, SUPERCON_ACCENT_C, 0);
        } else {
            lv_obj_set_style_border_width(row, 0, 0);
        }
        lv_obj_set_style_radius(row, 0, 0);
        lv_obj_set_style_pad_ver(row, 3, 0);
        lv_obj_set_style_pad_hor(row, 8, 0);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);

        lv_obj_t *t = lv_label_create(row);
        lv_label_set_text(t, times[i]);
        lv_obj_set_style_text_color(t, SHARK_CYAN_C, 0);
        lv_obj_set_style_min_width(t, 45, 0);

        lv_obj_t *tl = lv_label_create(row);
        lv_label_set_text(tl, titles[i]);
        lv_obj_set_style_text_color(tl, is_break[i] ? lv_color_hex(0x808080) : SHARK_WHITE_C, 0);
        lv_obj_set_flex_grow(tl, 1);
    }

    lvgl_test_run(200);

    uint16_t *fb = lvgl_sim_get_framebuffer();
    int non_zero = 0;
    for (int i = 0; i < EXPECTED_HOR_RES * EXPECTED_VER_RES; i++) {
        if (fb[i] != 0) non_zero++;
    }
    TEST_ASSERT_TRUE(non_zero > 1000);

    int result = lvgl_test_save_ppm("factory_schedule.ppm");
    TEST_ASSERT_EQUAL_INT(0, result);
}

/* ================================================================
 *  TEST: Supercon Net Tools screen (standalone C recreation)
 * ================================================================ */

#define NET_BG_C            lv_color_hex(0x0D1117)
#define NET_GREEN_C         lv_color_hex(0x39D353)
#define NET_YELLOW_C        lv_color_hex(0xE39810)
#define NET_RED_C           lv_color_hex(0xFF6B6B)
#define NET_PANEL_C         lv_color_hex(0x161B22)

void test_supercon_nettools(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, NET_BG_C, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    lv_obj_t *cont = lv_obj_create(scr);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(cont, NET_BG_C, 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_radius(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);

    /* Header */
    lv_obj_t *header = lv_obj_create(cont);
    lv_obj_set_size(header, LV_PCT(100), 28);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x21262D), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_radius(header, 0, 0);
    lv_obj_set_style_pad_hor(header, 8, 0);

    lv_obj_t *ntitle = lv_label_create(header);
    lv_label_set_text(ntitle, LV_SYMBOL_WIFI " Net Tools");
    lv_obj_set_style_text_color(ntitle, NET_GREEN_C, 0);
    lv_obj_set_style_text_font(ntitle, &font_alibaba_12, 0);
    lv_obj_align(ntitle, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t *nstats = lv_label_create(header);
    lv_label_set_text(nstats, "Sent: 8  |  Recv: 6  |  Loss: 25%  |  Avg: 60 ms");
    lv_obj_set_style_text_color(nstats, SHARK_WHITE_C, 0);
    lv_obj_align(nstats, LV_ALIGN_RIGHT_MID, 0, 0);

    /* Body: ping log + peers */
    lv_obj_t *body = lv_obj_create(cont);
    lv_obj_set_size(body, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(body, 1);
    lv_obj_set_style_bg_opa(body, LV_OPA_0, 0);
    lv_obj_set_style_border_width(body, 0, 0);
    lv_obj_set_style_pad_all(body, 4, 0);
    lv_obj_set_flex_flow(body, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(body, 4, 0);

    /* Ping log panel */
    lv_obj_t *ping_panel = lv_obj_create(body);
    lv_obj_set_flex_grow(ping_panel, 3);
    lv_obj_set_style_bg_color(ping_panel, NET_PANEL_C, 0);
    lv_obj_set_style_bg_opa(ping_panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(ping_panel, lv_color_hex(0x30363D), 0);
    lv_obj_set_style_border_width(ping_panel, 1, 0);
    lv_obj_set_style_radius(ping_panel, 4, 0);
    lv_obj_set_style_pad_all(ping_panel, 4, 0);
    lv_obj_set_flex_flow(ping_panel, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *pt = lv_label_create(ping_panel);
    lv_label_set_text(pt, "PING LOG");
    lv_obj_set_style_text_color(pt, SHARK_CYAN_C, 0);

    const char *pings[] = {"PONG #1  RTT: 45 ms", "PONG #2  RTT: 62 ms",
                           "PING #3  TIMEOUT", "PONG #4  RTT: 38 ms",
                           "PONG #5  RTT: 125 ms", "PONG #6  RTT: 52 ms"};
    lv_color_t pcols[] = {NET_GREEN_C, NET_GREEN_C, NET_RED_C, NET_GREEN_C, NET_YELLOW_C, NET_GREEN_C};
    for (int i = 0; i < 6; i++) {
        lv_obj_t *pl = lv_label_create(ping_panel);
        lv_label_set_text(pl, pings[i]);
        lv_obj_set_style_text_color(pl, pcols[i], 0);
    }

    /* Peer panel */
    lv_obj_t *peer_panel = lv_obj_create(body);
    lv_obj_set_flex_grow(peer_panel, 2);
    lv_obj_set_style_bg_color(peer_panel, NET_PANEL_C, 0);
    lv_obj_set_style_bg_opa(peer_panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(peer_panel, lv_color_hex(0x30363D), 0);
    lv_obj_set_style_border_width(peer_panel, 1, 0);
    lv_obj_set_style_radius(peer_panel, 4, 0);
    lv_obj_set_style_pad_all(peer_panel, 4, 0);
    lv_obj_set_flex_flow(peer_panel, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *peert = lv_label_create(peer_panel);
    lv_label_set_text(peert, "PEERS");
    lv_obj_set_style_text_color(peert, SHARK_CYAN_C, 0);

    const char *peers[] = {"Badge-0x3A1F\n  -45 dBm", "Badge-0x7B2E\n  -72 dBm",
                           "Badge-0xA100\n  -95 dBm", "Badge-0x1244\n  -58 dBm"};
    lv_color_t peer_cols[] = {NET_GREEN_C, NET_GREEN_C, NET_YELLOW_C, NET_GREEN_C};
    for (int i = 0; i < 4; i++) {
        lv_obj_t *p = lv_label_create(peer_panel);
        lv_label_set_text(p, peers[i]);
        lv_obj_set_style_text_color(p, peer_cols[i], 0);
    }

    lvgl_test_run(200);

    uint16_t *fb = lvgl_sim_get_framebuffer();
    int non_zero = 0;
    for (int i = 0; i < EXPECTED_HOR_RES * EXPECTED_VER_RES; i++) {
        if (fb[i] != 0) non_zero++;
    }
    TEST_ASSERT_TRUE(non_zero > 1000);

    int result = lvgl_test_save_ppm("factory_nettools.ppm");
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

    /* Supercon-inspired app screens */
    RUN_TEST(test_supercon_nametag);
    RUN_TEST(test_supercon_about);
    RUN_TEST(test_supercon_coc);
    RUN_TEST(test_supercon_badgeshark);
    RUN_TEST(test_supercon_schedule);
    RUN_TEST(test_supercon_nettools);

    /* Asset quality verification */
    RUN_TEST(test_font_rendering_quality);
    RUN_TEST(test_icon_rendering);
    RUN_TEST(test_screenshot_sizes);

    return UNITY_END();
}
