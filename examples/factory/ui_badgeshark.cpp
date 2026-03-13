/**
 * @file      ui_badgeshark.cpp
 * @brief     BadgeShark - Badge Management System
 * @details   Manage attendee badges with QR code scanning and badge creation
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

static const char* BADGE_DATA_FILE = "/badges.json";

// Badge structure
typedef struct {
    char name[64];
    char company[64];
    char role[32];
    char email[128];
    uint8_t qr_code[256];
    uint8_t qr_len;
    bool is_scanned;
} Badge_t;

// Global badge storage
static Badge_t badges[50];
static uint16_t badge_count = 0;
static uint16_t selected_badge = 0;

// UI States
static enum {
    BADGE_STATE_LIST,
    BADGE_STATE_VIEW,
    BADGE_STATE_SCAN,
    BADGE_STATE_CREATE
} badge_state = BADGE_STATE_LIST;

// Forward declarations
static void badge_render_list(void);
static void badge_render_view(void);
static void badge_render_scan(void);
static void badge_render_create(void);
static void badge_back(void);
static void badge_select(uint16_t idx);
static void badge_create_new(void);

// Badge management functions
static void badge_init(void) {
    memset(badges, 0, sizeof(badges));
    badge_count = 0;
    selected_badge = 0;
    badge_state = BADGE_STATE_LIST;
}

static void badge_load_from_file(void) {
#ifdef NATIVE_BUILD
    FILE* fp = fopen(BADGE_DATA_FILE, "r");
    if (fp) {
        fscanf(fp, "%d", &badge_count);
        for (uint16_t i = 0; i < badge_count && i < 50; i++) {
            fscanf(fp, "%s %s %s %s %d", 
                   badges[i].name, badges[i].company, 
                   badges[i].role, badges[i].email, &badges[i].qr_len);
            badges[i].is_scanned = true;
        }
        fclose(fp);
    }
#else
    // TODO: Load from SPIFFS
    (void)fp;
#endif
}

static void badge_save_to_file(void) {
#ifdef NATIVE_BUILD
    FILE* fp = fopen(BADGE_DATA_FILE, "w");
    if (fp) {
        fprintf(fp, "%d\n", badge_count);
        for (uint16_t i = 0; i < badge_count; i++) {
            fprintf(fp, "%s %s %s %s %d\n",
                    badges[i].name, badges[i].company,
                    badges[i].role, badges[i].email, badges[i].qr_len);
        }
        fclose(fp);
    }
#else
    // TODO: Save to SPIFFS
    (void)fp;
#endif
}

static void badge_add(const char* name, const char* company, const char* role, const char* email) {
    if (badge_count >= 50) return;
    
    Badge_t* b = &badges[badge_count];
    strncpy(b->name, name, sizeof(b->name) - 1);
    strncpy(b->company, company, sizeof(b->company) - 1);
    strncpy(b->role, role, sizeof(b->role) - 1);
    strncpy(b->email, email, sizeof(b->email) - 1);
    b->qr_len = 0;
    b->is_scanned = false;
    badge_count++;
}

static void badge_render_list(void) {
    lv_obj_t* scr = lv_scr_act();
    
    // Title
    lv_obj_t* title = lv_label_create(scr, NULL);
    lv_label_set_text(title, "BadgeShark");
    lv_obj_set_style_local_font(title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_28);
    lv_obj_set_pos(title, 10, 5);
    
    // Back button (top-left)
    lv_obj_t* back_btn = lv_btn_create(scr, NULL);
    lv_obj_set_pos(back_btn, 5, 5);
    lv_obj_set_size(back_btn, 40, 40);
    lv_obj_set_style_bg_color(back_btn, LV_COLOR_BLACK, 0);
    lv_obj_set_style_border_width(back_btn, 0, 0);
    lv_obj_set_style_pad_all(back_btn, 10, 0);
    lv_obj_add_flag(back_btn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_t* back_label = lv_label_create(back_btn, NULL);
    lv_label_set_text(back_label, "←");
    lv_obj_set_style_local_text_color(back_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_font(back_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_24);
    lv_obj_align(back_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(back_btn, [](lv_event_t* e) {
        lv_obj_t* btn = lv_event_get_target(e);
        (void)btn;
        badge_back();
    }, LV_EVENT_CLICKED, NULL);
    
    // Badge list container
    lv_obj_t* list = lv_list_create(scr, NULL);
    lv_obj_set_size(list, 300, 200);
    lv_obj_align(list, LV_ALIGN_CENTER, 0, 30);
    
    // Add badges to list
    for (uint16_t i = 0; i < badge_count; i++) {
        Badge_t* b = &badges[i];
        lv_obj_t* btn = lv_list_add_btn(list, LV_SYMBOL_FILE, b->name);
        if (b->is_scanned) {
            lv_obj_t* icon = lv_list_get_btn(list, btn);
            lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
        }
    }
    
    // Create button
    lv_obj_t* create_btn = lv_btn_create(scr, NULL);
    lv_obj_set_size(create_btn, 150, 50);
    lv_obj_align(create_btn, LV_ALIGN_CENTER, 0, 150);
    lv_obj_set_style_bg_color(create_btn, LV_COLOR_BLUE, 0);
    lv_obj_t* create_label = lv_label_create(create_btn, NULL);
    lv_label_set_text(create_label, "Create Badge");
    lv_obj_align(create_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(create_btn, [](lv_event_t* e) {
        lv_obj_t* btn = lv_event_get_target(e);
        (void)btn;
        badge_create_new();
    }, LV_EVENT_CLICKED, NULL);
}

static void badge_render_view(void) {
    lv_obj_t* scr = lv_scr_act();
    
    // Title
    lv_obj_t* title = lv_label_create(scr, NULL);
    lv_label_set_text(title, "Badge Details");
    lv_obj_set_style_local_font(title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_28);
    lv_obj_set_pos(title, 10, 5);
    
    // Back button (top-left)
    lv_obj_t* back_btn = lv_btn_create(scr, NULL);
    lv_obj_set_pos(back_btn, 5, 5);
    lv_obj_set_size(back_btn, 40, 40);
    lv_obj_set_style_bg_color(back_btn, LV_COLOR_BLACK, 0);
    lv_obj_set_style_border_width(back_btn, 0, 0);
    lv_obj_set_style_pad_all(back_btn, 10, 0);
    lv_obj_t* back_label = lv_label_create(back_btn, NULL);
    lv_label_set_text(back_label, "←");
    lv_obj_set_style_local_text_color(back_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_font(back_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_24);
    lv_obj_align(back_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(back_btn, [](lv_event_t* e) {
        lv_obj_t* btn = lv_event_get_target(e);
        (void)btn;
        badge_back();
    }, LV_EVENT_CLICKED, NULL);
    
    if (selected_badge < badge_count) {
        Badge_t* b = &badges[selected_badge];
        
        // Name
        lv_obj_t* name_label = lv_label_create(scr, NULL);
        lv_label_set_text(name_label, b->name);
        lv_obj_set_style_local_font(name_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_24);
        lv_obj_align(name_label, LV_ALIGN_CENTER, 0, 30);
        
        // Company
        lv_obj_t* company_label = lv_label_create(scr, NULL);
        lv_label_set_text(company_label, b->company);
        lv_obj_set_style_local_text_color(company_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLUE);
        lv_obj_align(company_label, LV_ALIGN_CENTER, 0, 60);
        
        // Role
        lv_obj_t* role_label = lv_label_create(scr, NULL);
        lv_label_set_text(role_label, b->role);
        lv_obj_align(role_label, LV_ALIGN_CENTER, 0, 90);
        
        // Email
        lv_obj_t* email_label = lv_label_create(scr, NULL);
        lv_label_set_text(email_label, b->email);
        lv_obj_align(email_label, LV_ALIGN_CENTER, 0, 120);
        lv_obj_set_style_local_text_color(email_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    }
}

static void badge_render_scan(void) {
    lv_obj_t* scr = lv_scr_act();
    
    // Title
    lv_obj_t* title = lv_label_create(scr, NULL);
    lv_label_set_text(title, "Scan QR Code");
    lv_obj_set_style_local_font(title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_28);
    lv_obj_set_pos(title, 10, 5);
    
    // Back button (top-left)
    lv_obj_t* back_btn = lv_btn_create(scr, NULL);
    lv_obj_set_pos(back_btn, 5, 5);
    lv_obj_set_size(back_btn, 40, 40);
    lv_obj_set_style_bg_color(back_btn, LV_COLOR_BLACK, 0);
    lv_obj_set_style_border_width(back_btn, 0, 0);
    lv_obj_set_style_pad_all(back_btn, 10, 0);
    lv_obj_t* back_label = lv_label_create(back_btn, NULL);
    lv_label_set_text(back_label, "←");
    lv_obj_set_style_local_text_color(back_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_font(back_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_24);
    lv_obj_align(back_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(back_btn, [](lv_event_t* e) {
        lv_obj_t* btn = lv_event_get_target(e);
        (void)btn;
        badge_back();
    }, LV_EVENT_CLICKED, NULL);
    
    // QR scan area
    lv_obj_t* qr_area = lv_obj_create(scr, NULL);
    lv_obj_set_size(qr_area, 200, 200);
    lv_obj_align(qr_area, LV_ALIGN_CENTER, 0, 50);
    lv_obj_set_style_bg_color(qr_area, LV_COLOR_BLACK, 0);
    lv_obj_set_style_border_width(qr_area, 2, 0);
    lv_obj_set_style_border_color(qr_area, LV_COLOR_WHITE, 0);
    
    lv_obj_t* status_label = lv_label_create(scr, NULL);
    lv_label_set_text(status_label, "Scanning...");
    lv_obj_align(status_label, LV_ALIGN_CENTER, 0, 180);
}

static void badge_render_create(void) {
    lv_obj_t* scr = lv_scr_act();
    
    // Title
    lv_obj_t* title = lv_label_create(scr, NULL);
    lv_label_set_text(title, "Create Badge");
    lv_obj_set_style_local_font(title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_28);
    lv_obj_set_pos(title, 10, 5);
    
    // Back button (top-left)
    lv_obj_t* back_btn = lv_btn_create(scr, NULL);
    lv_obj_set_pos(back_btn, 5, 5);
    lv_obj_set_size(back_btn, 40, 40);
    lv_obj_set_style_bg_color(back_btn, LV_COLOR_BLACK, 0);
    lv_obj_set_style_border_width(back_btn, 0, 0);
    lv_obj_set_style_pad_all(back_btn, 10, 0);
    lv_obj_t* back_label = lv_label_create(back_btn, NULL);
    lv_label_set_text(back_label, "←");
    lv_obj_set_style_local_text_color(back_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_font(back_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_24);
    lv_obj_align(back_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(back_btn, [](lv_event_t* e) {
        lv_obj_t* btn = lv_event_get_target(e);
        (void)btn;
        badge_back();
    }, LV_EVENT_CLICKED, NULL);
    
    // Form fields
    lv_obj_t* name_label = lv_label_create(scr, NULL);
    lv_label_set_text(name_label, "Name:");
    lv_obj_align(name_label, LV_ALIGN_CENTER, -100, 50);
    
    lv_obj_t* name_input = lv_textarea_create(scr, NULL);
    lv_obj_set_size(name_input, 200, 40);
    lv_obj_align(name_input, LV_ALIGN_CENTER, 0, 50);
    lv_obj_set_placeholder_text(name_input, "Enter name");
    
    lv_obj_t* company_label = lv_label_create(scr, NULL);
    lv_label_set_text(company_label, "Company:");
    lv_obj_align(company_label, LV_ALIGN_CENTER, -100, 100);
    
    lv_obj_t* company_input = lv_textarea_create(scr, NULL);
    lv_obj_set_size(company_input, 200, 40);
    lv_obj_align(company_input, LV_ALIGN_CENTER, 0, 100);
    lv_obj_set_placeholder_text(company_input, "Enter company");
    
    lv_obj_t* role_label = lv_label_create(scr, NULL);
    lv_label_set_text(role_label, "Role:");
    lv_obj_align(role_label, LV_ALIGN_CENTER, -100, 150);
    
    lv_obj_t* role_input = lv_textarea_create(scr, NULL);
    lv_obj_set_size(role_input, 200, 40);
    lv_obj_align(role_input, LV_ALIGN_CENTER, 0, 150);
    lv_obj_set_placeholder_text(role_input, "Enter role");
    
    // Save button
    lv_obj_t* save_btn = lv_btn_create(scr, NULL);
    lv_obj_set_size(save_btn, 150, 50);
    lv_obj_align(save_btn, LV_ALIGN_CENTER, 0, 210);
    lv_obj_set_style_bg_color(save_btn, LV_COLOR_GREEN, 0);
    lv_obj_t* save_label = lv_label_create(save_btn, NULL);
    lv_label_set_text(save_label, "Save");
    lv_obj_align(save_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(save_btn, [](lv_event_t* e) {
        lv_obj_t* btn = lv_event_get_target(e);
        (void)btn;
        // Get text from inputs and create badge
        lv_textarea_clear_all(name_input);
        lv_textarea_clear_all(company_input);
        lv_textarea_clear_all(role_input);
        badge_state = BADGE_STATE_LIST;
        badge_render_list();
    }, LV_EVENT_CLICKED, NULL);
}

static void badge_back(void) {
    badge_state = BADGE_STATE_LIST;
    badge_render_list();
}

static void badge_select(uint16_t idx) {
    if (idx < badge_count) {
        selected_badge = idx;
        badge_state = BADGE_STATE_VIEW;
        badge_render_view();
    }
}

static void badge_create_new(void) {
    badge_state = BADGE_STATE_CREATE;
    badge_render_create();
}

// Main UI functions
void ui_badgeshark_init(void) {
    badge_init();
    badge_load_from_file();
    badge_render_list();
}

void ui_badgeshark_tick(void) {
    // Handle UI updates
    (void)ui_badgeshark_tick;
}

void ui_badgeshark_cleanup(void) {
    badge_save_to_file();
    badge_init();
}

// Event handlers
void ui_badgeshark_on_click(uint8_t button) {
    (void)button;
    // Handle button clicks
}

void ui_badgeshark_on_scroll(int16_t delta) {
    (void)delta;
    // Handle scroll events
}

void ui_badgeshark_on_scan(const char* qr_data) {
    if (qr_data && badge_count < 50) {
        // Parse QR data and create badge
        char name[64] = "Scanned";
        char company[64] = "Unknown";
        char role[32] = "Attendee";
        char email[128] = "scan@example.com";
        
        badge_add(name, company, role, email);
        badge_state = BADGE_STATE_VIEW;
        badge_render_view();
    }
}

#ifdef NATIVE_BUILD
// Test functions
static void test_badge_basic(void) {
    badge_init();
    TEST_ASSERT_EQUAL(0, badge_count);
    
    badge_add("John Doe", "Tech Corp", "Engineer", "john@example.com");
    TEST_ASSERT_EQUAL(1, badge_count);
    TEST_ASSERT_EQUAL_STRING("John Doe", badges[0].name);
    
    badge_add("Jane Smith", "Startup Inc", "Designer", "jane@example.com");
    TEST_ASSERT_EQUAL(2, badge_count);
    
    printf("Badge tests passed\n");
}

static void test_badge_limits(void) {
    badge_init();
    
    // Add 50 badges
    for (int i = 0; i < 50; i++) {
        char name[64];
        snprintf(name, sizeof(name), "User %d", i);
        badge_add(name, "Test Corp", "User", "test@example.com");
    }
    
    TEST_ASSERT_EQUAL(50, badge_count);
    
    // Try to add one more (should fail)
    badge_add("Overflow", "Bad Corp", "Bad", "bad@example.com");
    TEST_ASSERT_EQUAL(50, badge_count);
    
    printf("Badge limit tests passed\n");
}

void ui_badgeshark_run_tests(void) {
    test_badge_basic();
    test_badge_limits();
}
#endif
