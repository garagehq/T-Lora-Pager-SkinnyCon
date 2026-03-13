/**
 * @file      ui_nametag.cpp
 * @brief     Nametag - Digital Name Tag Display
 * @details   Display attendee name tags with QR codes and company info
 */

#include "ui_define.h"
#include "ui_main.h"

#ifdef NATIVE_BUILD
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#else
#include <Arduino.h>
#endif

static const char* NAMETAG_DATA_FILE = "/nametags.json";

// Nametag structure
typedef struct {
    char name[64];
    char company[64];
    char role[32];
    char email[128];
    uint8_t qr_code[256];
    uint8_t qr_len;
    uint8_t color_theme;
} Nametag_t;

// Global nametag storage
static Nametag_t nametags[20];
static uint16_t nametag_count = 0;
static uint16_t selected_nametag = 0;

// UI States
static enum {
    NAMETAG_STATE_LIST,
    NAMETAG_STATE_VIEW,
    NAMETAG_STATE_CREATE
} nametag_state = NAMETAG_STATE_LIST;

// Forward declarations
static void nametag_render_list(void);
static void nametag_render_view(void);
static void nametag_render_create(void);
static void nametag_back(void);
static void nametag_select(uint16_t idx);
static void nametag_create_new(void);

// Nametag management functions
static void nametag_init(void) {
    memset(nametags, 0, sizeof(nametags));
    nametag_count = 0;
    selected_nametag = 0;
    nametag_state = NAMETAG_STATE_LIST;
}

static void nametag_load_from_file(void) {
#ifdef NATIVE_BUILD
    FILE* fp = fopen(NAMETAG_DATA_FILE, "r");
    if (fp) {
        fscanf(fp, "%d", &nametag_count);
        for (uint16_t i = 0; i < nametag_count && i < 20; i++) {
            fscanf(fp, "%s %s %s %s %d %d",
                   nametags[i].name, nametags[i].company,
                   nametags[i].role, nametags[i].email,
                   &nametags[i].qr_len, &nametags[i].color_theme);
        }
        fclose(fp);
    }
#else
    // TODO: Load from SPIFFS
    (void)fp;
#endif
}

static void nametag_save_to_file(void) {
#ifdef NATIVE_BUILD
    FILE* fp = fopen(NAMETAG_DATA_FILE, "w");
    if (fp) {
        fprintf(fp, "%d\n", nametag_count);
        for (uint16_t i = 0; i < nametag_count; i++) {
            fprintf(fp, "%s %s %s %s %d %d\n",
                    nametags[i].name, nametags[i].company,
                    nametags[i].role, nametags[i].email,
                    nametags[i].qr_len, nametags[i].color_theme);
        }
        fclose(fp);
    }
#else
    // TODO: Save to SPIFFS
    (void)fp;
#endif
}

static void nametag_add(const char* name, const char* company, const char* role, const char* email, uint8_t theme) {
    if (nametag_count >= 20) return;
    
    Nametag_t* n = &nametags[nametag_count];
    strncpy(n->name, name, sizeof(n->name) - 1);
    strncpy(n->company, company, sizeof(n->company) - 1);
    strncpy(n->role, role, sizeof(n->role) - 1);
    strncpy(n->email, email, sizeof(n->email) - 1);
    n->color_theme = theme;
    n->qr_len = 0;
    nametag_count++;
    nametag_save_to_file();
}

