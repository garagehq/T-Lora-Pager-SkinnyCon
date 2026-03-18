/**
 * @file      nav_logic_helpers.h
 * @brief     Navigation logic helper functions for T-LoRa-Pager-SkinnyCon
 * 
 * This module provides reusable navigation logic for tileview-based UIs,
 * including:
 * - Page navigation (next/prev/back/first/last)
 * - Focus management for widgets
 * - Tileview state helpers
 * - Navigation animation control
 * 
 * These helpers are designed to work with LVGL's tileview widget and
 * integrate with the screen_state_manager and button_event_handlers modules.
 */

#ifndef NAV_LOGIC_HELPERS_H
#define NAV_LOGIC_HELPERS_H

#include <stdint.h>
#include <stdbool.h>
#include "ui_define.h"

/* Maximum number of pages in a tileview */
#define MAX_TILEVIEW_PAGES  8

/* Tileview navigation direction */
typedef enum {
    NAV_DIR_LEFT,         /* Previous tile */
    NAV_DIR_RIGHT,        /* Next tile */
    NAV_DIR_UP,           /* Previous row (if vertical) */
    NAV_DIR_DOWN,         /* Next row (if vertical) */
    NAV_DIR_FIRST,        /* First tile in list */
    NAV_DIR_LAST,         /* Last tile in list */
    NAV_DIR_COUNT
} NavDirection_t;

/* Navigation mode */
typedef enum {
    NAV_MODE_DIRECT,      /* Direct navigation (first/last/next/prev) */
    NAV_MODE_CIRCULAR,    /* Circular (last->first, first->last) */
    NAV_MODE_TREE,        /* Tree-like (back parent after children) */
    NAV_MODE_COUNT
} NavMode_t;

/* Tileview navigation context */
typedef struct {
    lv_obj_t *tileview;       /* Parent tileview widget */
    NavMode_t mode;           /* Navigation mode */
    uint32_t current_tile;    /* Current tile row */
    uint32_t current_col;     /* Current tile column */
    uint32_t page_count;      /* Number of pages */
    bool animation_enabled;   /* Enable/disable animations */
    uint32_t animation_time;  /* Animation duration (ms) */
} NavContext_t;

/* ================================================================
 *  NAVIGATION CONTEXT FUNCTIONS
 * ================================================================ */

/**
 * @brief Initialize navigation context
 * 
 * @param context Pointer to context to initialize
 * @param tileview Parent tileview widget
 * @param mode Navigation mode
 */
void nav_context_init(NavContext_t *context, lv_obj_t *tileview, NavMode_t mode);

/**
 * @brief Get current navigation context tile index
 * 
 * @param context Pointer to context
 * @return Current tile index, or UINT32_MAX if invalid
 */
uint32_t nav_context_get_current_tile(const NavContext_t *context);

/**
 * @brief Set current tile index
 * 
 * @param context Pointer to context
 * @param tile Tile index to set
 */
void nav_context_set_current_tile(NavContext_t *context, uint32_t tile);

/**
 * @brief Enable/disable animation
 * 
 * @param context Pointer to context
 * @param enabled true to enable, false to disable
 */
void nav_context_set_animation_enabled(NavContext_t *context, bool enabled);

/**
 * @brief Set animation duration
 * 
 * @param context Pointer to context
 * @param time_ms Animation time in milliseconds
 */
void nav_context_set_animation_time(NavContext_t *context, uint32_t time_ms);

/* ================================================================
 *  NAVIGATION HELPER FUNCTIONS
 * ================================================================ */

/**
 * @brief Navigate to next tile (right)
 * 
 * Handles mode-specific navigation (direct vs circular vs tree).
 * 
 * @param context Pointer to navigation context
 * @return true if navigation performed, false if at last tile
 */
bool nav_next(NavContext_t *context);

/**
 * @brief Navigate to previous tile (left)
 * 
 * @param context Pointer to navigation context
 * @return true if navigation performed, false if at first tile
 */
bool nav_prev(NavContext_t *context);

/**
 * @brief Navigate back (for tree mode)
 * 
 * In tree mode, returns to parent. In other modes, acts like nav_prev.
 * 
 * @param context Pointer to navigation context
 * @return true if navigation performed, false if at root
 */
bool nav_back(NavContext_t *context);

/**
 * @brief Navigate to first tile
 * 
 * @param context Pointer to navigation context
 */
void nav_first(NavContext_t *context);

/**
 * @brief Navigate to last tile
 * 
 * @param context Pointer to navigation context
 */
void nav_last(NavContext_t *context);

