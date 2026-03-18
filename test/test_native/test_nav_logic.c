/**
 * @file test_nav_logic.c
 * Unity tests for navigation logic helpers (standalone logic tests).
 * 
 * Tests navigation mode logic, state transitions, and calculations.
 * Runs on native platform — no LVGL hardware required.
 */

#include <unity.h>
#include <stdint.h>
#include <string.h>

/* Navigation logic constants and enums (simplified for testing) */
typedef enum {
    NAV_MODE_DIRECT,
    NAV_MODE_CIRCULAR,
    NAV_MODE_TREE
} NavMode_t;

typedef struct {
    NavMode_t mode;
    uint32_t current_tile;
    uint32_t page_count;
    bool animation_enabled;
} NavigationState_t;

/* Navigation logic implementation (standalone) */
static void nav_init(NavigationState_t *state, NavMode_t mode, uint32_t page_count)
{
    if (!state) return;
    state->mode = mode;
    state->current_tile = 0;
    state->page_count = page_count;
    state->animation_enabled = true;
}

static bool nav_next(NavigationState_t *state)
{
    if (!state || state->page_count == 0) return false;
    
    uint32_t next = state->current_tile + 1;
    if (next >= state->page_count) {
        switch (state->mode) {
            case NAV_MODE_CIRCULAR:
                next = 0;
                break;
            case NAV_MODE_TREE:
            case NAV_MODE_DIRECT:
            default:
                next = state->page_count - 1;
                break;
        }
    }
    state->current_tile = next;
    return true;
}

static bool nav_prev(NavigationState_t *state)
{
    if (!state || state->page_count == 0) return false;
    
    if (state->current_tile == 0) {
        switch (state->mode) {
            case NAV_MODE_CIRCULAR:
                state->current_tile = state->page_count - 1;
                return true;
            case NAV_MODE_TREE:
            case NAV_MODE_DIRECT:
            default:
                return false;
        }
    }
    
    state->current_tile--;
    return true;
}

static bool nav_goto(NavigationState_t *state, uint32_t tile)
{
    if (!state || tile >= state->page_count) return false;
    state->current_tile = tile;
    return true;
}

static void nav_first(NavigationState_t *state)
{
    if (state && state->page_count > 0) {
        state->current_tile = 0;
    }
}

static void nav_last(NavigationState_t *state)
{
    if (state && state->page_count > 0) {
        state->current_tile = state->page_count - 1;
    }
}

static const char *nav_get_mode_name(NavMode_t mode)
{
    static const char *names[] = {"DIRECT", "CIRCULAR", "TREE"};
    if (mode < NAV_MODE_TREE + 1) {
        return names[mode];
    }
    return "UNKNOWN";
}

/* ================================================================
 *  TEST SETUP/TEARDOWN
 * ================================================================ */

void setUp(void) {}
void tearDown(void) {}

/* ================================================================
 *  INITIALIZATION TESTS
 * ================================================================ */

void test_nav_init_sets_default_state(void)
{
    NavigationState_t state;
    nav_init(&state, NAV_MODE_DIRECT, 5);
    
    TEST_ASSERT_EQUAL_UINT32(0, state.current_tile);
    TEST_ASSERT_EQUAL_UINT32(5, state.page_count);
    TEST_ASSERT_EQUAL_UINT32(NAV_MODE_DIRECT, state.mode);
    TEST_ASSERT_TRUE(state.animation_enabled);
}

void test_nav_init_handles_zero_pages(void)
{
    NavigationState_t state;
    nav_init(&state, NAV_MODE_CIRCULAR, 0);
    
    TEST_ASSERT_EQUAL_UINT32(0, state.current_tile);
    TEST_ASSERT_EQUAL_UINT32(0, state.page_count);
}

/* ================================================================
 *  NAV NEXT TESTS
 * ================================================================ */

void test_nav_next_direct_mode(void)
{
    NavigationState_t state;
    nav_init(&state, NAV_MODE_DIRECT, 5);
    
    TEST_ASSERT_TRUE(nav_next(&state));
    TEST_ASSERT_EQUAL_UINT32(1, state.current_tile);
    
    TEST_ASSERT_TRUE(nav_next(&state));
    TEST_ASSERT_EQUAL_UINT32(2, state.current_tile);
}

void test_nav_next_reaches_last_in_direct_mode(void)
{
    NavigationState_t state;
    nav_init(&state, NAV_MODE_DIRECT, 5);
    
    nav_next(&state); nav_next(&state); nav_next(&state); nav_next(&state);
    TEST_ASSERT_EQUAL_UINT32(4, state.current_tile);
    
    /* Next should stay at last */
    TEST_ASSERT_TRUE(nav_next(&state));
    TEST_ASSERT_EQUAL_UINT32(4, state.current_tile);
}

