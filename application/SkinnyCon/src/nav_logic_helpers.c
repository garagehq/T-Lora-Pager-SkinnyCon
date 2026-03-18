/**
 * @file      nav_logic_helpers.c
 * @brief     Navigation logic helper implementation
 * 
 * See nav_logic_helpers.h for API documentation.
 * 
 * This module provides reusable navigation logic for tileview-based UIs,
 * with support for multiple navigation modes (direct, circular, tree).
 */

#include "nav_logic_helpers.h"
#include <stdio.h>

static const char *mode_names[] = {
    "DIRECT",
    "CIRCULAR",
    "TREE"
};

static const char *direction_names[] = {
    "LEFT",
    "RIGHT",
    "UP",
    "DOWN",
    "FIRST",
    "LAST"
};

void nav_context_init(NavContext_t *context, lv_obj_t *tileview, NavMode_t mode)
{
    if (!context || !tileview) {
        return;
    }
    
    context->tileview = tileview;
    context->mode = mode;
    context->current_tile = 0;
    context->current_col = 0;
    context->page_count = nav_get_tile_count(tileview);
    context->animation_enabled = true;
    context->animation_time = 300;  /* Default 300ms animation */
}

uint32_t nav_context_get_current_tile(const NavContext_t *context)
{
    if (!context) {
        return UINT32_MAX;
    }
    return context->current_tile;
}

void nav_context_set_current_tile(NavContext_t *context, uint32_t tile)
{
    if (!context) {
        return;
    }
    
    uint32_t max_tile = nav_get_tile_count(context->tileview);
    if (tile < max_tile) {
        context->current_tile = tile;
    }
}

void nav_context_set_animation_enabled(NavContext_t *context, bool enabled)
{
    if (context) {
        context->animation_enabled = enabled;
    }
}

void nav_context_set_animation_time(NavContext_t *context, uint32_t time_ms)
{
    if (context) {
        context->animation_time = time_ms;
    }
}

bool nav_next(NavContext_t *context)
{
    if (!context) {
        return false;
    }
    
    uint32_t max_tile = context->page_count;
    if (max_tile == 0) {
        return false;
    }
    
    uint32_t next_tile = context->current_tile + 1;
    if (next_tile >= max_tile) {
        /* Handle mode-specific behavior */
        switch (context->mode) {
            case NAV_MODE_CIRCULAR:
                /* Wrap to first */
                next_tile = 0;
                break;
            case NAV_MODE_TREE:
                /* Stay at last */
                return false;
            case NAV_MODE_DIRECT:
            default:
                /* Stay at last */
                return false;
        }
    }
    
    context->current_tile = next_tile;
    nav_goto(context->tileview, next_tile, context->animation_enabled);
    return true;
}

bool nav_prev(NavContext_t *context)
{
    if (!context) {
        return false;
    }
    
    uint32_t max_tile = context->page_count;
    if (max_tile == 0) {
        return false;
    }
    
    if (context->current_tile == 0) {
        /* Handle mode-specific behavior */
        switch (context->mode) {
            case NAV_MODE_CIRCULAR:
                /* Wrap to last */
                context->current_tile = max_tile - 1;
                nav_goto(context->tileview, context->current_tile, 
                         context->animation_enabled);
                return true;
            case NAV_MODE_TREE:
            case NAV_MODE_DIRECT:
            default:
                /* Stay at first */
                return false;
        }
    }
    
    context->current_tile--;
    nav_goto(context->tileview, context->current_tile, context->animation_enabled);
    return true;
}

bool nav_back(NavContext_t *context)
{
    if (!context) {
        return false;
    }
    
    if (context->mode == NAV_MODE_TREE) {
        /* In tree mode, go to parent (simplified: go to previous) */
        return nav_prev(context);
    } else {
        /* Other modes: act like prev */
        return nav_prev(context);
    }
}

void nav_first(NavContext_t *context)
{
    if (!context) {
        return;
    }
    
    context->current_tile = 0;
    nav_goto(context->tileview, 0, context->animation_enabled);
}

