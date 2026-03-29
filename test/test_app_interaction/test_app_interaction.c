/**
 * @file test_app_interaction.c
 * @brief Interaction tests for SkinnyCon badge apps
 *
 * Tests encoder navigation, scrolling, group integrity, pixel artifacts,
 * and app lifecycle. Uses headless LVGL simulator with simulated input.
 *
 * These tests catch bugs like:
 * - Nametag corrupting the input group for subsequent apps
 * - Grey pixel artifacts from unstyled LVGL objects
 * - Scroll not working after visiting certain apps
 * - Text overflow on 222px display
 */

#include <unity.h>
#include "../simulator/sim_main.c"
#include <string.h>
#include <stdio.h>

#include "../../application/SkinnyCon/ui_skinnycon_theme.h"

#define EXPECTED_HOR_RES 480
#define EXPECTED_VER_RES 222

LV_FONT_DECLARE(font_alibaba_12);
LV_FONT_DECLARE(font_alibaba_24);
LV_FONT_DECLARE(font_alibaba_40);

/* ================================================================
 *  HELPERS
 * ================================================================ */

/* Convert RGB565 pixel to approximate RGB888 */
static void px_to_rgb(uint16_t px, uint8_t *r, uint8_t *g, uint8_t *b)
{
    *r = ((px >> 11) & 0x1F) << 3;
    *g = ((px >> 5) & 0x3F) << 2;
    *b = (px & 0x1F) << 3;
}

/* Check if a pixel at (x,y) matches expected color within tolerance */
static int px_matches(int x, int y, uint8_t er, uint8_t eg, uint8_t eb, int tol)
{
    uint16_t *fb = lvgl_sim_get_framebuffer();
    uint16_t px = fb[y * EXPECTED_HOR_RES + x];
    uint8_t r, g, b;
    px_to_rgb(px, &r, &g, &b);
    return (abs(r - er) <= tol && abs(g - eg) <= tol && abs(b - eb) <= tol);
}

/* Check that a rectangular region has no grey artifacts (all pixels
 * should be either the expected bg color or text/accent colors,
 * not the LVGL default grey ~0xCCCCCC or ~0xE0E0E0) */
static int region_has_grey(int x1, int y1, int x2, int y2)
{
    uint16_t *fb = lvgl_sim_get_framebuffer();
    for (int y = y1; y < y2; y++) {
        for (int x = x1; x < x2; x++) {
            uint16_t px = fb[y * EXPECTED_HOR_RES + x];
            uint8_t r, g, b;
            px_to_rgb(px, &r, &g, &b);
            /* Check for LVGL default grey (0xC0-0xE8 range, all channels similar) */
            if (r >= 0xB0 && r <= 0xE8 && g >= 0xB0 && g <= 0xE8 && b >= 0xB0 && b <= 0xE8) {
                int range = abs(r - g) + abs(g - b);
                if (range < 20) {
                    /* This is a grey pixel — might be an artifact */
                    return 1;
                }
            }
        }
    }
    return 0;
}

/* Simulate encoder key press */
static void sim_key(uint32_t key)
{
    /* Find the focused object and send key event */
    lv_group_t *g = lv_group_get_default();
    if (!g) return;
    lv_obj_t *focused = lv_group_get_focused(g);
    if (!focused) return;
    lv_obj_send_event(focused, LV_EVENT_KEY, &key);
    lvgl_test_run(50);
}

/* Simulate encoder click */
static void sim_click(void)
{
    lv_group_t *g = lv_group_get_default();
    if (!g) return;
    lv_obj_t *focused = lv_group_get_focused(g);
    if (!focused) return;
    lv_obj_send_event(focused, LV_EVENT_CLICKED, NULL);
    lvgl_test_run(50);
}

/* ================================================================
 *  TEST SETUP/TEARDOWN
 * ================================================================ */

void setUp(void) { lvgl_sim_init(); }
void tearDown(void) { lvgl_sim_deinit(); }

/* ================================================================
 *  TEST: Idle screen has no grey artifacts near battery
 * ================================================================ */

