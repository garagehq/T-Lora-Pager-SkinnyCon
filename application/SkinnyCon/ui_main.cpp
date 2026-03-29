/**
 * @file      ui.cpp
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2025  ShenZhen XinYuan Electronic Technology Co., Ltd
 * @date      2025-01-04
 *
 */
#include "ui_define.h"
#include "ui_skinnycon_theme.h"

LV_IMG_DECLARE(img_microphone);
LV_IMG_DECLARE(img_ir_remote);
LV_IMG_DECLARE(img_music);
LV_IMG_DECLARE(img_wifi);
LV_IMG_DECLARE(img_configuration);
LV_IMG_DECLARE(img_radio);
LV_IMG_DECLARE(img_gps);
LV_IMG_DECLARE(img_power);
LV_IMG_DECLARE(img_monitoring);
LV_IMG_DECLARE(img_cry);
LV_IMG_DECLARE(img_keyboard);
LV_IMG_DECLARE(img_gyroscope);
LV_IMG_DECLARE(img_msgchat);
LV_IMG_DECLARE(img_bluetooth);
LV_IMG_DECLARE(img_test);
LV_IMG_DECLARE(img_dog);
LV_IMG_DECLARE(img_background);
LV_IMG_DECLARE(img_battery);
LV_IMG_DECLARE(img_MotionRecognition);
LV_IMG_DECLARE(img_MotorLearning);
LV_IMG_DECLARE(img_camera);
LV_IMG_DECLARE(img_si4735);
LV_IMG_DECLARE(img_track);
LV_IMG_DECLARE(img_compass);
LV_IMG_DECLARE(img_nfc);
LV_IMG_DECLARE(img_batter_low);

LV_IMG_DECLARE(img_background2);

LV_FONT_DECLARE(font_alibaba_12);
LV_FONT_DECLARE(font_alibaba_24);
LV_FONT_DECLARE(font_alibaba_40);
LV_FONT_DECLARE(font_alibaba_60);
LV_FONT_DECLARE(font_alibaba_100);

#define DEVICE_CAN_SLEEP                (LV_OBJ_FLAG_USER_1)
#define SCREEN_TIMEOUT 10000

lv_obj_t *main_screen;
lv_obj_t *menu_panel;
lv_group_t *menu_g, *app_g;
static lv_timer_t *clock_timer;
static lv_obj_t *clock_page;
static lv_timer_t *disp_timer = NULL;
static lv_timer_t *dev_timer = NULL;
static uint32_t disp_time_ms = 0;

typedef struct {
    lv_obj_t *hour;
    lv_obj_t *minute;
    lv_obj_t *date;
    lv_obj_t *seg;
    lv_obj_t *battery_bar;
    lv_obj_t *battery_label;
} clock_label_t;;

static clock_label_t clock_label;

#if LVGL_VERSION_MAJOR == 9
static uint32_t name_change_id;
#endif


static lv_obj_t *desc_label;
static RTC_DATA_ATTR uint8_t brightness_level = 0;
static RTC_DATA_ATTR uint8_t keyboard_level = 0;

void set_low_power_mode_flag(bool enable)
{
    if (enable) {
        lv_obj_add_flag(main_screen, DEVICE_CAN_SLEEP);
    } else {
        lv_obj_remove_flag(main_screen, DEVICE_CAN_SLEEP);
    }
}

bool get_enter_low_power_flag()
{
    bool rlst = lv_obj_has_flag(main_screen, DEVICE_CAN_SLEEP);
    return rlst;
}

void menu_show()
{
    set_default_group(menu_g);
    lv_tileview_set_tile_by_index(main_screen, 0, 0, LV_ANIM_ON);
    lv_timer_resume(disp_timer);
    lv_disp_trig_activity(NULL);
    hw_feedback();
}

void menu_hidden()
{
    lv_tileview_set_tile_by_index(main_screen, 0, 1, LV_ANIM_ON);
    lv_timer_pause(disp_timer);
}

bool isinMenu()
{
    return !lv_obj_has_flag(main_screen, LV_OBJ_FLAG_HIDDEN);
}

void set_default_group(lv_group_t *group)
{
    lv_indev_t *cur_drv = NULL;
    for (;;) {
        cur_drv = lv_indev_get_next(cur_drv);
        if (!cur_drv) {
            break;
        }
        if (lv_indev_get_type(cur_drv) == LV_INDEV_TYPE_KEYPAD) {
            lv_indev_set_group(cur_drv, group);
        }
        if (lv_indev_get_type(cur_drv)  == LV_INDEV_TYPE_ENCODER) {
            lv_indev_set_group(cur_drv, group);
        }
        if (lv_indev_get_type(cur_drv)  == LV_INDEV_TYPE_POINTER) {
            lv_indev_set_group(cur_drv, group);
        }
    }
    lv_group_set_default(group);
}


static void btn_event_cb(lv_event_t *e)
{
    lv_event_code_t c = lv_event_get_code(e);
    char *text = (char *)lv_event_get_user_data(e);
    if (c == LV_EVENT_FOCUSED) {
#if LVGL_VERSION_MAJOR == 9
        lv_obj_send_event(desc_label, (lv_event_code_t )name_change_id, text);
#else
        lv_msg_send(MSG_MENU_NAME_CHANGED, text);
#endif
    }
}

typedef void (*icon_draw_fn)(lv_obj_t *parent);

