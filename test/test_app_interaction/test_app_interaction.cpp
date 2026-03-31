/**
 * @file test_app_interaction.cpp
 * @brief App lifecycle tests using real nametag + schedule source code
 */

extern "C" {
#include "../simulator/sim_main.c"
}

#include <unity.h>
#include <string.h>
#include <stdio.h>

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

/* Stubs */
lv_group_t *test_menu_g = NULL;
lv_group_t *test_app_g = NULL;
static bool menu_show_called = false;

void menu_show() { menu_show_called = true; }
void set_default_group(lv_group_t *g) { lv_group_set_default(g); }
void hw_feedback() {}
void disable_keyboard() {}
void enable_keyboard() {}
void hw_set_keyboard_read_callback(void(*r)(int, char &)) {}
bool is_screen_small() { return false; }
void hw_set_radio_listening() {}
bool hw_has_keyboard() { return true; }
typedef struct { int d; } monitor_params_t;
void hw_get_monitor_params(monitor_params_t &p) {}
void hw_get_date_time(struct tm &t) { memset(&t, 0, sizeof(t)); }

lv_obj_t *create_menu(lv_obj_t *parent, lv_event_cb_t cb) {
    lv_obj_t *m = lv_menu_create(parent);
    lv_menu_set_mode_root_back_button(m, LV_MENU_ROOT_BACK_BUTTON_ENABLED);
    lv_obj_add_event_cb(m, cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_size(m, LV_PCT(100), LV_PCT(100));
    lv_obj_center(m);
    return m;
}

/* Include real apps */
#include "../../application/SkinnyCon/ui_nametag.cpp"
#include "../../application/SkinnyCon/ui_schedule.cpp"

/* Helpers */
static lv_obj_t *open_app(app_t *app) {
    lv_group_set_default(test_app_g);
    lv_obj_t *p = lv_obj_create(lv_screen_active());
    lv_obj_set_size(p, LV_PCT(100), LV_PCT(100));
    app->setup_func_cb(p);
    lvgl_test_run(100);
    return p;
}

static void close_app(app_t *app, lv_obj_t *p) {
    app->exit_func_cb(NULL);
    lv_obj_del(p);
    lvgl_test_run(50);
}

void setUp(void) {
    lvgl_sim_init();
    test_menu_g = lv_group_create();
    test_app_g = lv_group_create();
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

/* Tests */

void test_nametag_opens(void) {
    lv_obj_t *p = open_app(&ui_nametag_main);
    TEST_ASSERT_NOT_NULL(lv_obj_get_child(p, 0));
    lvgl_test_save_ppm("test_nametag_open.ppm");
    close_app(&ui_nametag_main, p);
}

void test_schedule_opens(void) {
    lv_obj_t *p = open_app(&ui_schedule_main);
    TEST_ASSERT_NOT_NULL(lv_obj_get_child(p, 0));
    lvgl_test_save_ppm("test_schedule_open.ppm");
    close_app(&ui_schedule_main, p);
}

void test_nametag_then_schedule(void) {
    lv_obj_t *p1 = open_app(&ui_nametag_main);
    close_app(&ui_nametag_main, p1);
    lv_obj_t *p2 = open_app(&ui_schedule_main);
    TEST_ASSERT_NOT_NULL_MESSAGE(lv_obj_get_child(p2, 0),
        "Schedule should work after nametag");
    close_app(&ui_schedule_main, p2);
}

void test_schedule_then_nametag(void) {
    lv_obj_t *p1 = open_app(&ui_schedule_main);
    close_app(&ui_schedule_main, p1);
    lv_obj_t *p2 = open_app(&ui_nametag_main);
    TEST_ASSERT_NOT_NULL_MESSAGE(lv_obj_get_child(p2, 0),
        "Nametag should work after schedule");
    close_app(&ui_nametag_main, p2);
}

void test_three_app_cycle(void) {
    for (int i = 0; i < 3; i++) {
        lv_obj_t *p1 = open_app(&ui_nametag_main);
        close_app(&ui_nametag_main, p1);
        lv_obj_t *p2 = open_app(&ui_schedule_main);
        close_app(&ui_schedule_main, p2);
    }
    TEST_ASSERT_TRUE(true);
}

/* Verify nametag group only has main page items initially,
   not hidden subpage rows */
void test_nametag_group_main_page(void) {
    lv_obj_t *p = open_app(&ui_nametag_main);
    lv_group_t *g = lv_group_get_default();
    TEST_ASSERT_NOT_NULL(g);

    /* Main page should have: name_cont + name_ta + edit_name + about + coc + badge = 6 */
    uint32_t count = lv_group_get_obj_count(g);
    printf("[TEST] Main page group count: %d\n", (int)count);
    TEST_ASSERT_EQUAL_MESSAGE(6, count,
        "Main page group should have exactly 6 items (name_cont + ta + 4 menu conts)");

    close_app(&ui_nametag_main, p);
}

/* Simulate clicking About and verify group swaps to about rows + back btn */
void test_nametag_group_about_subpage(void) {
    lv_obj_t *p = open_app(&ui_nametag_main);
    lv_group_t *g = lv_group_get_default();

    /* Simulate clicking "About SkinnyCon" (main_conts[1]) */
    lv_obj_send_event(main_conts[1], LV_EVENT_CLICKED, NULL);
    /* Process the deferred timer */
    lvgl_test_run(100);

    uint32_t count = lv_group_get_obj_count(g);
    printf("[TEST] About subpage group count: %d\n", (int)count);
    /* About page has 7 text rows + back button = 8 */
    TEST_ASSERT_EQUAL_MESSAGE(8, count,
        "About subpage group should have 8 items (back btn + 7 text rows)");

    close_app(&ui_nametag_main, p);
}

/* Simulate clicking CoC and verify group */
void test_nametag_group_coc_subpage(void) {
    lv_obj_t *p = open_app(&ui_nametag_main);
    lv_group_t *g = lv_group_get_default();

    /* Simulate clicking "Code of Conduct" (main_conts[2]) */
    lv_obj_send_event(main_conts[2], LV_EVENT_CLICKED, NULL);
    lvgl_test_run(100);

    uint32_t count = lv_group_get_obj_count(g);
    printf("[TEST] CoC subpage group count: %d\n", (int)count);
    /* CoC page has 9 text rows + back button = 10 */
    TEST_ASSERT_EQUAL_MESSAGE(10, count,
        "CoC subpage group should have 10 items (back btn + 9 text rows)");

    close_app(&ui_nametag_main, p);
}

/* Simulate clicking Badge Info and verify group */
void test_nametag_group_badge_subpage(void) {
    lv_obj_t *p = open_app(&ui_nametag_main);
    lv_group_t *g = lv_group_get_default();

    /* Simulate clicking "Badge Info" (main_conts[3]) */
    lv_obj_send_event(main_conts[3], LV_EVENT_CLICKED, NULL);
    lvgl_test_run(100);

    uint32_t count = lv_group_get_obj_count(g);
    printf("[TEST] Badge subpage group count: %d\n", (int)count);
    /* Badge page has 8 text rows + back button = 9 */
    TEST_ASSERT_EQUAL_MESSAGE(9, count,
        "Badge subpage group should have 9 items (back btn + 8 text rows)");

    close_app(&ui_nametag_main, p);
}

/* Verify group restores to main page after going back from a subpage */
void test_nametag_group_back_from_subpage(void) {
    lv_obj_t *p = open_app(&ui_nametag_main);
    lv_group_t *g = lv_group_get_default();

    /* Enter About subpage */
    lv_obj_send_event(main_conts[1], LV_EVENT_CLICKED, NULL);
    lvgl_test_run(100);
    TEST_ASSERT_EQUAL(8, lv_group_get_obj_count(g));

    /* Click the back button to go back to main */
    lv_obj_t *back_btn = lv_menu_get_main_header_back_button(menu);
    TEST_ASSERT_NOT_NULL(back_btn);
    lv_obj_send_event(back_btn, LV_EVENT_CLICKED, NULL);
    lvgl_test_run(100);

    /* Should be back to main page items */
    uint32_t count = lv_group_get_obj_count(g);
    printf("[TEST] After back, group count: %d\n", (int)count);
    /* Main page: back_btn + ta + 4 menu conts = 6 */
    TEST_ASSERT_EQUAL_MESSAGE(6, count,
        "After going back, group should have 6 items (back btn + ta + 4 menu conts)");

    close_app(&ui_nametag_main, p);
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_nametag_opens);
    RUN_TEST(test_schedule_opens);
    RUN_TEST(test_nametag_then_schedule);
    RUN_TEST(test_schedule_then_nametag);
    RUN_TEST(test_three_app_cycle);
    RUN_TEST(test_nametag_group_main_page);
    RUN_TEST(test_nametag_group_about_subpage);
    RUN_TEST(test_nametag_group_coc_subpage);
    RUN_TEST(test_nametag_group_badge_subpage);
    RUN_TEST(test_nametag_group_back_from_subpage);
    return UNITY_END();
}