void nav_last(NavContext_t *context)
{
    if (!context) {
        return;
    }
    
    uint32_t max_tile = context->page_count;
    if (max_tile > 0) {
        context->current_tile = max_tile - 1;
        nav_goto(context->tileview, context->current_tile, context->animation_enabled);
    }
}

bool nav_goto(lv_obj_t *tileview, uint32_t tile, bool anim)
{
    if (!tileview) {
        return false;
    }
    
    uint32_t max_tile = nav_get_tile_count(tileview);
    if (tile >= max_tile) {
        return false;
    }
    
    lv_tileview_set_tile_by_index(tileview, 0, tile, anim ? LV_ANIM_ON : LV_ANIM_OFF);
    return true;
}

bool nav_goto_context(NavContext_t *context, uint32_t tile)
{
    if (!context || !context->tileview) {
        return false;
    }
    
    uint32_t max_tile = context->page_count;
    if (tile >= max_tile) {
        return false;
    }
    
    context->current_tile = tile;
    nav_goto(context->tileview, tile, context->animation_enabled);
    return true;
}

lv_coord_t nav_get_tileview_width(lv_obj_t *tileview)
{
    if (!tileview) {
        return 0;
    }
    return lv_obj_get_width(tileview);
}

lv_coord_t nav_get_tileview_height(lv_obj_t *tileview)
{
    if (!tileview) {
        return 0;
    }
    return lv_obj_get_height(tileview);
}

uint32_t nav_get_current_visible_tile(lv_obj_t *tileview)
{
    if (!tileview) {
        return UINT32_MAX;
    }
    
    lv_coord_t scroll_x = lv_obj_get_scroll_x(tileview);
    lv_coord_t tile_width = nav_get_tileview_width(tileview);
    
    if (tile_width <= 0) {
        return 0;
    }
    
    return (uint32_t)(scroll_x / tile_width);
}

void nav_set_tile(lv_obj_t *tileview, uint32_t row, uint32_t col, bool anim)
{
    if (!tileview) {
        return;
    }
    lv_tileview_set_tile_by_index(tileview, row, col, anim ? LV_ANIM_ON : LV_ANIM_OFF);
}

uint32_t nav_get_tile_count(lv_obj_t *tileview)
{
    if (!tileview) {
        return 0;
    }
    
    uint32_t count = 0;
    lv_obj_t *child = lv_obj_get_child(tileview, 0);
    while (child) {
        count++;
        child = lv_obj_get_next(child);
    }
    return count;
}

bool nav_focus_next(lv_obj_t *tileview)
{
    if (!tileview) {
        return false;
    }
    
    lv_group_t *group = lv_obj_get_group(tileview);
    if (!group) {
        return false;
    }
    
    lv_obj_t *current = lv_group_get_focused(group);
    lv_obj_t *next = lv_obj_get_next(current);
    
    if (next) {
        lv_group_focus_obj(next);
        return true;
    }
    
    return false;
}

bool nav_focus_prev(lv_obj_t *tileview)
{
    if (!tileview) {
        return false;
    }
    
    lv_group_t *group = lv_obj_get_group(tileview);
    if (!group) {
        return false;
    }
    
    lv_obj_t *current = lv_group_get_focused(group);
    lv_obj_t *prev = lv_obj_get_prev(current);
    
    if (prev) {
        lv_group_focus_obj(prev);
        return true;
    }
    
    return false;
}

bool nav_focus_first(lv_obj_t *tileview)
{
    if (!tileview) {
        return false;
    }
    
    lv_group_t *group = lv_obj_get_group(tileview);
    if (!group) {
        return false;
    }
    
    lv_obj_t *first = lv_group_get_first(group);
    if (first) {
        lv_group_focus_obj(first);
        return true;
    }
    
    return false;
}

bool nav_focus_last(lv_obj_t *tileview)
{
    if (!tileview) {
        return false;
    }
    
    lv_group_t *group = lv_obj_get_group(tileview);
    if (!group) {
        return false;
    }
    
    lv_obj_t *last = NULL;
    lv_obj_t *child = lv_group_get_first(group);
    while (child) {
        last = child;
        child = lv_obj_get_next(child);
    }
    
    if (last) {
        lv_group_focus_obj(last);
        return true;
    }
    
    return false;
}