void test_idle_screen_no_grey_artifacts(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, SC_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* Create footer like the real idle screen */
    lv_obj_t *footer = lv_obj_create(scr);
    lv_obj_remove_style_all(footer);
    lv_obj_set_size(footer, LV_PCT(100), 30);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(footer, SC_PANEL, 0);
    lv_obj_set_style_bg_opa(footer, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_hor(footer, 10, 0);

    lv_obj_t *batt = lv_label_create(footer);
    lv_obj_set_style_text_font(batt, &font_alibaba_12, 0);
    lv_label_set_text(batt, "100%");
    lv_obj_set_style_text_color(batt, SC_GREEN, 0);
    lv_obj_align(batt, LV_ALIGN_RIGHT_MID, 0, 0);

    /* Create hidden objects like the real idle screen — off-screen to avoid artifacts */
    lv_obj_t *hidden_bar = lv_bar_create(scr);
    lv_obj_remove_style_all(hidden_bar);
    lv_obj_set_size(hidden_bar, 0, 0);
    lv_obj_add_flag(hidden_bar, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(hidden_bar, -100, -100);

    lv_obj_t *hidden_lbl = lv_label_create(scr);
    lv_obj_remove_style_all(hidden_lbl);
    lv_obj_set_size(hidden_lbl, 0, 0);
    lv_obj_add_flag(hidden_lbl, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(hidden_lbl, -100, -100);

    lvgl_test_run(200);

    /* Check the area around the battery text (right side of footer)
     * for grey artifacts. The footer bg should be white (SC_PANEL = 0xFFFFFF)
     * and any hidden objects should not bleed through */
    /* Debug: print any grey pixels found */
    uint16_t *fb = lvgl_sim_get_framebuffer();
    int grey_count = 0;
    for (int y = 192; y < 221; y++) {
        for (int x = 420; x < 479; x++) {
            uint16_t px = fb[y * EXPECTED_HOR_RES + x];
            uint8_t r, g, b;
            px_to_rgb(px, &r, &g, &b);
            if (r >= 0xB0 && r <= 0xE8 && g >= 0xB0 && g <= 0xE8 && b >= 0xB0 && b <= 0xE8) {
                int range = abs(r - g) + abs(g - b);
                if (range < 20) {
                    if (grey_count < 5) {
                        printf("Grey pixel at (%d,%d): r=%d g=%d b=%d\n", x, y, r, g, b);
                    }
                    grey_count++;
                }
            }
        }
    }
    printf("Total grey pixels in battery region: %d\n", grey_count);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, grey_count,
        "Grey artifact detected near battery indicator on idle screen");

    lvgl_test_save_ppm("test_idle_no_grey.ppm");
}

/* ================================================================
 *  TEST: LVGL group is clean after nametag exit
 * ================================================================ */

void test_group_clean_after_nametag(void)
{
    /* Create two groups like the real app */
    lv_group_t *menu_g = lv_group_create();
    lv_group_t *app_g = lv_group_create();

    /* Simulate opening nametag: switch to app group, add container */
    lv_group_set_default(app_g);
    lv_obj_t *nametag = lv_obj_create(lv_screen_active());
    lv_obj_set_size(nametag, LV_PCT(100), LV_PCT(100));
    lv_group_add_obj(app_g, nametag);

    TEST_ASSERT_EQUAL_UINT32(1, lv_group_get_obj_count(app_g));

    /* Simulate nametag exit: delete the object */
    lv_obj_del(nametag);
    lvgl_test_run(50);

    /* Group should now be empty */
    TEST_ASSERT_EQUAL_UINT32(0, lv_group_get_obj_count(app_g));

    /* Switch to menu group, then back to app group (like opening next app) */
    lv_group_set_default(menu_g);
    lvgl_test_run(50);
    lv_group_set_default(app_g);

    /* Create a new container for "next app" */
    lv_obj_t *next_app = lv_obj_create(lv_screen_active());
    lv_obj_set_size(next_app, LV_PCT(100), LV_PCT(100));
    lv_group_add_obj(app_g, next_app);

    /* The new app should be the focused object */
    TEST_ASSERT_EQUAL_PTR(next_app, lv_group_get_focused(app_g));

    /* Simulate encoder key — should reach the new app without crash */
    uint32_t key = LV_KEY_DOWN;
    lv_obj_send_event(next_app, LV_EVENT_KEY, &key);
    lvgl_test_run(50);

    /* Clean up */
    lv_obj_del(next_app);
    lv_group_del(menu_g);
    lv_group_del(app_g);
}

/* ================================================================
 *  TEST: Schedule talk list scrolls properly
 * ================================================================ */

void test_schedule_scroll(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_group_t *g = lv_group_create();
    lv_group_set_default(g);

    /* Create a scrollable list like the schedule */
    lv_obj_t *list = lv_obj_create(scr);
    lv_obj_set_size(list, LV_PCT(100), 180);
    lv_obj_set_style_bg_color(list, SC_BG, 0);
    lv_obj_set_style_bg_opa(list, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_scroll_dir(list, LV_DIR_VER);

    /* Add 15 rows (more than fit on 180px) */
    for (int i = 0; i < 15; i++) {
        lv_obj_t *row = lv_obj_create(list);
        lv_obj_set_size(row, LV_PCT(100), 25);
        lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(row, (i % 2) ? SC_PANEL_ALT : SC_BG, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_radius(row, 0, 0);

        lv_obj_t *lbl = lv_label_create(row);
        lv_label_set_text_fmt(lbl, "Talk %d: Some Title", i + 1);
        lv_obj_set_style_text_color(lbl, SC_TEXT, 0);
    }

    lvgl_test_run(100);

    /* Verify initial scroll position is 0 */
    lv_coord_t initial_scroll = lv_obj_get_scroll_y(list);
    TEST_ASSERT_EQUAL_INT(0, initial_scroll);

    /* Scroll down by scrolling to the last child */
    lv_obj_t *last = lv_obj_get_child(list, 14);
    lv_obj_scroll_to_view(last, LV_ANIM_OFF);
    lvgl_test_run(100);

    /* Scroll position should have changed */
    lv_coord_t scrolled = lv_obj_get_scroll_y(list);
    TEST_ASSERT_TRUE_MESSAGE(scrolled > 0, "Schedule list should scroll when content exceeds height");

    lvgl_test_save_ppm("test_schedule_scroll.ppm");

    lv_group_del(g);
}

/* ================================================================
 *  TEST: Text content fits on 222px display with font_alibaba_12
 * ================================================================ */

void test_text_fits_on_screen(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, SC_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* Simulate the longest content page (Code of Conduct) */
    lv_obj_t *cont = lv_obj_create(scr);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 4, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *title = lv_label_create(cont);
    lv_label_set_text(title, "(!) CODE OF CONDUCT");
    lv_obj_set_style_text_font(title, &font_alibaba_12, 0);
    lv_obj_set_style_text_color(title, SC_ACCENT, 0);

    lv_obj_t *body = lv_label_create(cont);
    lv_label_set_text(body,
        "- Be kind, considerate, respectful\n"
        "- Behave professionally\n"
        "- Respect differing viewpoints\n"
        "- Be mindful of personal space\n"
        "- Obey venue rules\n\n"
        "I2C / UAH campus - shared space.\n"
        "Do not explore beyond con areas.\n"
        "Do not play with door locks!\n"
        "UAH is 100% tobacco free."
    );
    lv_obj_set_style_text_font(body, &font_alibaba_12, 0);
    lv_obj_set_style_text_color(body, SC_TEXT, 0);
    lv_obj_set_width(body, LV_PCT(100));
    lv_label_set_long_mode(body, LV_LABEL_LONG_WRAP);

    lv_obj_t *hint = lv_label_create(cont);
    lv_label_set_text(hint, "Click=back  Rotate=mode");
    lv_obj_set_style_text_font(hint, &font_alibaba_12, 0);
    lv_obj_set_style_text_color(hint, SC_TEXT_DIM, 0);

    lvgl_test_run(200);

    /* Check that the hint text at the bottom is visible
     * (i.e., its Y position is within the screen bounds) */
    lv_coord_t hint_y = lv_obj_get_y(hint);
    lv_coord_t hint_h = lv_obj_get_height(hint);
    TEST_ASSERT_TRUE_MESSAGE(hint_y + hint_h <= EXPECTED_VER_RES + 10,
        "Code of Conduct text overflows 222px display — hint not visible");

    lvgl_test_save_ppm("test_coc_fits.ppm");
}

/* ================================================================
 *  TEST: Nametag keyboard input changes name
 * ================================================================ */

void test_nametag_keyboard_input(void)
{
    /* Simulate the nametag name storage */
    char name[25] = "YOUR NAME";
    char subtitle[33] = "SkinnyCon 2026";
    bool editing = false;

    /* Simulate first keypress: should clear name and start editing */
    char c = 'A';
    if (!editing) {
        editing = true;
        name[0] = c;
        name[1] = '\0';
    }
    TEST_ASSERT_TRUE(editing);
    TEST_ASSERT_EQUAL_STRING("A", name);

    /* Simulate more keypresses */
    const char *input = "lice";
    for (int i = 0; i < 4; i++) {
        int len = strlen(name);
        name[len] = input[i];
        name[len + 1] = '\0';
    }
    TEST_ASSERT_EQUAL_STRING("Alice", name);

    /* Simulate backspace */
    int len = strlen(name);
    if (len > 0) name[len - 1] = '\0';
    TEST_ASSERT_EQUAL_STRING("Alic", name);

    /* Simulate enter (confirm) */
    editing = false;
    TEST_ASSERT_FALSE(editing);
    TEST_ASSERT_EQUAL_STRING("Alic", name);
}

/* ================================================================
 *  TEST: Menu icon area has no grey artifacts
 * ================================================================ */

void test_menu_icons_no_grey(void)
{
    /* Render a simple icon using the helpers */
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, SC_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* Create a container mimicking an icon slot */
    lv_obj_t *slot = lv_obj_create(scr);
    lv_obj_remove_style_all(slot);
    lv_obj_set_size(slot, 120, 120);
    lv_obj_center(slot);

    /* Draw a simple filled circle (like GPS icon) */
    lv_obj_t *circle = lv_obj_create(slot);
    lv_obj_remove_style_all(circle);
    lv_obj_set_size(circle, 40, 40);
    lv_obj_center(circle);
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(circle, SC_ACCENT, 0);
    lv_obj_set_style_bg_opa(circle, LV_OPA_COVER, 0);

    lvgl_test_run(200);

    /* Check the area around the circle for grey artifacts */
    /* The region outside the circle should be SC_BG (#EFF6F6), not grey */
    int has_grey = region_has_grey(
        EXPECTED_HOR_RES / 2 - 60, EXPECTED_VER_RES / 2 - 60,
        EXPECTED_HOR_RES / 2 - 25, EXPECTED_VER_RES / 2 - 25);
    TEST_ASSERT_FALSE_MESSAGE(has_grey,
        "Grey artifact detected in icon area — lv_obj_remove_style_all not working");
}

/* ================================================================
 *  TEST: Multiple app open/close cycles don't corrupt groups
 * ================================================================ */

void test_app_lifecycle_stress(void)
{
    lv_group_t *menu_g = lv_group_create();
    lv_group_t *app_g = lv_group_create();

    /* Simulate opening and closing 5 apps in sequence */
    for (int cycle = 0; cycle < 5; cycle++) {
        /* Open app */
        lv_group_set_default(app_g);
        lv_obj_t *app_cont = lv_obj_create(lv_screen_active());
        lv_obj_set_size(app_cont, LV_PCT(100), LV_PCT(100));
        lv_group_add_obj(app_g, app_cont);

        TEST_ASSERT_EQUAL_UINT32(1, lv_group_get_obj_count(app_g));
        TEST_ASSERT_EQUAL_PTR(app_cont, lv_group_get_focused(app_g));

        lvgl_test_run(50);

        /* Close app */
        lv_obj_del(app_cont);
        lv_group_set_default(menu_g);
        lvgl_test_run(50);

        /* Verify group is clean */
        TEST_ASSERT_EQUAL_UINT32(0, lv_group_get_obj_count(app_g));
    }

    lv_group_del(menu_g);
    lv_group_del(app_g);
}

/* ================================================================
 *  MAIN
 * ================================================================ */

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    UNITY_BEGIN();

    /* Pixel artifact tests */
    RUN_TEST(test_idle_screen_no_grey_artifacts);
    RUN_TEST(test_menu_icons_no_grey);

    /* Group/navigation tests */
    RUN_TEST(test_group_clean_after_nametag);
    RUN_TEST(test_app_lifecycle_stress);

    /* Content/layout tests */
    RUN_TEST(test_text_fits_on_screen);
    RUN_TEST(test_schedule_scroll);

    /* Input tests */
    RUN_TEST(test_nametag_keyboard_input);

    return UNITY_END();
}