void test_nav_next_circular_wraps(void)
{
    NavigationState_t state;
    nav_init(&state, NAV_MODE_CIRCULAR, 5);
    
    /* Navigate to last */
    nav_next(&state); nav_next(&state); nav_next(&state); nav_next(&state);
    TEST_ASSERT_EQUAL_UINT32(4, state.current_tile);
    
    /* Next should wrap to 0 */
    TEST_ASSERT_TRUE(nav_next(&state));
    TEST_ASSERT_EQUAL_UINT32(0, state.current_tile);
}

void test_nav_next_tree_stays_at_last(void)
{
    NavigationState_t state;
    nav_init(&state, NAV_MODE_TREE, 5);
    
    /* Navigate to last */
    nav_next(&state); nav_next(&state); nav_next(&state); nav_next(&state);
    TEST_ASSERT_EQUAL_UINT32(4, state.current_tile);
    
    /* Next should stay at last */
    TEST_ASSERT_TRUE(nav_next(&state));
    TEST_ASSERT_EQUAL_UINT32(4, state.current_tile);
}

void test_nav_next_zero_pages_returns_false(void)
{
    NavigationState_t state;
    nav_init(&state, NAV_MODE_DIRECT, 0);
    
    TEST_ASSERT_FALSE(nav_next(&state));
}

/* ================================================================
 *  NAV PREV TESTS
 * ================================================================ */

void test_nav_prev_direct_mode(void)
{
    NavigationState_t state;
    nav_init(&state, NAV_MODE_DIRECT, 5);
    
    /* First go to last */
    nav_goto(&state, 4);
    TEST_ASSERT_TRUE(nav_prev(&state));
    TEST_ASSERT_EQUAL_UINT32(3, state.current_tile);
}

void test_nav_prev_reaches_first_in_direct_mode(void)
{
    NavigationState_t state;
    nav_init(&state, NAV_MODE_DIRECT, 5);
    
    nav_goto(&state, 4);
    nav_prev(&state); nav_prev(&state); nav_prev(&state);
    TEST_ASSERT_EQUAL_UINT32(1, state.current_tile);
    
    /* Prev should stay at first */
    TEST_ASSERT_FALSE(nav_prev(&state));
    TEST_ASSERT_EQUAL_UINT32(1, state.current_tile);
}

void test_nav_prev_circular_wraps(void)
{
    NavigationState_t state;
    nav_init(&state, NAV_MODE_CIRCULAR, 5);
    
    nav_init(&state, NAV_MODE_CIRCULAR, 5);
    nav_goto(&state, 0);
    
    /* Prev should wrap to last */
    TEST_ASSERT_TRUE(nav_prev(&state));
    TEST_ASSERT_EQUAL_UINT32(4, state.current_tile);
}

void test_nav_prev_zero_pages_returns_false(void)
{
    NavigationState_t state;
    nav_init(&state, NAV_MODE_DIRECT, 0);
    
    TEST_ASSERT_FALSE(nav_prev(&state));
}

/* ================================================================
 *  NAV GOTO TESTS
 * ================================================================ */

void test_nav_goto_valid_tile(void)
{
    NavigationState_t state;
    nav_init(&state, NAV_MODE_DIRECT, 5);
    
    TEST_ASSERT_TRUE(nav_goto(&state, 3));
    TEST_ASSERT_EQUAL_UINT32(3, state.current_tile);
}

void test_nav_goto_invalid_tile(void)
{
    NavigationState_t state;
    nav_init(&state, NAV_MODE_DIRECT, 5);
    
    TEST_ASSERT_FALSE(nav_goto(&state, 10));  /* Out of range */
    TEST_ASSERT_EQUAL_UINT32(0, state.current_tile);  /* Unchanged */
}

void test_nav_goto_reaches_last(void)
{
    NavigationState_t state;
    nav_init(&state, NAV_MODE_DIRECT, 5);
    
    TEST_ASSERT_TRUE(nav_goto(&state, 4));
    TEST_ASSERT_EQUAL_UINT32(4, state.current_tile);
}

/* ================================================================
 *  NAV FIRST/LAST TESTS
 * ================================================================ */

void test_nav_first_sets_to_zero(void)
{
    NavigationState_t state;
    nav_init(&state, NAV_MODE_DIRECT, 5);
    
    nav_goto(&state, 4);
    nav_first(&state);
    
    TEST_ASSERT_EQUAL_UINT32(0, state.current_tile);
}

void test_nav_last_sets_to_page_count_minus_one(void)
{
    NavigationState_t state;
    nav_init(&state, NAV_MODE_DIRECT, 5);
    
    nav_first(&state);
    nav_last(&state);
    
    TEST_ASSERT_EQUAL_UINT32(4, state.current_tile);
}

void test_nav_first_zero_pages(void)
{
    NavigationState_t state;
    nav_init(&state, NAV_MODE_DIRECT, 0);
    
    nav_first(&state);
    TEST_ASSERT_EQUAL_UINT32(0, state.current_tile);
}

