/**
 * @file test_runner.c
 * PlatformIO Unity test runner for T-Lora-Pager.
 */

#include "unity.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Test function to verify basic functionality
void test_basic_functionality(void) {
    int a = 5;
    int b = 10;
    int result = a + b;
    
    TEST_ASSERT_EQUAL_INT(15, result);
}

// Main test entry point - Unity framework requires this
void setUp(void) {
    // Set up before each test
}

void tearDown(void) {
    // Tear down after each test
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_basic_functionality);
    
    return UNITY_END();
}
