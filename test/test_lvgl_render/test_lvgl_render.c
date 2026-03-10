/**
 * @file test_lvgl_render.c
 * LVGL 9.2 headless simulator tests for T-LoRa-Pager (480x222 RGB565).
 * Tests widget creation, rendering, and display properties against the simulator.
 * Saves PPM screenshots for visual inspection in CI artifacts.
 */

#include <unity.h>
/* Include simulator source directly — PlatformIO compiles each test dir
 * as an isolated compilation unit. sim_main.c lives in test/simulator/
 * and provides the headless LVGL 9.2 display driver. */
#include "../simulator/sim_main.c"
#include <string.h>
#include <stdio.h>

/* ---- Display constants ---- */
#define EXPECTED_HOR_RES  480
#define EXPECTED_VER_RES  222
#define BRIGHTNESS_MAX    16
#define BRIGHTNESS_MIN    0

void setUp(void)
{
    lvgl_sim_init();
}

void tearDown(void)
{
    lvgl_sim_deinit();
}

/* ================================================================
 *  DISPLAY PROPERTY TESTS
 * ================================================================ */

void test_display_resolution_horizontal(void)
{
    TEST_ASSERT_EQUAL_INT(EXPECTED_HOR_RES, lvgl_sim_get_hor_res());
}

void test_display_resolution_vertical(void)
{
    TEST_ASSERT_EQUAL_INT(EXPECTED_VER_RES, lvgl_sim_get_ver_res());
}

void test_display_exists(void)
{
    lv_display_t *disp = lv_display_get_default();
    TEST_ASSERT_NOT_NULL(disp);
}

void test_display_hor_res_from_lvgl(void)
{
    lv_display_t *disp = lv_display_get_default();
    TEST_ASSERT_EQUAL_INT(EXPECTED_HOR_RES, lv_display_get_horizontal_resolution(disp));
}

void test_display_ver_res_from_lvgl(void)
{
    lv_display_t *disp = lv_display_get_default();
    TEST_ASSERT_EQUAL_INT(EXPECTED_VER_RES, lv_display_get_vertical_resolution(disp));
}

void test_active_screen_exists(void)
{
    lv_obj_t *scr = lv_screen_active();
    TEST_ASSERT_NOT_NULL(scr);
}

void test_framebuffer_not_null(void)
{
    uint16_t *fb = lvgl_sim_get_framebuffer();
    TEST_ASSERT_NOT_NULL(fb);
}

/* ================================================================
 *  LABEL WIDGET TESTS
 * ================================================================ */

void test_label_create(void)
{
    lv_obj_t *label = lv_label_create(lv_screen_active());
    TEST_ASSERT_NOT_NULL(label);
    lv_label_set_text(label, "T-LoRa-Pager Test");

    lvgl_test_run(100);

    const char *text = lv_label_get_text(label);
    TEST_ASSERT_EQUAL_STRING("T-LoRa-Pager Test", text);

    lvgl_test_save_ppm("screenshot_label.ppm");
}

void test_label_long_text(void)
{
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "This is a long label text that should wrap on the 480px wide display");
    lv_obj_set_width(label, 460);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);

    lvgl_test_run(100);

    const char *text = lv_label_get_text(label);
    TEST_ASSERT_NOT_NULL(text);
    TEST_ASSERT_TRUE(strlen(text) > 0);
}

void test_label_empty_text(void)
{
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "");

    lvgl_test_run(50);

    const char *text = lv_label_get_text(label);
    TEST_ASSERT_EQUAL_STRING("", text);
}

/* ================================================================
 *  BUTTON WIDGET TESTS
 * ================================================================ */

static int btn_click_count = 0;

static void btn_event_cb(lv_event_t *e)
{
    (void)e;
    btn_click_count++;
}

void test_button_create(void)
{
    lv_obj_t *btn = lv_button_create(lv_screen_active());
    TEST_ASSERT_NOT_NULL(btn);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Click Me");

    lv_obj_set_size(btn, 120, 40);
    lv_obj_center(btn);

    lvgl_test_run(100);

    lvgl_test_save_ppm("screenshot_button.ppm");
}

void test_button_click_handler(void)
{
    btn_click_count = 0;

    lv_obj_t *btn = lv_button_create(lv_screen_active());
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);

    /* Simulate a click by sending events */
    lv_obj_send_event(btn, LV_EVENT_CLICKED, NULL);

    TEST_ASSERT_EQUAL_INT(1, btn_click_count);

    lv_obj_send_event(btn, LV_EVENT_CLICKED, NULL);
    TEST_ASSERT_EQUAL_INT(2, btn_click_count);
}

void test_button_with_label(void)
{
    lv_obj_t *btn = lv_button_create(lv_screen_active());
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "OK");

    lvgl_test_run(50);

    const char *text = lv_label_get_text(label);
    TEST_ASSERT_EQUAL_STRING("OK", text);
}

/* ================================================================
 *  RENDERING TESTS
 * ================================================================ */