void nav_set_default_focus(lv_obj_t *tileview)
{
    if (!tileview) {
        return;
    }
    
    lv_group_t *group = lv_obj_get_group(tileview);
    if (!group) {
        return;
    }
    
    lv_obj_t *first = lv_group_get_first(group);
    if (first) {
        lv_group_focus_obj(first);
    }
}

void nav_create_event_handler(lv_obj_t *tileview, NavContext_t *context)
{
    if (!tileview || !context) {
        return;
    }
    
    /* Event handler for encoder navigation */
    lv_obj_add_event_cb(tileview, [](lv_event_t *e) {
        lv_event_code_t code = lv_event_get_code(e);
        lv_obj_t *tileview = lv_event_get_target(e);
        NavContext_t *ctx = (NavContext_t *)lv_event_get_user_data(e);
        
        if (!ctx) {
            return;
        }
        
        if (code == LV_EVENT_KEY) {
            lv_key_t key = *(lv_key_t *)lv_event_get_param(e);
            
            if (key == LV_KEY_NEXT || key == LV_KEY_RIGHT) {
                ctx->current_tile++;
                if (ctx->current_tile >= ctx->page_count) {
                    if (ctx->mode == NAV_MODE_CIRCULAR) {
                        ctx->current_tile = 0;
                    } else {
                        ctx->current_tile = ctx->page_count - 1;
                    }
                }
                nav_set_tile(tileview, 0, ctx->current_tile, ctx->animation_enabled);
            } else if (key == LV_KEY_PREV || key == LV_KEY_LEFT) {
                if (ctx->current_tile > 0) {
                    ctx->current_tile--;
                } else if (ctx->mode == NAV_MODE_CIRCULAR) {
                    ctx->current_tile = ctx->page_count - 1;
                }
                nav_set_tile(tileview, 0, ctx->current_tile, ctx->animation_enabled);
            }
        }
    }, LV_EVENT_ALL, context);
}

void nav_handle_rotate_up(lv_obj_t *tileview, NavContext_t *context)
{
    (void)tileview;
    if (!context) {
        return;
    }
    
    context->current_tile++;
    if (context->current_tile >= context->page_count) {
        if (context->mode == NAV_MODE_CIRCULAR) {
            context->current_tile = 0;
        } else {
            context->current_tile = context->page_count - 1;
        }
    }
    
    nav_set_tile(tileview, 0, context->current_tile, context->animation_enabled);
}

void nav_handle_rotate_down(lv_obj_t *tileview, NavContext_t *context)
{
    (void)tileview;
    if (!context) {
        return;
    }
    
    if (context->current_tile > 0) {
        context->current_tile--;
    } else if (context->mode == NAV_MODE_CIRCULAR) {
        context->current_tile = context->page_count - 1;
    }
    
    nav_set_tile(tileview, 0, context->current_tile, context->animation_enabled);
}

void nav_handle_click(lv_obj_t *tileview, NavContext_t *context)
{
    (void)tileview;
    (void)context;
    /* Click typically selects/focuses current tile */
    nav_focus_first(tileview);
}

const char *nav_get_mode_name(NavMode_t mode)
{
    if (mode < NAV_MODE_COUNT) {
        return mode_names[mode];
    }
    return "UNKNOWN";
}

const char *nav_get_direction_name(NavDirection_t dir)
{
    if (dir < NAV_DIR_COUNT) {
        return direction_names[dir];
    }
    return "UNKNOWN";
}

void nav_print_context(NavContext_t *context)
{
    if (!context) {
        printf("NavContext: NULL\n");
        return;
    }
    
    printf("NavContext:\n");
    printf("  Mode: %s\n", nav_get_mode_name(context->mode));
    printf("  Current Tile: %u\n", context->current_tile);
    printf("  Page Count: %u\n", context->page_count);
    printf("  Animation: %s\n", context->animation_enabled ? "enabled" : "disabled");
    printf("  Animation Time: %lu ms\n", context->animation_time);
}
