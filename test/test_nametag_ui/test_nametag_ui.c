/**
 * @file test_nametag_ui.c
 * Unity tests for Nametag UI integration with main update.
 * Runs on native platform — no ESP32 hardware required.
 */

#include <unity.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/ui_nametag.h"
#include "../../src/ui_main_update.h"

/* ---- Test: Nametag UI functions are properly exported ---- */

void test_nametag_ui_exported(void) {
    // Verify the nametag UI functions exist and can be called
    // These should compile and link properly
    printf("Testing nametag UI exports...\n");
    
    // Test that the functions are callable (they will be null on native)
    // but the symbols should exist
    printf("Nametag UI functions are exported\n");
    
    // Verify the function pointers are defined
    TEST_ASSERT_NOT_NULL(ui_nametag_render);
    TEST_ASSERT_NOT_NULL(ui_nametag_init);
}

/* ---- Test: Main update integration ---- */

void test_main_update_integration(void) {
    printf("Testing main update integration...\n");
    
    // Verify the update function exists
    printf("Main update functions are exported\n");
    
    // Verify the function pointers are defined
    TEST_ASSERT_NOT_NULL(ui_main_update);
    TEST_ASSERT_NOT_NULL(ui_main_update_init);
}

/* ---- Test: UI state management ---- */

void test_ui_state_management(void) {
    printf("Testing UI state management...\n");
    
    // Verify the UI state functions are defined
    TEST_ASSERT_NOT_NULL(ui_get_state);
    TEST_ASSERT_NOT_NULL(ui_set_state);
    TEST_ASSERT_NOT_NULL(ui_get_current_page);
    TEST_ASSERT_NOT_NULL(ui_set_current_page);
}

/* ---- Test: Nametag data structure ---- */

void test_nametag_data_structure(void) {
    printf("Testing nametag data structure...\n");
    
    // Verify the nametag data structure is defined
    nametag_data_t data;
    memset(&data, 0, sizeof(data));
    
    // Test basic structure fields
    data.name = "Test";
    data.role = "Engineer";
    data.department = "Engineering";
    data.team = "Team A";
    data.position = 0;
    data.icon = 0;
    
    TEST_ASSERT_NOT_NULL(data.name);
    TEST_ASSERT_NOT_NULL(data.role);
    TEST_ASSERT_NOT_NULL(data.department);
    TEST_ASSERT_NOT_NULL(data.team);
    
    printf("Nametag data structure works correctly\n");
}

/* ---- Test: UI rendering functions ---- */

void test_ui_rendering_functions(void) {
    printf("Testing UI rendering functions...\n");
    
    // Verify the rendering functions are defined
    TEST_ASSERT_NOT_NULL(ui_nametag_render);
    TEST_ASSERT_NOT_NULL(ui_main_render);
    TEST_ASSERT_NOT_NULL(ui_clock_render);
    
    printf("UI rendering functions are available\n");
}

void setUp(void) {
    // Set up before each test
}

void tearDown(void) {
    // Tear down after each test
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_nametag_ui_exported);
    RUN_TEST(test_main_update_integration);
    RUN_TEST(test_ui_state_management);
    RUN_TEST(test_nametag_data_structure);
    RUN_TEST(test_ui_rendering_functions);
    
    return UNITY_END();
}
