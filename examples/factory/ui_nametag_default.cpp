/**
 * @file      ui_nametag_default.cpp
 * @brief     Override default screen to show Nametag instead of Clock
 * @details   This file replaces the default clock screen with nametag for conference use.
 *            Users can switch back to clock via settings.
 */

#include "lvgl.h"
#include "ui_nametag.cpp"

/**
 * @brief Setup nametag as default screen
 * @return LV_OBJ_T* The nametag page object
 */
lv_obj_t* setupNametagDefault() {
    // Call the nametag setup function
    nametag_setup();
    
    // Return the nametag page
    return ui_nametag_main.page;
}

/**
 * @brief Exit nametag screen
 */
void exitNametagDefault() {
    nametag_exit();
}

/**
 * @brief Application structure for nametag default
 */
app_t ui_nametag_default_main = {
    setupNametagDefault,
    exitNametagDefault,
    NULL
};