/**
 * @brief Navigate to specific tile
 * 
 * @param context Pointer to navigation context
 * @param tile Tile index to navigate to
 * @return true if valid tile, false if tile index out of range
 */
bool nav_goto(NavContext_t *context, uint32_t tile);

/* ================================================================
 *  TILEVIEW WIDGET HELPERS
 * ================================================================ */

/**
 * @brief Get tileview width for current navigation
 * 
 * Helper to calculate tile dimensions.
 * 
 * @param tileview Tileview widget
 * @return Width in pixels
 */
lv_coord_t nav_get_tileview_width(lv_obj_t *tileview);

/**
 * @brief Get tileview height for current navigation
 * 
 * @param tileview Tileview widget
 * @return Height in pixels
 */
lv_coord_t nav_get_tileview_height(lv_obj_t *tileview);

/**
 * @brief Get current visible tile index
 * 
 * @param tileview Tileview widget
 * @return Tile index, or UINT32_MAX if invalid
 */
uint32_t nav_get_current_visible_tile(lv_obj_t *tileview);

/**
 * @brief Set tileview tile with animation control
 * 
 * Wrapper around lv_tileview_set_tile_by_index with animation option.
 * 
 * @param tileview Tileview widget
 * @param row Row index
 * @param col Column index
 * @param anim true to animate, false for instant
 */
void nav_set_tile(lv_obj_t *tileview, uint32_t row, uint32_t col, bool anim);

/**
 * @brief Get number of tiles in a tileview row
 * 
 * @param tileview Tileview widget
 * @return Number of tiles, or 0 if invalid
 */
uint32_t nav_get_tile_count(lv_obj_t *tileview);

/* ================================================================
 *  FOCUS MANAGEMENT
 * ================================================================ */

/**
 * @brief Focus next focusable widget in current tile
 * 
 * @param tileview Tileview widget
 * @return true if focused, false if no focusable widget
 */
bool nav_focus_next(lv_obj_t *tileview);

/**
 * @brief Focus previous focusable widget in current tile
 * 
 * @param tileview Tileview widget
 * @return true if focused, false if no focusable widget
 */
bool nav_focus_prev(lv_obj_t *tileview);

/**
 * @brief Focus first focusable widget in current tile
 * 
 * @param tileview Tileview widget
 * @return true if focused, false if no focusable widget
 */
bool nav_focus_first(lv_obj_t *tileview);

/**
 * @brief Focus last focusable widget in current tile
 * 
 * @param tileview Tileview widget
 * @return true if focused, false if no focusable widget
 */
bool nav_focus_last(lv_obj_t *tileview);

/**
 * @brief Set default focus on tileview entry
 * 
 * Automatically focuses first widget in tile.
 * 
 * @param tileview Tileview widget
 */
void nav_set_default_focus(lv_obj_t *tileview);

/* ================================================================
 *  NAVIGATION EVENT HANDLERS (LVGL)
 * ================================================================ */

/**
 * @brief Create tileview event handler for navigation
 * 
 * Attaches event handler to tileview for navigation events.
 * 
 * @param tileview Tileview widget
 * @param context Navigation context
 */
void nav_create_event_handler(lv_obj_t *tileview, NavContext_t *context);

/**
 * @brief Handle rotate up event (navigation)
 * 
 * Called by encoder rotate-up.
 * 
 * @param tileview Tileview widget
 * @param context Navigation context
 */
void nav_handle_rotate_up(lv_obj_t *tileview, NavContext_t *context);

/**
 * @brief Handle rotate down event (navigation)
 * 
 * Called by encoder rotate-down.
 * 
 * @param tileview Tileview widget
 * @param context Navigation context
 */
void nav_handle_rotate_down(lv_obj_t *tileview, NavContext_t *context);

/**
 * @brief Handle click event (navigation)
 * 
 * Called by encoder click (select).
 * 
 * @param tileview Tileview widget
 * @param context Navigation context
 */
void nav_handle_click(lv_obj_t *tileview, NavContext_t *context);

/* ================================================================
 *  UTILITY FUNCTIONS
 * ================================================================ */

/**
 * @brief Get string name of navigation mode (debugging)
 * 
 * @param mode Mode to convert
 * @return Static string with mode name
 */
const char *nav_get_mode_name(NavMode_t mode);

/**
 * @brief Get string name of navigation direction (debugging)
 * 
 * @param dir Direction to convert
 * @return Static string with direction name
 */
const char *nav_get_direction_name(NavDirection_t dir);

/**
 * @brief Print navigation context state (debugging)
 * 
 * @param context Pointer to context
 */
void nav_print_context(NavContext_t *context);

#endif /* NAV_LOGIC_HELPERS_H */
