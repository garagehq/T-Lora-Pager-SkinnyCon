/**
 * @file sim_main.h
 * Headless LVGL 9.2 simulator API for T-LoRa-Pager (480x222 RGB565).
 */

#ifndef SIM_MAIN_H
#define SIM_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include <stdint.h>

/**
 * @brief Initialize LVGL with a headless 480x222 RGB565 display.
 *        Call once before any LVGL operations in tests.
 */
void lvgl_sim_init(void);

/**
 * @brief Tear down the LVGL simulator, freeing resources.
 */
void lvgl_sim_deinit(void);

/**
 * @brief Advance LVGL timers by the given number of milliseconds.
 *        This runs lv_timer_handler() in steps of LV_DEF_REFR_PERIOD.
 * @param ms Number of milliseconds to simulate.
 */
void lvgl_test_run(uint32_t ms);

/**
 * @brief Save the current framebuffer contents as a PPM image.
 * @param filename Output file path (e.g., "screenshot.ppm").
 * @return 0 on success, -1 on failure.
 */
int lvgl_test_save_ppm(const char *filename);

/**
 * @brief Get a pointer to the raw RGB565 framebuffer.
 * @return Pointer to the framebuffer array (480 * 222 uint16_t values).
 */
uint16_t *lvgl_sim_get_framebuffer(void);

/**
 * @brief Get the horizontal resolution of the simulated display.
 * @return 480
 */
int lvgl_sim_get_hor_res(void);

/**
 * @brief Get the vertical resolution of the simulated display.
 * @return 222
 */
int lvgl_sim_get_ver_res(void);

#ifdef __cplusplus
}
#endif

#endif /* SIM_MAIN_H */