void test_nav_last_zero_pages(void)
{
    NavigationState_t state;
    nav_init(&state, NAV_MODE_DIRECT, 0);
    
    nav_last(&state);
    TEST_ASSERT_EQUAL_UINT32(0, state.current_tile);
}

/* ================================================================
 *  UTILITY FUNCTION TESTS
 * ================================================================ */

void test_get_mode_name_direct(void)
{
    TEST_ASSERT_EQUAL_STRING("DIRECT", nav_get_mode_name(NAV_MODE_DIRECT));
}

void test_get_mode_name_circular(void)
{
    TEST_ASSERT_EQUAL_STRING("CIRCULAR", nav_get_mode_name(NAV_MODE_CIRCULAR));
}

void test_get_mode_name_tree(void)
{
    TEST_ASSERT_EQUAL_STRING("TREE", nav_get_mode_name(NAV_MODE_TREE));
}

void test_get_mode_name_unknown(void)
{
    TEST_ASSERT_EQUAL_STRING("UNKNOWN", nav_get_mode_name((NavMode_t)99));
}

/* ================================================================
 *  COMPLEX NAVIGATION SCENARIOS
 * ================================================================ */

void test_circular_navigation_cycle(void)
{
    NavigationState_t state;
    nav_init(&state, NAV_MODE_CIRCULAR, 3);
    
    /* Cycle through all tiles */
    nav_next(&state);  /* 0 -> 1 */
    nav_next(&state);  /* 1 -> 2 */
    nav_next(&state);  /* 2 -> 0 (wrap) */
    
    TEST_ASSERT_EQUAL_UINT32(0, state.current_tile);
    TEST_ASSERT_TRUE(nav_next(&state));  /* 0 -> 1 */
    TEST_ASSERT_EQUAL_UINT32(1, state.current_tile);
}

void test_direct_navigation_roundtrip(void)
{
    NavigationState_t state;
    nav_init(&state, NAV_MODE_DIRECT, 5);
    
    nav_goto(&state, 3);
    
    nav_prev(&state); nav_prev(&state);  /* 3 -> 1 */
    nav_next(&state); nav_next(&state);  /* 1 -> 3 */
    
    TEST_ASSERT_EQUAL_UINT32(3, state.current_tile);
}

void test_tree_navigation_stays_at_bounds(void)
{
    NavigationState_t state;
    nav_init(&state, NAV_MODE_TREE, 5);
    
    /* Go to end */
    for (int i = 0; i < 10; i++) {
        nav_next(&state);
    }
    TEST_ASSERT_EQUAL_UINT32(4, state.current_tile);
    
    /* Prev should also stay at end */
    for (int i = 0; i < 10; i++) {
        nav_prev(&state);
    }
    TEST_ASSERT_EQUAL_UINT32(0, state.current_tile);
}

/* ================================================================
 *  MAIN
 * ================================================================ */

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    
    /* Initialization tests */
    RUN_TEST(test_nav_init_sets_default_state);
    RUN_TEST(test_nav_init_handles_zero_pages);
    
    /* Nav next tests */
    RUN_TEST(test_nav_next_direct_mode);
    RUN_TEST(test_nav_next_reaches_last_in_direct_mode);
    RUN_TEST(test_nav_next_circular_wraps);
    RUN_TEST(test_nav_next_tree_stays_at_last);
    RUN_TEST(test_nav_next_zero_pages_returns_false);
    
    /* Nav prev tests */
    RUN_TEST(test_nav_prev_direct_mode);
    RUN_TEST(test_nav_prev_reaches_first_in_direct_mode);
    RUN_TEST(test_nav_prev_circular_wraps);
    RUN_TEST(test_nav_prev_zero_pages_returns_false);
    
    /* Nav goto tests */
    RUN_TEST(test_nav_goto_valid_tile);
    RUN_TEST(test_nav_goto_invalid_tile);
    RUN_TEST(test_nav_goto_reaches_last);
    
    /* Nav first/last tests */
    RUN_TEST(test_nav_first_sets_to_zero);
    RUN_TEST(test_nav_last_sets_to_page_count_minus_one);
    RUN_TEST(test_nav_first_zero_pages);
    RUN_TEST(test_nav_last_zero_pages);
    
    /* Utility function tests */
    RUN_TEST(test_get_mode_name_direct);
    RUN_TEST(test_get_mode_name_circular);
    RUN_TEST(test_get_mode_name_tree);
    RUN_TEST(test_get_mode_name_unknown);
    
    /* Complex navigation scenarios */
    RUN_TEST(test_circular_navigation_cycle);
    RUN_TEST(test_direct_navigation_roundtrip);
    RUN_TEST(test_tree_navigation_stays_at_bounds);
    
    return UNITY_END();
}
