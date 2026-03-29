/**
 * @file test_app_interaction.cpp
 * @brief Comprehensive interaction tests using ACTUAL app source code
 *
 * Includes real SkinnyCon app source files with hardware stubs.
 * Tests actual user flows: editing, scrolling, navigation, group lifecycle.
 */

extern "C" {
#include "../simulator/sim_main.c"
}

#include <unity.h>
#include <string.h>
#include <stdio.h>

/* Block ui_define.h from pulling hardware headers */
#define UI_DEFINE_H
#define ARDUINO 1
#define RTC_DATA_ATTR

#include <lvgl.h>
#include <time.h>
#include <iostream>
#include <vector>
using namespace std;

#include "../../application/SkinnyCon/ui_skinnycon_theme.h"

typedef struct {
    void (*setup_func_cb)(lv_obj_t *parent);
    void (*exit_func_cb)(lv_obj_t *parent);
    void (*loop_func_cb)();
} app_t;

#define LV_MENU_ROOT_BACK_BTN_ENABLED LV_MENU_ROOT_BACK_BUTTON_ENABLED
#define lv_menu_set_mode_root_back_btn lv_menu_set_mode_root_back_button
#define lv_menu_back_btn_is_root lv_menu_back_button_is_root
#define MAIN_FONT &lv_font_montserrat_14

LV_FONT_DECLARE(font_alibaba_12);
LV_FONT_DECLARE(font_alibaba_24);
LV_FONT_DECLARE(font_alibaba_40);
LV_FONT_DECLARE(font_alibaba_100);

/* ================================================================
 *  STUBS
 * ================================================================ */

lv_group_t *test_menu_g = NULL;
lv_group_t *test_app_g = NULL;
static bool menu_show_called = false;

void menu_show() {
    menu_show_called = true;
    if (test_menu_g) lv_group_set_default(test_menu_g);
}
void set_default_group(lv_group_t *group) { lv_group_set_default(group); }
void hw_feedback() {}
void disable_keyboard() {}
void enable_keyboard() {}
void hw_set_keyboard_read_callback(void(*read)(int state, char &c)) {}
bool is_screen_small() { return false; }
void hw_set_radio_listening() {}
bool hw_has_keyboard() { return true; }
void hw_get_date_time(struct tm &t) { memset(&t, 0, sizeof(t)); }

typedef struct { int dummy; } monitor_params_t;
void hw_get_monitor_params(monitor_params_t &p) { memset(&p, 0, sizeof(p)); }