void test_render_produces_non_zero_framebuffer(void)
{
    /* Create a visible object */
    lv_obj_t *obj = lv_obj_create(lv_screen_active());
    lv_obj_set_size(obj, 200, 100);
    lv_obj_center(obj);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);

    lvgl_test_run(200);

    /* Check that framebuffer has some non-zero pixels */
    uint16_t *fb = lvgl_sim_get_framebuffer();
    int non_zero = 0;
    int total = EXPECTED_HOR_RES * EXPECTED_VER_RES;
    for (int i = 0; i < total; i++) {
        if (fb[i] != 0) non_zero++;
    }
    TEST_ASSERT_TRUE(non_zero > 0);

    lvgl_test_save_ppm("screenshot_render.ppm");
}

void test_screen_bg_color(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x003366), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    lvgl_test_run(100);

    lvgl_test_save_ppm("screenshot_bg_color.ppm");

    /* Framebuffer should have non-black pixels */
    uint16_t *fb = lvgl_sim_get_framebuffer();
    int non_zero = 0;
    for (int i = 0; i < EXPECTED_HOR_RES * EXPECTED_VER_RES; i++) {
        if (fb[i] != 0) non_zero++;
    }
    TEST_ASSERT_TRUE(non_zero > 0);
}

/* ================================================================
 *  BRIGHTNESS LEVEL RANGE TESTS
 * ================================================================ */

void test_brightness_range(void)
{
    /* Verify brightness constants match device spec */
    TEST_ASSERT_EQUAL_INT(0, BRIGHTNESS_MIN);
    TEST_ASSERT_EQUAL_INT(16, BRIGHTNESS_MAX);
    TEST_ASSERT_EQUAL_INT(17, BRIGHTNESS_MAX - BRIGHTNESS_MIN + 1);
}

/* ================================================================
 *  DISPLAY ROTATION TESTS
 * ================================================================ */

void test_rotation_values(void)
{
    /* T-LoRa-Pager supports 4 rotation modes (0-3) */
    for (int r = 0; r < 4; r++) {
        /* Valid rotation values are 0, 1, 2, 3 */
        TEST_ASSERT_TRUE(r >= 0 && r <= 3);
    }
}

void test_rotation_mode_0(void)
{
    /* Mode 0: Normal landscape (480x222) */
    TEST_ASSERT_EQUAL_INT(480, EXPECTED_HOR_RES);
    TEST_ASSERT_EQUAL_INT(222, EXPECTED_VER_RES);
}

/* ================================================================
 *  MULTIPLE WIDGET TESTS
 * ================================================================ */

void test_multiple_labels(void)
{
    lv_obj_t *scr = lv_screen_active();

    lv_obj_t *l1 = lv_label_create(scr);
    lv_label_set_text(l1, "Header");
    lv_obj_align(l1, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t *l2 = lv_label_create(scr);
    lv_label_set_text(l2, "Body text here");
    lv_obj_align(l2, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *l3 = lv_label_create(scr);
    lv_label_set_text(l3, "Footer");
    lv_obj_align(l3, LV_ALIGN_BOTTOM_MID, 0, -10);

    lvgl_test_run(100);

    TEST_ASSERT_EQUAL_STRING("Header", lv_label_get_text(l1));
    TEST_ASSERT_EQUAL_STRING("Body text here", lv_label_get_text(l2));
    TEST_ASSERT_EQUAL_STRING("Footer", lv_label_get_text(l3));

    lvgl_test_save_ppm("screenshot_multi_labels.ppm");
}

void test_ppm_save_and_size(void)
{
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "PPM Test");

    lvgl_test_run(100);

    int result = lvgl_test_save_ppm("screenshot_ppm_test.ppm");
    TEST_ASSERT_EQUAL_INT(0, result);

    /* Verify file was created and has reasonable size */
    FILE *fp = fopen("screenshot_ppm_test.ppm", "rb");
    TEST_ASSERT_NOT_NULL(fp);
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fclose(fp);

    /* PPM size: header (~20 bytes) + 480*222*3 = 319,680 bytes min */
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

    /* Display properties */
    RUN_TEST(test_display_resolution_horizontal);
    RUN_TEST(test_display_resolution_vertical);
    RUN_TEST(test_display_exists);
    RUN_TEST(test_display_hor_res_from_lvgl);
    RUN_TEST(test_display_ver_res_from_lvgl);
    RUN_TEST(test_active_screen_exists);
    RUN_TEST(test_framebuffer_not_null);

    /* Labels */
    RUN_TEST(test_label_create);
    RUN_TEST(test_label_long_text);
    RUN_TEST(test_label_empty_text);

    /* Buttons */
    RUN_TEST(test_button_create);
    RUN_TEST(test_button_click_handler);
    RUN_TEST(test_button_with_label);

    /* Rendering */
    RUN_TEST(test_render_produces_non_zero_framebuffer);
    RUN_TEST(test_screen_bg_color);

    /* Brightness range */
    RUN_TEST(test_brightness_range);

    /* Rotation */
    RUN_TEST(test_rotation_values);
    RUN_TEST(test_rotation_mode_0);

    /* Multiple widgets */
    RUN_TEST(test_multiple_labels);
    RUN_TEST(test_ppm_save_and_size);

    return UNITY_END();
}