static lv_obj_t *create_app_btn(lv_obj_t *parent, const char *name, app_t *app_fun)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_coord_t w = 150;
    lv_coord_t h = LV_PCT(100);

    lv_obj_set_size(btn, w, h);
    lv_obj_set_style_bg_opa(btn, LV_OPA_0, 0);
    lv_obj_set_style_outline_color(btn, SC_ACCENT, LV_STATE_FOCUS_KEY);
    lv_obj_set_style_shadow_width(btn, 30, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(btn, SC_BORDER, LV_PART_MAIN);
    uint32_t phy_hor_res = lv_display_get_physical_horizontal_resolution(NULL);
    if (phy_hor_res < 320) {
        lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    }
    lv_obj_set_user_data(btn, (void *)name);

    /* Text change event callback */
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_FOCUSED, (void *)name);

    /* Click to select event callback */
    lv_obj_add_event_cb(btn, [](lv_event_t *e) {
        lv_event_code_t c = lv_event_get_code(e);
        app_t *func_cb = (app_t *)lv_event_get_user_data(e);
        lv_obj_t *parent = lv_obj_get_child(main_screen, 1);
        if (lv_obj_has_flag(main_screen, LV_OBJ_FLAG_HIDDEN)) {
            return;
        }
        if (c == LV_EVENT_CLICKED) {
            set_default_group(app_g);
            hw_feedback();
            if (func_cb->setup_func_cb) {
                (*func_cb->setup_func_cb)(parent);
            }
            menu_hidden();
        }
    },
    LV_EVENT_CLICKED, app_fun);

    return btn;
}

static void create_app(lv_obj_t *parent, const char *name, const lv_img_dsc_t *img, app_t *app_fun)
{
    lv_obj_t *btn = create_app_btn(parent, name, app_fun);
    if (img != NULL) {
        lv_obj_t *icon = lv_image_create(btn);
        lv_image_set_src(icon, img);
        lv_obj_center(icon);
    }
}

static void create_app_drawn(lv_obj_t *parent, const char *name, icon_draw_fn draw_fn, app_t *app_fun)
{
    lv_obj_t *btn = create_app_btn(parent, name, app_fun);
    if (draw_fn) draw_fn(btn);
}

/* ── SkinnyCon logo helper ──────────────────────────────────────── */

/**
 * @brief Draw the SKINNYCON logo with teal circle replacing the O.
 * @param parent Parent object to draw into
 * @param font Font for the text (determines circle size)
 * @param circle_size Diameter of the teal circle (match to cap height)
 * @return The flex-row container holding the logo
 */