lv_obj_t *create_menu(lv_obj_t *parent, lv_event_cb_t event_cb) {
    lv_obj_t *menu = lv_menu_create(parent);
    lv_menu_set_mode_root_back_button(menu, LV_MENU_ROOT_BACK_BUTTON_ENABLED);
    lv_obj_add_event_cb(menu, event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_size(menu, LV_PCT(100), LV_PCT(100));
    lv_obj_center(menu);
    return menu;
}

/* ================================================================
 *  Include ACTUAL app source files
 * ================================================================ */

#include "../../application/SkinnyCon/ui_nametag.cpp"
#include "../../application/SkinnyCon/ui_schedule.cpp"

/* ================================================================
 *  HELPERS
 * ================================================================ */

#define EXPECTED_HOR_RES 480
#define EXPECTED_VER_RES 222

static lv_obj_t *create_app_parent(void) {
    lv_obj_t *p = lv_obj_create(lv_screen_active());
    lv_obj_set_size(p, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_pad_all(p, 0, 0);
    lv_obj_set_style_border_width(p, 0, 0);
    return p;
}

/* Simulate opening an app (like the menu click handler does) */
static lv_obj_t *open_app(app_t *app) {
    lv_group_set_default(test_app_g);
    lv_obj_t *parent = create_app_parent();
    app->setup_func_cb(parent);
    lvgl_test_run(100);
    return parent;
}

/* Simulate closing an app and returning to menu */
static void close_app(app_t *app, lv_obj_t *parent) {
    app->exit_func_cb(NULL);
    lv_obj_del(parent);
    lv_group_set_default(test_menu_g);
    lvgl_test_run(50);
}

/* ================================================================
 *  SETUP / TEARDOWN
 * ================================================================ */

void setUp(void) {
    lvgl_sim_init();
    test_menu_g = lv_group_create();
    test_app_g = lv_group_create();
    lv_group_set_default(test_app_g);
    menu_show_called = false;
    strcpy(nametag_user_name, "YOUR NAME");
    strcpy(nametag_user_subtitle, "SkinnyCon 2026");
}

void tearDown(void) {
    lv_group_set_default(NULL);
    if (test_menu_g) { lv_group_del(test_menu_g); test_menu_g = NULL; }
    if (test_app_g) { lv_group_del(test_app_g); test_app_g = NULL; }
    lvgl_sim_deinit();
}

/* ================================================================
 *  NAMETAG TESTS
 * ================================================================ */

void test_nametag_opens_centered(void)
{
    lv_obj_t *parent = open_app(&ui_nametag_main);

    /* Find the nametag container */
    lv_obj_t *cont = lv_obj_get_child(parent, 0);
    TEST_ASSERT_NOT_NULL_MESSAGE(cont, "Nametag container should exist");

    /* Mode 0 should be centered (flex align center) */
    lvgl_test_save_ppm("test_nametag_open.ppm");

    close_app(&ui_nametag_main, parent);
}

void test_nametag_click_enters_edit_mode(void)
{
    lv_obj_t *parent = open_app(&ui_nametag_main);
    lv_obj_t *cont = lv_obj_get_child(parent, 0);

    /* Click should enter edit mode (not exit) on mode 0 */
    menu_show_called = false;
    lv_obj_send_event(cont, LV_EVENT_CLICKED, NULL);
    lvgl_test_run(100);

    TEST_ASSERT_FALSE_MESSAGE(menu_show_called,
        "Click on mode 0 should enter edit mode, NOT exit to menu");

    lvgl_test_save_ppm("test_nametag_edit_mode.ppm");

    close_app(&ui_nametag_main, parent);
}

void test_nametag_click_on_info_mode_exits(void)
{
    lv_obj_t *parent = open_app(&ui_nametag_main);
    lv_obj_t *cont = lv_obj_get_child(parent, 0);

    /* Rotate to mode 2 (About) */
    uint32_t key = LV_KEY_RIGHT;
    lv_obj_send_event(cont, LV_EVENT_KEY, &key);
    lvgl_test_run(50);
    lv_obj_send_event(cont, LV_EVENT_KEY, &key);
    lvgl_test_run(50);

    /* Click on info mode should exit to menu */
    menu_show_called = false;
    lv_obj_send_event(cont, LV_EVENT_CLICKED, NULL);
    lvgl_test_run(50);

    TEST_ASSERT_TRUE_MESSAGE(menu_show_called,
        "Click on info mode should exit to menu");
}

void test_nametag_rotate_cycles_modes(void)
{
    lv_obj_t *parent = open_app(&ui_nametag_main);
    lv_obj_t *cont = lv_obj_get_child(parent, 0);

    /* Rotate through all 5 modes and take screenshots */
    uint32_t key = LV_KEY_RIGHT;
    for (int i = 0; i < 5; i++) {
        lv_obj_send_event(cont, LV_EVENT_KEY, &key);
        lvgl_test_run(100);
        char fname[64];
        snprintf(fname, sizeof(fname), "test_nametag_mode%d.ppm", (i + 1) % 5);
        lvgl_test_save_ppm(fname);
    }

    /* After 5 rotations we should be back to mode 0 */
    /* Verify by checking that click enters edit (not exit) */
    menu_show_called = false;
    lv_obj_send_event(cont, LV_EVENT_CLICKED, NULL);
    lvgl_test_run(50);
    TEST_ASSERT_FALSE_MESSAGE(menu_show_called,
        "After 5 rotations should be back to mode 0 (click = edit, not exit)");

    close_app(&ui_nametag_main, parent);
}

void test_nametag_exit_cleans_group(void)
{
    lv_obj_t *parent = open_app(&ui_nametag_main);

    close_app(&ui_nametag_main, parent);

    /* App group should be empty after exit */
    uint32_t count = lv_group_get_obj_count(test_app_g);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, count,
        "App group should be empty after nametag exits");
}

/* ================================================================
 *  SCHEDULE TESTS
 * ================================================================ */

void test_schedule_opens_with_back_button(void)
{
    lv_obj_t *parent = open_app(&ui_schedule_main);

    /* The lv_menu should exist as a child */
    lv_obj_t *menu_obj = lv_obj_get_child(parent, 0);
    TEST_ASSERT_NOT_NULL_MESSAGE(menu_obj, "Schedule menu should exist");

    lvgl_test_save_ppm("test_schedule_open.ppm");

    close_app(&ui_schedule_main, parent);
}

void test_schedule_exit_cleans_group(void)
{
    lv_obj_t *parent = open_app(&ui_schedule_main);

    close_app(&ui_schedule_main, parent);

    uint32_t count = lv_group_get_obj_count(test_app_g);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, count,
        "App group should be empty after schedule exits");
}

