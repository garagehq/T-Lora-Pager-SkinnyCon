/**
 * @file sim_main.c
 * SDL2-free headless LVGL 9.2 simulator for T-LoRa-Pager (480x222 RGB565).
 *
 * Provides:
 *   - lvgl_sim_init()          Initialize LVGL with headless display
 *   - lvgl_sim_deinit()        Tear down LVGL
 *   - lvgl_test_run(ms)        Run lv_timer_handler() for N milliseconds
 *   - lvgl_test_save_ppm(fn)   Save current framebuffer as PPM image
 *
 * The framebuffer uses RGB565 (16-bit) matching the physical display.
 * PPM export converts to 24-bit RGB for standard image viewers.
 */

#include "sim_main.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ---- Display geometry (T-LoRa-Pager) ---- */
#define SIM_HOR_RES  480
#define SIM_VER_RES  222

/* ---- Static framebuffer ---- */
static uint16_t framebuffer[SIM_HOR_RES * SIM_VER_RES];
static lv_display_t *sim_display = NULL;

/* ---- Tick source ---- */
static uint32_t sim_tick_ms = 0;

static uint32_t sim_tick_get_cb(void)
{
    return sim_tick_ms;
}

/* ---- Flush callback: copy rendered area into framebuffer ---- */
static void sim_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    int32_t x, y;
    uint16_t *src = (uint16_t *)px_map;

    for (y = area->y1; y <= area->y2; y++) {
        for (x = area->x1; x <= area->x2; x++) {
            if (x >= 0 && x < SIM_HOR_RES && y >= 0 && y < SIM_VER_RES) {
                framebuffer[y * SIM_HOR_RES + x] = *src;
            }
            src++;
        }
    }

    lv_display_flush_ready(disp);
}

/* ---- Public API ---- */

void lvgl_sim_init(void)
{
    lv_init();

    /* Register tick source */
    lv_tick_set_cb(sim_tick_get_cb);

    /* Create display with 480x222 resolution */
    sim_display = lv_display_create(SIM_HOR_RES, SIM_VER_RES);
    if (!sim_display) {
        fprintf(stderr, "sim_main: failed to create display\n");
        return;
    }

    /* Set color format to RGB565 */
    lv_display_set_color_format(sim_display, LV_COLOR_FORMAT_RGB565);

    /* Allocate draw buffers */
    static uint8_t buf1[SIM_HOR_RES * 40 * 2]; /* 40 rows, 2 bytes per pixel */
    lv_display_set_buffers(sim_display, buf1, NULL, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);

    /* Set flush callback */
    lv_display_set_flush_cb(sim_display, sim_flush_cb);

    /* Clear framebuffer */
    memset(framebuffer, 0, sizeof(framebuffer));

    sim_tick_ms = 0;
}

void lvgl_sim_deinit(void)
{
    if (sim_display) {
        lv_display_delete(sim_display);
        sim_display = NULL;
    }
    lv_deinit();
}

void lvgl_test_run(uint32_t ms)
{
    uint32_t target = sim_tick_ms + ms;
    while (sim_tick_ms < target) {
        lv_timer_handler();
        sim_tick_ms += LV_DEF_REFR_PERIOD;
    }
}

int lvgl_test_save_ppm(const char *filename)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "sim_main: cannot open %s for writing\n", filename);
        return -1;
    }

    /* PPM header: P6 format (binary RGB) */
    fprintf(fp, "P6\n%d %d\n255\n", SIM_HOR_RES, SIM_VER_RES);

    /* Convert RGB565 to RGB888 */
    for (int i = 0; i < SIM_HOR_RES * SIM_VER_RES; i++) {
        uint16_t px = framebuffer[i];
        uint8_t r = ((px >> 11) & 0x1F) << 3;
        uint8_t g = ((px >> 5) & 0x3F) << 2;
        uint8_t b = (px & 0x1F) << 3;
        /* Expand low bits for better color fidelity */
        r |= r >> 5;
        g |= g >> 6;
        b |= b >> 5;
        uint8_t rgb[3] = {r, g, b};
        fwrite(rgb, 1, 3, fp);
    }

    fclose(fp);
    return 0;
}

uint16_t *lvgl_sim_get_framebuffer(void)
{
    return framebuffer;
}

int lvgl_sim_get_hor_res(void)
{
    return SIM_HOR_RES;
}

int lvgl_sim_get_ver_res(void)
{
    return SIM_VER_RES;
}