static lv_obj_t *draw_skinnycon_logo(lv_obj_t *parent, const lv_font_t *font, int circle_size)
{
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row, LV_OPA_0, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_style_pad_column(row, 0, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *left = lv_label_create(row);
    lv_label_set_text(left, "SKINNYC");
    lv_obj_set_style_text_font(left, font, 0);
    lv_obj_set_style_text_color(left, SC_TEXT, 0);

    lv_obj_t *circle = lv_obj_create(row);
    lv_obj_set_size(circle, circle_size, circle_size);
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(circle, SC_TEAL, 0);
    lv_obj_set_style_bg_opa(circle, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(circle, 0, 0);
    /* Overlap with adjacent letters like the real logo */
    lv_obj_set_style_margin_left(circle, -2, 0);
    lv_obj_set_style_margin_right(circle, -2, 0);

    lv_obj_t *right = lv_label_create(row);
    lv_label_set_text(right, "N");
    lv_obj_set_style_text_font(right, font, 0);
    lv_obj_set_style_text_color(right, SC_TEXT, 0);

    return row;
}

/* ── LVGL-drawn menu icons ─────────────────────────────────────── */

/* Helper: create a clean shape with no default LVGL styling artifacts */
static lv_obj_t *icon_shape(lv_obj_t *parent, int w, int h)
{
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_set_size(o, w, h);
    lv_obj_set_style_bg_opa(o, LV_OPA_0, 0);
    lv_obj_set_style_border_width(o, 0, 0);
    lv_obj_set_style_pad_all(o, 0, 0);
    lv_obj_set_style_radius(o, 0, 0);
    lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE);
    return o;
}

/* Helper: create a filled shape */
static lv_obj_t *icon_fill(lv_obj_t *parent, int w, int h, lv_color_t color, int radius)
{
    lv_obj_t *o = icon_shape(parent, w, h);
    lv_obj_set_style_bg_color(o, color, 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(o, radius, 0);
    return o;
}

/* Helper: create a ring (border-only circle/shape) */
static lv_obj_t *icon_ring(lv_obj_t *parent, int w, int h, lv_color_t color, int bw, int radius)
{
    lv_obj_t *o = icon_shape(parent, w, h);
    lv_obj_set_style_border_color(o, color, 0);
    lv_obj_set_style_border_width(o, bw, 0);
    lv_obj_set_style_radius(o, radius, 0);
    return o;
}

/* Nametag icon: vertical ID badge with person silhouette */
static void draw_icon_nametag(lv_obj_t *parent)
{
    LV_FONT_DECLARE(font_alibaba_12);

    lv_obj_t *badge = lv_obj_create(parent);
    lv_obj_set_size(badge, 44, 58);
    lv_obj_center(badge);
    lv_obj_set_style_bg_color(badge, SC_PANEL, 0);
    lv_obj_set_style_bg_opa(badge, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(badge, SC_TEXT, 0);
    lv_obj_set_style_border_width(badge, 2, 0);
    lv_obj_set_style_radius(badge, 4, 0);
    lv_obj_set_style_pad_all(badge, 0, 0);
    lv_obj_remove_flag(badge, LV_OBJ_FLAG_SCROLLABLE);

    /* Orange header band */
    lv_obj_t *band = lv_obj_create(badge);
    lv_obj_set_size(band, LV_PCT(100), 10);
    lv_obj_align(band, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(band, SC_ACCENT, 0);
    lv_obj_set_style_bg_opa(band, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(band, 0, 0);
    lv_obj_set_style_radius(band, 0, 0);

    /* Person head (circle) */
    lv_obj_t *head = lv_obj_create(badge);
    lv_obj_set_size(head, 14, 14);
    lv_obj_align(head, LV_ALIGN_CENTER, 0, -6);
    lv_obj_set_style_radius(head, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(head, SC_TEXT_DIM, 0);
    lv_obj_set_style_bg_opa(head, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(head, 0, 0);

    /* Person body (wider rounded rect) */
    lv_obj_t *body = lv_obj_create(badge);
    lv_obj_set_size(body, 22, 10);
    lv_obj_align(body, LV_ALIGN_CENTER, 0, 6);
    lv_obj_set_style_radius(body, 10, 0);
    lv_obj_set_style_bg_color(body, SC_TEXT_DIM, 0);
    lv_obj_set_style_bg_opa(body, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(body, 0, 0);

    /* Name text line */
    lv_obj_t *nline = lv_obj_create(badge);
    lv_obj_set_size(nline, 28, 3);
    lv_obj_align(nline, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(nline, SC_TEXT, 0);
    lv_obj_set_style_bg_opa(nline, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(nline, 0, 0);
    lv_obj_set_style_radius(nline, 1, 0);

    /* Subtitle text line */
    lv_obj_t *sline = lv_obj_create(badge);
    lv_obj_set_size(sline, 20, 2);
    lv_obj_align(sline, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_set_style_bg_color(sline, SC_TEXT_DIM, 0);
    lv_obj_set_style_bg_opa(sline, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(sline, 0, 0);
    lv_obj_set_style_radius(sline, 1, 0);
}

/* Schedule icon: calendar page with day number */
static void draw_icon_schedule(lv_obj_t *parent)
{
    LV_FONT_DECLARE(font_alibaba_24);

    lv_obj_t *cal = lv_obj_create(parent);
    lv_obj_set_size(cal, 48, 52);
    lv_obj_center(cal);
    lv_obj_set_style_bg_color(cal, SC_PANEL, 0);
    lv_obj_set_style_bg_opa(cal, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(cal, SC_TEXT, 0);
    lv_obj_set_style_border_width(cal, 2, 0);
    lv_obj_set_style_radius(cal, 4, 0);
    lv_obj_set_style_pad_all(cal, 0, 0);
    lv_obj_remove_flag(cal, LV_OBJ_FLAG_SCROLLABLE);

    /* Red/orange header band (like a real calendar top) */
    lv_obj_t *hdr = lv_obj_create(cal);
    lv_obj_set_size(hdr, LV_PCT(100), 14);
    lv_obj_align(hdr, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(hdr, SC_ACCENT, 0);
    lv_obj_set_style_bg_opa(hdr, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(hdr, 0, 0);
    lv_obj_set_style_radius(hdr, 0, 0);

    /* Day number "12" (May 12) */
    lv_obj_t *day = lv_label_create(cal);
    lv_label_set_text(day, "12");
    lv_obj_set_style_text_font(day, &font_alibaba_24, 0);
    lv_obj_set_style_text_color(day, SC_TEXT, 0);
    lv_obj_align(day, LV_ALIGN_CENTER, 0, 5);
}

/* Net Tools icon: signal/antenna waves */
static void draw_icon_nettools(lv_obj_t *parent)
{
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 56, 48);
    lv_obj_center(cont);
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_remove_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    /* Center dot (antenna base) */
    lv_obj_t *dot = lv_obj_create(cont);
    lv_obj_set_size(dot, 8, 8);
    lv_obj_align(dot, LV_ALIGN_BOTTOM_MID, 0, -2);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(dot, SC_ACCENT, 0);
    lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(dot, 0, 0);

    /* Concentric arcs (signal waves) — using rounded rectangles */
    int sizes[] = {22, 36, 50};
    for (int i = 0; i < 3; i++) {
        lv_obj_t *arc = lv_obj_create(cont);
        lv_obj_set_size(arc, sizes[i], sizes[i] / 2);
        lv_obj_align(arc, LV_ALIGN_BOTTOM_MID, 0, -2);
        lv_obj_set_style_bg_opa(arc, LV_OPA_0, 0);
        lv_obj_set_style_border_color(arc, SC_TEAL, 0);
        lv_obj_set_style_border_width(arc, 2, 0);
        lv_obj_set_style_border_side(arc, (lv_border_side_t)(LV_BORDER_SIDE_TOP | LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_RIGHT), 0);
        lv_obj_set_style_radius(arc, sizes[i], 0);
    }
}

/* BadgeShark icon: eye */
static void draw_icon_badgeshark(lv_obj_t *parent)
{
    lv_obj_t *c = icon_shape(parent, 52, 48);
    lv_obj_center(c);
    lv_obj_t *eye = icon_ring(c, 44, 28, SC_TEXT, 3, LV_RADIUS_CIRCLE);
    lv_obj_align(eye, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t *pupil = icon_fill(c, 16, 16, SC_TEXT, LV_RADIUS_CIRCLE);
    lv_obj_align(pupil, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t *hl = icon_fill(c, 6, 6, SC_PANEL, LV_RADIUS_CIRCLE);
    lv_obj_align(hl, LV_ALIGN_CENTER, -2, -2);
}

/* LoRa icon: simple radio tower — vertical bar with 3 horizontal bars */
static void draw_icon_lora(lv_obj_t *parent)
{
    LV_FONT_DECLARE(font_alibaba_12);
    lv_obj_t *c = icon_shape(parent, 48, 52);
    lv_obj_center(c);

    /* Tower body */
    lv_obj_t *tower = icon_fill(c, 6, 40, SC_TEXT, 2);
    lv_obj_align(tower, LV_ALIGN_CENTER, 0, 0);
    /* Cross bars */
    lv_obj_t *b1 = icon_fill(c, 30, 4, SC_TEXT, 1);
    lv_obj_align(b1, LV_ALIGN_CENTER, 0, -12);
    lv_obj_t *b2 = icon_fill(c, 22, 4, SC_TEXT, 1);
    lv_obj_align(b2, LV_ALIGN_CENTER, 0, -2);
    lv_obj_t *b3 = icon_fill(c, 14, 4, SC_TEXT, 1);
    lv_obj_align(b3, LV_ALIGN_CENTER, 0, 8);
    /* Tip */
    lv_obj_t *tip = icon_fill(c, 10, 10, SC_ACCENT, LV_RADIUS_CIRCLE);
    lv_obj_align(tip, LV_ALIGN_TOP_MID, 0, 0);
}

/* LoRa Chat icon: two overlapping speech bubbles */
static void draw_icon_chat(lv_obj_t *parent)
{
    lv_obj_t *c = icon_shape(parent, 52, 48);
    lv_obj_center(c);
    /* Back bubble (slightly offset, lighter) */
    lv_obj_t *back = icon_fill(c, 36, 26, SC_TEXT_DIM, 12);
    lv_obj_align(back, LV_ALIGN_TOP_RIGHT, -2, 0);
    /* Front bubble (foreground) */
    lv_obj_t *front = icon_fill(c, 36, 26, SC_TEXT, 12);
    lv_obj_align(front, LV_ALIGN_BOTTOM_LEFT, 2, -6);
    /* Three dots in front bubble */
    for (int i = 0; i < 3; i++) {
        lv_obj_t *dot = icon_fill(front, 5, 5, SC_PANEL, LV_RADIUS_CIRCLE);
        lv_obj_align(dot, LV_ALIGN_CENTER, (i - 1) * 10, 0);
    }
}

/* Setting icon: two horizontal sliders */
static void draw_icon_setting(lv_obj_t *parent)
{
    lv_obj_t *c = icon_shape(parent, 48, 48);
    lv_obj_center(c);

    /* Three horizontal tracks with knobs at different positions */
    int y_offsets[] = {-14, 0, 14};
    int knob_x[] = {10, -6, 6};
    for (int i = 0; i < 3; i++) {
        lv_obj_t *track = icon_fill(c, 36, 3, SC_TEXT_DIM, 1);
        lv_obj_align(track, LV_ALIGN_CENTER, 0, y_offsets[i]);
        lv_obj_t *knob = icon_fill(c, 10, 10, SC_TEXT, LV_RADIUS_CIRCLE);
        lv_obj_align(knob, LV_ALIGN_CENTER, knob_x[i], y_offsets[i]);
    }
}

/* Wireless icon: WiFi dot + 3 arcs */
static void draw_icon_wireless(lv_obj_t *parent)
{
    lv_obj_t *c = icon_shape(parent, 52, 48);
    lv_obj_center(c);
    lv_obj_t *dot = icon_fill(c, 8, 8, SC_TEXT, LV_RADIUS_CIRCLE);
    lv_obj_align(dot, LV_ALIGN_BOTTOM_MID, 0, -4);
    int sizes[] = {22, 36, 50};
    for (int i = 0; i < 3; i++) {
        lv_obj_t *arc = icon_ring(c, sizes[i], sizes[i] / 2, SC_TEXT, 3, sizes[i]);
        lv_obj_align(arc, LV_ALIGN_BOTTOM_MID, 0, -4);
        lv_obj_set_style_border_side(arc, (lv_border_side_t)(LV_BORDER_SIDE_TOP), 0);
    }
}

/* GPS icon: location pin — circle with narrow point below */
static void draw_icon_gps(lv_obj_t *parent)
{
    lv_obj_t *c = icon_shape(parent, 36, 52);
    lv_obj_center(c);
    /* Pin point (narrow, underneath the circle) */
    lv_obj_t *point = icon_fill(c, 8, 22, SC_ACCENT, 0);
    lv_obj_align(point, LV_ALIGN_BOTTOM_MID, 0, -2);
    /* Pin head circle (on top of point) */
    lv_obj_t *head = icon_fill(c, 30, 30, SC_ACCENT, LV_RADIUS_CIRCLE);
    lv_obj_align(head, LV_ALIGN_TOP_MID, 0, 2);
    /* White center dot */
    lv_obj_t *inner = icon_fill(c, 10, 10, SC_PANEL, LV_RADIUS_CIRCLE);
    lv_obj_align(inner, LV_ALIGN_TOP_MID, 0, 12);
}

/* Power icon: power button */
static void draw_icon_power(lv_obj_t *parent)
{
    lv_obj_t *c = icon_shape(parent, 48, 48);
    lv_obj_center(c);
    lv_obj_t *ring = icon_ring(c, 36, 36, SC_TEXT, 4, LV_RADIUS_CIRCLE);
    lv_obj_center(ring);
    lv_obj_t *line = icon_fill(c, 5, 22, SC_TEXT, 2);
    lv_obj_align(line, LV_ALIGN_TOP_MID, 0, 4);
}

/* Microphone icon: simple mic capsule + stand */
static void draw_icon_mic(lv_obj_t *parent)
{
    lv_obj_t *c = icon_shape(parent, 40, 52);
    lv_obj_center(c);
    /* Mic capsule */
    lv_obj_t *capsule = icon_fill(c, 20, 30, SC_TEXT, 10);
    lv_obj_align(capsule, LV_ALIGN_TOP_MID, 0, 0);
    /* Stand stem */
    lv_obj_t *stem = icon_fill(c, 4, 12, SC_TEXT, 0);
    lv_obj_align(stem, LV_ALIGN_BOTTOM_MID, 0, -8);
    /* Base */
    lv_obj_t *base = icon_fill(c, 20, 4, SC_TEXT, 2);
    lv_obj_align(base, LV_ALIGN_BOTTOM_MID, 0, -4);
}

/* IMU icon: chip with motion lines */
static void draw_icon_imu(lv_obj_t *parent)
{
    lv_obj_t *c = icon_shape(parent, 48, 48);
    lv_obj_center(c);
    /* Chip body */
    lv_obj_t *chip = icon_ring(c, 28, 28, SC_TEXT, 3, 4);
    lv_obj_align(chip, LV_ALIGN_CENTER, -2, 2);
    /* Chip pins (4 on each side) */
    for (int i = 0; i < 3; i++) {
        lv_obj_t *lp = icon_fill(c, 6, 2, SC_TEXT, 0);
        lv_obj_align(lp, LV_ALIGN_CENTER, -17, -6 + i * 6);
        lv_obj_t *rp = icon_fill(c, 6, 2, SC_TEXT, 0);
        lv_obj_align(rp, LV_ALIGN_CENTER, 13, -6 + i * 6);
    }
    /* Motion arc (upper-right) */
    lv_obj_t *a1 = icon_ring(c, 20, 10, SC_ACCENT, 2, 20);
    lv_obj_align(a1, LV_ALIGN_TOP_RIGHT, -2, 4);
    lv_obj_set_style_border_side(a1, (lv_border_side_t)(LV_BORDER_SIDE_TOP), 0);
    lv_obj_t *a2 = icon_ring(c, 30, 14, SC_ACCENT, 2, 30);
    lv_obj_align(a2, LV_ALIGN_TOP_RIGHT, 2, 0);
    lv_obj_set_style_border_side(a2, (lv_border_side_t)(LV_BORDER_SIDE_TOP), 0);
}


void menu_name_label_event_cb(lv_event_t *e)
{
#if LVGL_VERSION_MAJOR == 9
    const char *v = (const char *)lv_event_get_param(e);
    if (v) {
        lv_label_set_text(lv_event_get_target_obj(e), v);
    }
#else
    lv_obj_t *label = lv_event_get_target(e);
    lv_msg_t *m = lv_event_get_msg(e);
    const char *v = (const char *)lv_msg_get_payload(m);
    if (v) {
        lv_label_set_text(label, v);
    }
#endif
}



static void clock_update_datetime(lv_timer_t *t)
{
    const char *week[] = {"Sun", "Mon", "Tue", "Wed", "Thur", "Fri", "Sat"};
    struct tm timeinfo;
    hw_get_date_time(timeinfo);

    uint8_t week_index = timeinfo.tm_wday > 6 ? 6 : timeinfo.tm_wday;
    lv_label_set_text_fmt(clock_label.hour, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
    lv_label_set_text_fmt(clock_label.date, "%02d-%02d %s", timeinfo.tm_mon + 1, timeinfo.tm_mday, week[week_index]);
    monitor_params_t params;
    hw_get_monitor_params(params);
    lv_label_set_text_fmt(clock_label.battery_label, "%d%%", params.battery_percent);
}

lv_obj_t *setupClock()
{
    /* SkinnyCon badge idle screen — replaces the default clock face.
     * Shows conference name prominently with time/battery in footer. */
    lv_obj_t *page = lv_obj_create(lv_screen_active());
    lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));
    lv_obj_remove_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_radius(page, 0, 0);
    lv_obj_set_style_bg_color(page, SC_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, LV_PART_MAIN);

    /* Top accent bar */
    lv_obj_t *bar_top = lv_obj_create(page);
    lv_obj_set_size(bar_top, LV_PCT(100), 4);
    lv_obj_align(bar_top, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(bar_top, SC_TEAL, 0);
    lv_obj_set_style_bg_opa(bar_top, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(bar_top, 0, 0);
    lv_obj_set_style_radius(bar_top, 0, 0);
    lv_obj_set_style_pad_all(bar_top, 0, 0);

    /* Conference logo */
    lv_obj_t *logo_row = draw_skinnycon_logo(page, &font_alibaba_60, 52);
    lv_obj_align(logo_row, LV_ALIGN_CENTER, 0, -25);

    /* Year subtitle */
    lv_obj_t *year_label = lv_label_create(page);
    lv_label_set_text(year_label, "Presented by Skinny Research and Development");
    lv_obj_set_style_text_font(year_label, &font_alibaba_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(year_label, SC_TEXT_DIM, LV_PART_MAIN);
    lv_obj_align(year_label, LV_ALIGN_CENTER, 0, 15);

    /* Bottom bar with time + battery */
    lv_obj_t *footer = lv_obj_create(page);
    lv_obj_set_size(footer, LV_PCT(100), 30);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(footer, SC_PANEL, 0);
    lv_obj_set_style_bg_opa(footer, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(footer, 0, 0);
    lv_obj_set_style_radius(footer, 0, 0);
    lv_obj_set_style_pad_hor(footer, 10, 0);

    /* Time (left side of footer) */
    lv_obj_t *time_label = lv_label_create(footer);
    lv_obj_set_style_text_font(time_label, &font_alibaba_12, LV_PART_MAIN);
    lv_label_set_text(time_label, "12:34");
    lv_obj_set_style_text_color(time_label, SC_TEXT, LV_PART_MAIN);
    lv_obj_align(time_label, LV_ALIGN_LEFT_MID, 0, 0);
    clock_label.hour = time_label;  /* Reuse for time updates */

    /* Date (center of footer) */
    lv_obj_t *date_label = lv_label_create(footer);
    lv_obj_set_style_text_font(date_label, &font_alibaba_12, LV_PART_MAIN);
    lv_label_set_text(date_label, "03-24 Mon");
    lv_obj_set_style_text_color(date_label, SC_TEXT_DIM, LV_PART_MAIN);
    lv_obj_align(date_label, LV_ALIGN_CENTER, 0, 0);
    clock_label.date = date_label;

    /* Battery (right side of footer) */
    lv_obj_t *batt_label = lv_label_create(footer);
    lv_obj_set_style_text_font(batt_label, &font_alibaba_12, LV_PART_MAIN);
    lv_label_set_text(batt_label, "100%");
    lv_obj_set_style_text_color(batt_label, SC_GREEN, LV_PART_MAIN);
    lv_obj_align(batt_label, LV_ALIGN_RIGHT_MID, 0, 0);
    clock_label.battery_label = batt_label;

    /* Hidden labels not used in nametag mode (keep struct happy) */
    clock_label.seg = lv_label_create(page);
    lv_obj_add_flag(clock_label.seg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(clock_label.seg, 0, 0);
    clock_label.minute = lv_label_create(page);
    lv_obj_add_flag(clock_label.minute, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(clock_label.minute, 0, 0);
    clock_label.battery_bar = lv_bar_create(page);
    lv_obj_add_flag(clock_label.battery_bar, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(clock_label.battery_bar, 0, 0);

    clock_timer = lv_timer_create(clock_update_datetime, 1000, NULL);
    lv_timer_pause(clock_timer);

    return page;
}

#if  defined(USING_TOUCHPAD) || defined(USING_TOUCH_KEYBOARD)
typedef struct {
    lv_obj_t *obj;
    int id;
} ChildObject;

static void scrollbar_change_cb(lv_event_t *e)
{
    lv_obj_t *tileview = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *panel = lv_event_get_target_obj(e);
    lv_coord_t view_x = lv_obj_get_scroll_x(panel);
    lv_coord_t view_width = lv_obj_get_width(panel);
    int child_count = lv_obj_get_child_count(panel);
    ChildObject *child_objects = (ChildObject *)lv_malloc(child_count * sizeof(ChildObject));

    for (int i = 0; i < child_count; i++) {
        lv_obj_t *child = lv_obj_get_child(panel, i);
        child_objects[i].obj = child;
        child_objects[i].id = i;
    }

    uint8_t view_obj_count = 0;
    int32_t last_id = -1;
    for (int i = 0; i < child_count; i++) {
        lv_obj_t *child = child_objects[i].obj;
        lv_coord_t child_x = lv_obj_get_x(child);
        lv_coord_t child_width = lv_obj_get_width(child);

        if (child_x + child_width > view_x && child_x < view_x + view_width) {
            last_id = child_objects[i].id;
            view_obj_count++;
        }
    }

    if (last_id != -1) {
        lv_obj_t *obj = child_objects[last_id].obj;
        if (lv_obj_get_child(panel, 1) == obj || view_obj_count == 3) {
            last_id -= 1;
        }
        const char *name = (const char *)lv_obj_get_user_data(child_objects[last_id].obj);
        if (name) {
#if LVGL_VERSION_MAJOR == 9
            lv_obj_send_event(desc_label, (lv_event_code_t )name_change_id, (void *)name);
#else
            lv_msg_send(MSG_MENU_NAME_CHANGED, (void *)name);
#endif
        }
    }

    lv_free(child_objects);

}
#endif


static void hw_device_poll(lv_timer_t *t)
{
    monitor_params_t params;
    hw_get_monitor_params(params);
    if (params.battery_voltage < 3300 && params.usb_voltage == 0) {
        printf("Low battery voltage: %lu mV USB Voltage: %lu mV\n", params.battery_voltage, params.usb_voltage);
        lv_obj_clean(lv_screen_active());
        lv_obj_set_style_bg_color(lv_screen_active(), SC_BG, LV_PART_MAIN);
        lv_obj_set_style_radius(lv_screen_active(), 0, 0);

        lv_obj_t *image = lv_image_create(lv_screen_active());
        lv_image_set_src(image, &img_batter_low);
        lv_obj_center(image);

        lv_obj_t *label = lv_label_create(lv_screen_active());
        lv_label_set_text(label, "Battery Low!\nShutting down...");
        lv_obj_set_style_text_color(label, SC_RED, LV_PART_MAIN);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_18, LV_PART_MAIN);
        lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -30);

        lv_refr_now(NULL);
        lv_delay_ms(3000);
        hw_shutdown();
    }
}

static void ui_poll_timer_callback(lv_timer_t *t)
{
    bool timeout = lv_display_get_inactive_time(NULL) > SCREEN_TIMEOUT;
    if (timeout) {
        if (!lv_obj_has_flag(main_screen, LV_OBJ_FLAG_HIDDEN) && get_enter_low_power_flag()) {
            lv_obj_add_flag(main_screen, LV_OBJ_FLAG_HIDDEN);

            keyboard_level = hw_get_kb_backlight();
            hw_set_kb_backlight(0);
            lv_obj_remove_flag(clock_page, LV_OBJ_FLAG_HIDDEN);
            lv_timer_resume(clock_timer);

            hw_set_cpu_freq(80);

            if (hw_get_disp_timeout_ms() != 0) {
                disp_time_ms = lv_tick_get() + hw_get_disp_timeout_ms();
            } else {
                disp_time_ms = 0;
            }
        }
    } else {
        if (!lv_obj_has_flag(clock_page, LV_OBJ_FLAG_HIDDEN)) {

            hw_set_cpu_freq(240);

            lv_obj_add_flag(clock_page, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(main_screen, LV_OBJ_FLAG_HIDDEN);
            lv_timer_pause(clock_timer);

            hw_set_kb_backlight(keyboard_level);
        }
    }

    if (lv_obj_has_flag(main_screen, LV_OBJ_FLAG_HIDDEN)) {
        bool disp_on = hw_get_disp_is_on();
        if (disp_on && disp_time_ms != 0) {
            if (lv_tick_get() > disp_time_ms) {
                printf("Disp off\n");

                brightness_level =  hw_get_disp_backlight();
                printf("brightness_level:%d\n", brightness_level);

                hw_dec_brightness(0);

                hw_low_power_loop();
#ifdef NO_ENTER_LIGHT_SLEEP
                printf("Enter sleep\n");
                pinMode(0, INPUT_PULLUP);
                while (digitalRead(0) == HIGH) {
                    delay(10);
                }
                printf("Wakeup\n");
#endif
                lv_obj_add_flag(clock_page, LV_OBJ_FLAG_HIDDEN);
                lv_obj_remove_flag(main_screen, LV_OBJ_FLAG_HIDDEN);
                lv_timer_pause(clock_timer);

                hw_set_cpu_freq(240);

                lv_refr_now(NULL);

                lv_display_trigger_activity(NULL);

                hw_inc_brightness(brightness_level);

                hw_set_kb_backlight(keyboard_level);
            }
        }
    }
}

void setupGui()
{

    lv_obj_set_style_bg_color(lv_screen_active(), SC_BG, LV_PART_MAIN);
    lv_obj_set_style_radius(lv_screen_active(), 0, 0);

    /* Boot logo */
    lv_obj_t *boot_cont = lv_obj_create(lv_screen_active());
    lv_obj_set_size(boot_cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(boot_cont, LV_OPA_0, 0);
    lv_obj_set_style_border_width(boot_cont, 0, 0);
    lv_obj_set_flex_flow(boot_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(boot_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    draw_skinnycon_logo(boot_cont, &font_alibaba_40, 36);

    lv_obj_t *year_lbl = lv_label_create(boot_cont);
    lv_label_set_text(year_lbl, "2026");
    lv_obj_set_style_text_font(year_lbl, &font_alibaba_24, 0);
    lv_obj_set_style_text_color(year_lbl, SC_TEXT_DIM, 0);

    lv_refr_now(NULL);
    lv_delay_ms(5000);
    lv_obj_delete(boot_cont);

    disable_keyboard();

    const lv_font_t  *main_font = MAIN_FONT;
    lv_theme_default_init(NULL, SC_ACCENT, SC_BG,
                          false, main_font);

    theme_init();

    // Create groups
    menu_g = lv_group_create();
    app_g = lv_group_create();
    set_default_group(menu_g);

    static lv_style_t style_frameless;
    lv_style_init(&style_frameless);
    lv_style_set_radius(&style_frameless, 0);
    lv_style_set_border_width(&style_frameless, 0);
    lv_style_set_bg_color(&style_frameless, SC_BG);
    lv_style_set_shadow_width(&style_frameless, 55);
    lv_style_set_shadow_color(&style_frameless, SC_BORDER);

    /* opening animation */
    main_screen = lv_tileview_create(lv_screen_active());

    lv_obj_align(main_screen, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_size(main_screen, LV_PCT(100), LV_PCT(100));

    /* Create two views for switching menus and app UI */
    menu_panel = lv_tileview_add_tile(main_screen, 0, 0, LV_DIR_HOR);
    lv_tileview_add_tile(main_screen, 0, 1, LV_DIR_HOR);

    lv_obj_set_scrollbar_mode(main_screen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(main_screen, LV_OBJ_FLAG_SCROLLABLE);

    /* Initialize the menu view */
    lv_obj_t *panel = lv_obj_create(menu_panel);
    lv_obj_set_scrollbar_mode(panel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_size(panel, LV_PCT(100), LV_PCT(70));
    lv_obj_set_scroll_snap_x(panel, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_ROW);
    lv_obj_align(panel, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_style(panel, &style_frameless, 0);
#if  defined(USING_TOUCHPAD) || defined(USING_TOUCH_KEYBOARD)
    lv_obj_add_event_cb(panel, scrollbar_change_cb, LV_EVENT_SCROLL_END, NULL);
#endif

    extern app_t ui_sys_main ;
    extern app_t ui_radio_main ;
    extern app_t ui_audio_main ;
    extern app_t ui_wireless_main ;
    extern app_t ui_gps_main ;
    extern app_t ui_monitor_main ;
    extern app_t ui_power_main ;
    extern app_t ui_calendar_main;
    extern app_t ui_info_main;
    extern app_t ui_microphone_main;
    extern app_t ui_keyboard_main;
    extern app_t ui_sensor_main;
    extern app_t ui_msgchat_main;
    extern app_t ui_ble_main;
    extern app_t ui_ble_kb_main;
    extern app_t ui_factory_main;
    extern app_t ui_nametag_main;
    extern app_t ui_badgeshark_main;
    extern app_t ui_schedule_main;
    extern app_t ui_nettools_main;

    /* SkinnyCon primary apps — first in menu (custom drawn icons) */
    create_app_drawn(panel, "Nametag", draw_icon_nametag, &ui_nametag_main);
    create_app_drawn(panel, "Schedule", draw_icon_schedule, &ui_schedule_main);
    create_app_drawn(panel, "BadgeShark", draw_icon_badgeshark, &ui_badgeshark_main);
    create_app_drawn(panel, "Net Tools", draw_icon_nettools, &ui_nettools_main);

    /* Radio & messaging */
    create_app_drawn(panel, "LoRa", draw_icon_lora, &ui_radio_main);
    create_app_drawn(panel, "LoRa Chat", draw_icon_chat, &ui_msgchat_main);

    /* Other apps */
#if defined(USING_IR_REMOTE)
    extern app_t ui_ir_remote_main;
    create_app(panel, "IR Remote", &img_ir_remote, &ui_ir_remote_main);
#endif

#if defined(USING_EXTERN_NRF2401)
    extern app_t ui_nrf24_main;
    create_app(panel, "NRF24", &img_radio, &ui_nrf24_main);
#endif

#if defined(USING_BLE_CONTROL)
    create_app(panel, "Camera Remote", &img_camera, &ui_camera_remote_main);
#endif

#if defined(USING_SI473X_RADIO)
    extern app_t ui_si4735_main;
    create_app(panel, "Radio", &img_si4735, &ui_si4735_main);
#endif

#if defined(USING_MAG_QMC5883)
    extern app_t ui_compass_main;
    create_app(panel, "Compass", &img_compass, &ui_compass_main);
#endif

#if defined(USING_TRACKBALL)
    extern app_t ui_trackball_main;
    create_app(panel, "Trackball", &img_track, &ui_trackball_main);
#endif

#if defined(USING_ST25R3916)
    extern app_t ui_nfc_main;
    create_app(panel, "NFC", &img_nfc, &ui_nfc_main);
#endif

    // #if defined(TODO://)
    // extern app_t ui_recorder_main;
    // create_app(panel, "Recorder", &img_track, &ui_recorder_main);
    // #endif

    // Removed: Screen Test app (development-only, not needed for SkinnyCon)
    create_app_drawn(panel, "Setting", draw_icon_setting, &ui_sys_main);
    create_app_drawn(panel, "Wireless", draw_icon_wireless, &ui_wireless_main);

#if defined(USING_UART_BLE)
    create_app(panel, "Bluetooth", &img_bluetooth, &ui_ble_main);
#endif

#if defined(USING_INPUT_DEV_KEYBOARD)
    if (hw_has_keyboard()) {
        // Removed: Keyboard and Music apps (not needed for SkinnyCon)
    }
#endif

    create_app_drawn(panel, "GPS", draw_icon_gps, &ui_gps_main);
    // Removed: Monitor app (battery info already on clock face, detailed stats are dev-only)
    create_app_drawn(panel, "Power", draw_icon_power, &ui_power_main);
    create_app_drawn(panel, "Microphone", draw_icon_mic, &ui_microphone_main);
    create_app_drawn(panel, "IMU", draw_icon_imu, &ui_sensor_main);

    int offset = -10;
    if (lv_display_get_physical_vertical_resolution(NULL) > 320) {
        offset = -45;
    }
    /* Initialize the label */
    desc_label = lv_label_create(menu_panel);
    lv_obj_set_width(desc_label, LV_PCT(100));
    lv_obj_align(desc_label, LV_ALIGN_BOTTOM_MID, 0, offset);
    lv_obj_set_style_text_align(desc_label, LV_TEXT_ALIGN_CENTER, 0);

    if (lv_display_get_physical_horizontal_resolution(NULL) < 320) {
        lv_obj_set_style_text_font(desc_label, &font_alibaba_24, 0);
        lv_obj_align(desc_label, LV_ALIGN_BOTTOM_MID, 0, -25);
    } else {
        lv_obj_set_style_text_font(desc_label, &font_alibaba_40, 0);
    }
    lv_label_set_long_mode(desc_label, LV_LABEL_LONG_SCROLL_CIRCULAR);

#if LVGL_VERSION_MAJOR == 9
    name_change_id =  lv_event_register_id();
    lv_obj_add_event_cb(desc_label, menu_name_label_event_cb, (lv_event_code_t )name_change_id, NULL);
    lv_obj_send_event(lv_obj_get_child(panel, 0), LV_EVENT_FOCUSED, NULL);
#else
    lv_obj_add_event_cb(desc_label, menu_name_label_event_cb, LV_EVENT_MSG_RECEIVED, NULL);
    lv_msg_subsribe_obj(MSG_MENU_NAME_CHANGED, desc_label, NULL);
    lv_event_send(lv_obj_get_child(panel, 0), LV_EVENT_FOCUSED, NULL);
#endif

    lv_obj_update_snap(panel, LV_ANIM_ON);


    clock_page = setupClock();
    lv_obj_add_flag(clock_page, LV_OBJ_FLAG_HIDDEN);

    disp_timer = lv_timer_create(ui_poll_timer_callback, 1000, NULL);

    dev_timer = lv_timer_create(hw_device_poll, 5000, NULL);

    // Allow low power mode
    set_low_power_mode_flag(true);
    lv_display_trigger_activity(NULL);
}




static lv_obj_t *canvas;
static lv_indev_t *touch_indev;

void touch_panel_init()
{
    uint32_t width = lv_disp_get_hor_res(NULL);
    uint32_t height = lv_disp_get_ver_res(NULL);
#if 1
    lv_color_format_t cf = LV_COLOR_FORMAT_ARGB8888;
    uint32_t buffer_size =    LV_DRAW_BUF_SIZE(width, height, cf);
    uint8_t *buf_draw_buf = (uint8_t *)malloc(buffer_size);
    uint16_t stride_size = LV_DRAW_BUF_STRIDE(width, cf);

    printf("data_size:%u\n", buffer_size);
    printf("stride:%u\n", stride_size);
    printf("cf:%u\n", cf);

    static lv_draw_buf_t draw_buf = {
        .header = {
            .magic = (0x19),
            .cf = (cf),
            .flags = LV_IMAGE_FLAGS_MODIFIABLE,
            .w = (width),
            .h = (height),
            .stride = stride_size,
            .reserved_2 = 0,
        },
        .data_size = buffer_size,
        .data = buf_draw_buf,
        .unaligned_data = buf_draw_buf,
    };

    lv_image_header_t *header = &draw_buf.header;
    lv_draw_buf_init(&draw_buf, header->w, header->h,
                     (lv_color_format_t)header->cf,
                     header->stride,
                     buf_draw_buf,
                     buffer_size);
    lv_draw_buf_set_flag(&draw_buf, LV_IMAGE_FLAGS_MODIFIABLE);

    printf("data_size:%u\n", draw_buf.data_size);
    printf("stride:%u\n", draw_buf.header.stride);
    printf("cf:%u\n", draw_buf.header.cf);

#else
    // /*Create a buffer for the canvas*/
    LV_DRAW_BUF_DEFINE_STATIC(draw_buf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_COLOR_FORMAT_ARGB8888);
    LV_DRAW_BUF_INIT_STATIC(draw_buf);
#endif

    /*Create a canvas and initialize its palette*/
    canvas = lv_canvas_create(lv_screen_active());
    lv_canvas_set_draw_buf(canvas, &draw_buf);
    lv_canvas_fill_bg(canvas, lv_color_hex3(0xccc), LV_OPA_COVER);
    lv_obj_center(canvas);


    lv_indev_t *indev = lv_indev_get_next(NULL);
    while (indev) {
        if (lv_indev_get_type(indev) == LV_INDEV_TYPE_POINTER) {
            touch_indev = indev;
            break;
        }
        indev = lv_indev_get_next(indev);
    }
    lv_indev_type_t type = lv_indev_get_type(touch_indev);
    if (type != LV_INDEV_TYPE_POINTER) {
        return;
    }

    lv_timer_create([](lv_timer_t *t) {

#undef lv_point_t
        lv_point_t  point;
        lv_indev_state_t state =  lv_indev_get_state(touch_indev);
        if ( state == LV_INDEV_STATE_PRESSED ) {
            lv_indev_get_point(touch_indev, &point);
            printf("%d %d\n", point.x, point.y);

            lv_layer_t layer;
            lv_canvas_init_layer(canvas, &layer);

            lv_draw_arc_dsc_t dsc;
            lv_draw_arc_dsc_init(&dsc);
            dsc.color = lv_palette_main(LV_PALETTE_RED);
            dsc.width = 2;
            dsc.center.x =  point.x;
            dsc.center.y = point.y;
            dsc.width = 10;
            dsc.radius = 6;
            dsc.start_angle = 0;
            dsc.end_angle = 360;
            lv_draw_arc(&layer, &dsc);
            lv_canvas_finish_layer(canvas, &layer);
        }
    }, 30, NULL);

}


