static void nametag_render_list(void) {
#ifdef NATIVE_BUILD
    printf("=== Nametag List ===\n");
    for (uint16_t i = 0; i < nametag_count; i++) {
        printf("%d. %s - %s\n", i, nametags[i].name, nametags[i].company);
    }
    printf("\n[UP] [DOWN] [BACK] [CREATE]\n");
    (void)i;
#else
    // Display list on screen
    lv_obj_t* scr = lv_scr_act();
    lv_obj_t* list = lv_list_create(scr);
    lv_obj_set_size(list, LV_PCT(100), LV_PCT(100));
    lv_obj_align(list, LV_ALIGN_TOP_MID, 0, 40);
    
    for (uint16_t i = 0; i < nametag_count; i++) {
        lv_obj_t* btn = lv_list_add_btn(list, LV_SYMBOL_FILE, "Nametag");
        lv_btn_set_checkable(btn, false);
        lv_obj_set_user_data(btn, (void*)(uintptr_t)i);
    }
    
    lv_obj_t* back_btn = lv_btn_create(scr);
    lv_obj_set_size(back_btn, 80, 40);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_obj_t* label = lv_label_create(back_btn);
    lv_label_set_text(label, "< Back");
    lv_obj_center(label);
    lv_btn_set_checkable(back_btn, false);
    lv_obj_add_event_cb(back_btn, [](lv_event_t* e) {
        lv_obj_t* btn = lv_event_get_target(e);
        (void)btn;
        nametag_back();
    }, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t* create_btn = lv_btn_create(scr);
    lv_obj_set_size(create_btn, 80, 40);
    lv_obj_align(create_btn, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    label = lv_label_create(create_btn);
    lv_label_set_text(label, "+ New");
    lv_obj_center(label);
    lv_btn_set_checkable(create_btn, false);
    lv_obj_add_event_cb(create_btn, [](lv_event_t* e) {
        lv_obj_t* btn = lv_event_get_target(e);
        (void)btn;
        nametag_create_new();
    }, LV_EVENT_CLICKED, NULL);
#endif
}

static void nametag_render_view(void) {
#ifdef NATIVE_BUILD
    if (selected_nametag < nametag_count) {
        Nametag_t* n = &nametags[selected_nametag];
        printf("=== Nametag Details ===\n");
        printf("Name: %s\n", n->name);
        printf("Company: %s\n", n->company);
        printf("Role: %s\n", n->role);
        printf("Email: %s\n", n->email);
        printf("Theme: %d\n", n->color_theme);
        printf("\n[UP] [DOWN] [BACK]\n");
    }
#else
    // Display nametag details on screen
    lv_obj_t* scr = lv_scr_act();
    lv_obj_t* label = lv_label_create(scr);
    lv_label_set_text(label, "Nametag View");
    lv_obj_center(label);
    
    lv_obj_t* back_btn = lv_btn_create(scr);
    lv_obj_set_size(back_btn, 80, 40);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    label = lv_label_create(back_btn);
    lv_label_set_text(label, "< Back");
    lv_obj_center(label);
    lv_btn_set_checkable(back_btn, false);
    lv_obj_add_event_cb(back_btn, [](lv_event_t* e) {
        lv_obj_t* btn = lv_event_get_target(e);
        (void)btn;
        nametag_back();
    }, LV_EVENT_CLICKED, NULL);
#endif
}

static void nametag_render_create(void) {
#ifdef NATIVE_BUILD
    printf("=== Create Nametag ===\n");
    printf("Enter name, company, role, email\n");
    printf("\n[UP] [DOWN] [BACK] [SAVE]\n");
#else
    // Display create form on screen
    lv_obj_t* scr = lv_scr_act();
    lv_obj_t* label = lv_label_create(scr);
    lv_label_set_text(label, "Create Nametag");
    lv_obj_center(label);
    
    lv_obj_t* back_btn = lv_btn_create(scr);
    lv_obj_set_size(back_btn, 80, 40);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    label = lv_label_create(back_btn);
    lv_label_set_text(label, "< Back");
    lv_obj_center(label);
    lv_btn_set_checkable(back_btn, false);
    lv_obj_add_event_cb(back_btn, [](lv_event_t* e) {
        lv_obj_t* btn = lv_event_get_target(e);
        (void)btn;
        nametag_back();
    }, LV_EVENT_CLICKED, NULL);
#endif
}

static void nametag_back(void) {
    if (nametag_state == NAMETAG_STATE_VIEW) {
        nametag_state = NAMETAG_STATE_LIST;
        nametag_render_list();
    } else if (nametag_state == NAMETAG_STATE_CREATE) {
        nametag_state = NAMETAG_STATE_LIST;
        nametag_render_list();
    }
}

static void nametag_select(uint16_t idx) {
    if (idx < nametag_count) {
        selected_nametag = idx;
        nametag_state = NAMETAG_STATE_VIEW;
        nametag_render_view();
    }
}

static void nametag_create_new(void) {
    nametag_state = NAMETAG_STATE_CREATE;
    nametag_render_create();
}

// Public API
void ui_badge_shark_init(void) {
    nametag_init();
    nametag_load_from_file();
    nametag_render_list();
}

void ui_badge_shark_update(void) {
    (void)ui_badge_shark_update;
}

void ui_badge_shark_input(uint8_t input) {
    switch (input) {
        case BTN_UP:
            if (nametag_state == NAMETAG_STATE_LIST && nametag_count > 0) {
                selected_nametag = (selected_nametag + nametag_count - 1) % nametag_count;
                nametag_render_list();
            }
            break;
        case BTN_DOWN:
            if (nametag_state == NAMETAG_STATE_LIST && nametag_count > 0) {
                selected_nametag = (selected_nametag + 1) % nametag_count;
                nametag_render_list();
            }
            break;
        case BTN_SELECT:
            if (nametag_state == NAMETAG_STATE_LIST) {
                nametag_select(selected_nametag);
            }
            break;
        case BTN_BACK:
            nametag_back();
            break;
        case BTN_RIGHT:
            if (nametag_state == NAMETAG_STATE_LIST) {
                nametag_create_new();
            }
            break;
        default:
            break;
    }
}

#ifdef NATIVE_BUILD
// Unity test helpers
int nametag_test_get_count(void) {
    return nametag_count;
}

const Nametag_t* nametag_test_get(int idx) {
    if (idx < 0 || idx >= nametag_count) return NULL;
    return &nametags[idx];
}

void nametag_test_add(const char* name, const char* company) {
    nametag_add(name, company, "", "", 0);
}

void nametag_test_clear(void) {
    nametag_count = 0;
    memset(nametags, 0, sizeof(nametags));
}
#endif