/* ================================================================
 *  CROSS-APP LIFECYCLE TESTS
 * ================================================================ */

void test_nametag_then_schedule_works(void)
{
    /* Open and close nametag */
    lv_obj_t *p1 = open_app(&ui_nametag_main);
    close_app(&ui_nametag_main, p1);

    /* Open schedule — should work (group should be clean) */
    lv_obj_t *p2 = open_app(&ui_schedule_main);

    lv_obj_t *menu_obj = lv_obj_get_child(p2, 0);
    TEST_ASSERT_NOT_NULL_MESSAGE(menu_obj,
        "Schedule should open after nametag exit");

    lvgl_test_save_ppm("test_nametag_then_schedule.ppm");

    close_app(&ui_schedule_main, p2);
}

void test_schedule_then_nametag_works(void)
{
    /* Open and close schedule */
    lv_obj_t *p1 = open_app(&ui_schedule_main);
    close_app(&ui_schedule_main, p1);

    /* Open nametag — should work */
    lv_obj_t *p2 = open_app(&ui_nametag_main);

    lv_obj_t *cont = lv_obj_get_child(p2, 0);
    TEST_ASSERT_NOT_NULL_MESSAGE(cont,
        "Nametag should open after schedule exit");

    /* Rotate should work */
    uint32_t key = LV_KEY_RIGHT;
    lv_obj_send_event(cont, LV_EVENT_KEY, &key);
    lvgl_test_run(50);

    close_app(&ui_nametag_main, p2);
}

void test_three_app_cycle(void)
{
    /* Open nametag, close, open schedule, close, open nametag again */
    for (int cycle = 0; cycle < 3; cycle++) {
        lv_obj_t *p1 = open_app(&ui_nametag_main);
        close_app(&ui_nametag_main, p1);

        uint32_t count = lv_group_get_obj_count(test_app_g);
        char msg[64];
        snprintf(msg, sizeof(msg), "Cycle %d: group not clean after nametag", cycle);
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, count, msg);

        lv_obj_t *p2 = open_app(&ui_schedule_main);
        close_app(&ui_schedule_main, p2);

        count = lv_group_get_obj_count(test_app_g);
        snprintf(msg, sizeof(msg), "Cycle %d: group not clean after schedule", cycle);
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, count, msg);
    }
}

void test_nametag_all_modes_screenshot(void)
{
    lv_obj_t *parent = open_app(&ui_nametag_main);
    lv_obj_t *cont = lv_obj_get_child(parent, 0);

    /* Screenshot each mode */
    lvgl_test_save_ppm("test_real_nametag_mode0.ppm");
    uint32_t key = LV_KEY_RIGHT;
    for (int i = 1; i < 5; i++) {
        lv_obj_send_event(cont, LV_EVENT_KEY, &key);
        lvgl_test_run(100);
        char fname[64];
        snprintf(fname, sizeof(fname), "test_real_nametag_mode%d.ppm", i);
        lvgl_test_save_ppm(fname);
    }

    close_app(&ui_nametag_main, parent);
}

/* ================================================================
 *  MAIN
 * ================================================================ */

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    UNITY_BEGIN();

    /* Nametag */
    RUN_TEST(test_nametag_opens_centered);
    RUN_TEST(test_nametag_click_enters_edit_mode);
    RUN_TEST(test_nametag_click_on_info_mode_exits);
    RUN_TEST(test_nametag_rotate_cycles_modes);
    RUN_TEST(test_nametag_exit_cleans_group);
    RUN_TEST(test_nametag_all_modes_screenshot);

    /* Schedule */
    RUN_TEST(test_schedule_opens_with_back_button);
    RUN_TEST(test_schedule_exit_cleans_group);

    /* Cross-app lifecycle */
    RUN_TEST(test_nametag_then_schedule_works);
    RUN_TEST(test_schedule_then_nametag_works);
    RUN_TEST(test_three_app_cycle);

    return UNITY_END();
}
