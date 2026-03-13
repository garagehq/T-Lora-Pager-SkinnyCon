/**
 * @file test_simple.c
 * Simple test for T-Lora-Pager - no Unity framework needed.
 */

#include <stdio.h>
#include <stdint.h>

// Simple test function
int test_addition(int a, int b) {
    return a + b;
}

// Main test entry point
int main() {
    int result = test_addition(2, 3);
    if (result == 5) {
        printf("PASS: test_addition(2, 3) = 5\n");
        return 0;
    } else {
        printf("FAIL: test_addition(2, 3) = %d\n", result);
        return 1;
    }
}
