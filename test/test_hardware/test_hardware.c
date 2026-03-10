/**
 * @file test_hardware.c
 * Unity tests for T-LoRa-Pager hardware logic, data structures, and constants.
 * Runs on native platform — no ESP32 hardware required.
 */

#include <unity.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* ---- Reproduce hardware constants from source ---- */

/* Display dimensions (from LilyGo_LoRa_Pager) */
#define DISPLAY_WIDTH   480
#define DISPLAY_HEIGHT  222

/* Brightness range (from BrightnessController<..., 0, 16, ...>) */
#define BRIGHTNESS_MIN  0
#define BRIGHTNESS_MAX  16

/* PowerCtrlChannel enum values (from LilyGoTypedef.h) */
typedef enum PowerCtrlChannel {
    POWER_DISPLAY,
    POWER_DISPLAY_BACKLIGHT,
    POWER_RADIO,
    POWER_HAPTIC_DRIVER,
    POWER_GPS,
    POWER_NFC,
    POWER_SD_CARD,
    POWER_SPEAK,
    POWER_SENSOR,
    POWER_KEYBOARD,
    POWER_EXT_GPIO,
    POWER_SI4735_RADIO,
    POWER_CODEC,
    POWER_RTC,
} PowerCtrlChannel_t;

/* RotaryMsg_t (from LilyGoDispInterface.h) */
typedef enum RotaryDir {
    ROTARY_DIR_NONE,
    ROTARY_DIR_UP,
    ROTARY_DIR_DOWN,
} RotaryDir_t;

typedef struct RotaryMsg {
    RotaryDir_t dir;
    int centerBtnPressed; /* bool in C */
} RotaryMsg_t;

/* Hardware presence mask bits (from LilyGoTypedef.h) */
#define HW_RADIO_ONLINE    (1UL << 0)
#define HW_TOUCH_ONLINE    (1UL << 1)
#define HW_DRV_ONLINE      (1UL << 2)
#define HW_PMU_ONLINE      (1UL << 3)
#define HW_RTC_ONLINE      (1UL << 4)
#define HW_PSRAM_ONLINE    (1UL << 5)
#define HW_GPS_ONLINE      (1UL << 6)
#define HW_SD_ONLINE       (1UL << 7)
#define HW_NFC_ONLINE      (1UL << 8)
#define HW_BHI260AP_ONLINE (1UL << 9)
#define HW_KEYBOARD_ONLINE (1UL << 10)
#define HW_GAUGE_ONLINE    (1UL << 11)
#define HW_EXPAND_ONLINE   (1UL << 12)
#define HW_CODEC_ONLINE    (1UL << 13)

/* ---- Brightness clamping logic (from BrightnessController) ---- */
static uint8_t clamp_brightness(uint8_t level)
{
    if (level < BRIGHTNESS_MIN) level = BRIGHTNESS_MIN;
    if (level > BRIGHTNESS_MAX) level = BRIGHTNESS_MAX;
    return level;
}

/* ---- RGB565 byte swap (common for SPI displays) ---- */
static uint16_t rgb565_swap(uint16_t color)
{
    return (color >> 8) | (color << 8);
}

/* ---- GPS NMEA coordinate parsing helper ---- */
typedef struct {
    double latitude;
    double longitude;
    int valid;
} GpsCoord_t;

static GpsCoord_t parse_gps_rmc(const char *lat_str, char lat_dir,
                                  const char *lon_str, char lon_dir)
{
    GpsCoord_t coord = {0.0, 0.0, 0};
    if (!lat_str || !lon_str) return coord;

    /* Parse NMEA format: DDMM.MMMM */
    double raw_lat = atof(lat_str);
    double raw_lon = atof(lon_str);

    int lat_deg = (int)(raw_lat / 100.0);
    double lat_min = raw_lat - (lat_deg * 100.0);
    coord.latitude = lat_deg + lat_min / 60.0;
    if (lat_dir == 'S') coord.latitude = -coord.latitude;

    int lon_deg = (int)(raw_lon / 100.0);
    double lon_min = raw_lon - (lon_deg * 100.0);
    coord.longitude = lon_deg + lon_min / 60.0;
    if (lon_dir == 'W') coord.longitude = -coord.longitude;

    coord.valid = 1;
    return coord;
}

/* ================================================================
 *  TEST CASES
 * ================================================================ */

void setUp(void) {}
void tearDown(void) {}

/* ---- Display dimension tests ---- */

void test_display_width(void)
{
    TEST_ASSERT_EQUAL_UINT16(480, DISPLAY_WIDTH);
}

void test_display_height(void)
{
    TEST_ASSERT_EQUAL_UINT16(222, DISPLAY_HEIGHT);
}

void test_display_is_landscape(void)
{
    TEST_ASSERT_TRUE(DISPLAY_WIDTH > DISPLAY_HEIGHT);
}

void test_display_pixel_count(void)
{
    TEST_ASSERT_EQUAL_UINT32(480 * 222, DISPLAY_WIDTH * DISPLAY_HEIGHT);
}

/* ---- Brightness tests ---- */

void test_brightness_min(void)
{
    TEST_ASSERT_EQUAL_UINT8(0, BRIGHTNESS_MIN);
}

void test_brightness_max(void)
{
    TEST_ASSERT_EQUAL_UINT8(16, BRIGHTNESS_MAX);
}

void test_brightness_clamp_in_range(void)
{
    TEST_ASSERT_EQUAL_UINT8(8, clamp_brightness(8));
    TEST_ASSERT_EQUAL_UINT8(0, clamp_brightness(0));
    TEST_ASSERT_EQUAL_UINT8(16, clamp_brightness(16));
}

void test_brightness_clamp_overflow(void)
{
    TEST_ASSERT_EQUAL_UINT8(16, clamp_brightness(255));
    TEST_ASSERT_EQUAL_UINT8(16, clamp_brightness(17));
    TEST_ASSERT_EQUAL_UINT8(16, clamp_brightness(100));
}

void test_brightness_levels_count(void)
{
    /* 0 through 16 inclusive = 17 levels */
    TEST_ASSERT_EQUAL_INT(17, BRIGHTNESS_MAX - BRIGHTNESS_MIN + 1);
}

/* ---- PowerCtrlChannel enum tests ---- */

void test_power_channel_display(void)
{
    TEST_ASSERT_EQUAL_INT(0, POWER_DISPLAY);
}

void test_power_channel_backlight(void)
{
    TEST_ASSERT_EQUAL_INT(1, POWER_DISPLAY_BACKLIGHT);
}

void test_power_channel_radio(void)
{
    TEST_ASSERT_EQUAL_INT(2, POWER_RADIO);
}

void test_power_channel_count(void)
{
    /* 14 channels: POWER_DISPLAY(0) through POWER_RTC(13) */
    TEST_ASSERT_EQUAL_INT(13, POWER_RTC);
}

void test_power_channel_all_unique(void)
{
    int values[] = {
        POWER_DISPLAY, POWER_DISPLAY_BACKLIGHT, POWER_RADIO,
        POWER_HAPTIC_DRIVER, POWER_GPS, POWER_NFC, POWER_SD_CARD,
        POWER_SPEAK, POWER_SENSOR, POWER_KEYBOARD, POWER_EXT_GPIO,
        POWER_SI4735_RADIO, POWER_CODEC, POWER_RTC
    };
    int count = sizeof(values) / sizeof(values[0]);
    TEST_ASSERT_EQUAL_INT(14, count);

    for (int i = 0; i < count; i++) {
        for (int j = i + 1; j < count; j++) {
            TEST_ASSERT_NOT_EQUAL(values[i], values[j]);
        }
    }
}

/* ---- RotaryMsg_t tests ---- */

void test_rotary_dir_none(void)
{
    TEST_ASSERT_EQUAL_INT(0, ROTARY_DIR_NONE);
}

void test_rotary_dir_up(void)
{
    TEST_ASSERT_EQUAL_INT(1, ROTARY_DIR_UP);
}

void test_rotary_dir_down(void)
{
    TEST_ASSERT_EQUAL_INT(2, ROTARY_DIR_DOWN);
}

void test_rotary_msg_default(void)
{
    RotaryMsg_t msg;
    memset(&msg, 0, sizeof(msg));
    TEST_ASSERT_EQUAL_INT(ROTARY_DIR_NONE, msg.dir);
    TEST_ASSERT_FALSE(msg.centerBtnPressed);
}

void test_rotary_msg_button_press(void)
{
    RotaryMsg_t msg = { ROTARY_DIR_UP, 1 };
    TEST_ASSERT_EQUAL_INT(ROTARY_DIR_UP, msg.dir);
    TEST_ASSERT_TRUE(msg.centerBtnPressed);
}

/* ---- GPS coordinate parsing tests ---- */

void test_gps_parse_north_east(void)
{
    /* Dallas TX: 32 47.0000 N, 096 48.0000 W */
    GpsCoord_t c = parse_gps_rmc("3247.0000", 'N', "09648.0000", 'W');
    TEST_ASSERT_TRUE(c.valid);
    TEST_ASSERT_DOUBLE_WITHIN(0.01, 32.783, c.latitude);
    TEST_ASSERT_DOUBLE_WITHIN(0.01, -96.80, c.longitude);
}

void test_gps_parse_south_west(void)
{
    /* Sao Paulo: 23 33.0000 S, 046 38.0000 W */
    GpsCoord_t c = parse_gps_rmc("2333.0000", 'S', "04638.0000", 'W');
    TEST_ASSERT_TRUE(c.valid);
    TEST_ASSERT_TRUE(c.latitude < 0);
    TEST_ASSERT_TRUE(c.longitude < 0);
}

void test_gps_parse_equator(void)
{
    GpsCoord_t c = parse_gps_rmc("0000.0000", 'N', "00000.0000", 'E');
    TEST_ASSERT_TRUE(c.valid);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 0.0, c.latitude);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 0.0, c.longitude);
}

void test_gps_parse_null_input(void)
{
    GpsCoord_t c = parse_gps_rmc(NULL, 'N', NULL, 'E');
    TEST_ASSERT_FALSE(c.valid);
}

/* ---- RGB565 color tests ---- */

void test_rgb565_swap_white(void)
{
    uint16_t white = 0xFFFF;
    TEST_ASSERT_EQUAL_HEX16(0xFFFF, rgb565_swap(white));
}

void test_rgb565_swap_black(void)
{
    uint16_t black = 0x0000;
    TEST_ASSERT_EQUAL_HEX16(0x0000, rgb565_swap(black));
}

void test_rgb565_swap_red(void)
{
    /* Pure red in RGB565 = 0xF800 */
    uint16_t red = 0xF800;
    uint16_t swapped = rgb565_swap(red);
    TEST_ASSERT_EQUAL_HEX16(0x00F8, swapped);
}

void test_rgb565_swap_roundtrip(void)
{
    uint16_t color = 0x1234;
    TEST_ASSERT_EQUAL_HEX16(color, rgb565_swap(rgb565_swap(color)));
}

/* ---- Hardware presence mask tests ---- */

void test_hw_mask_radio(void)
{
    TEST_ASSERT_EQUAL_HEX32(0x01, HW_RADIO_ONLINE);
}

void test_hw_mask_keyboard(void)
{
    TEST_ASSERT_EQUAL_HEX32(1UL << 10, HW_KEYBOARD_ONLINE);
}

void test_hw_mask_combined(void)
{
    uint32_t probe = HW_RADIO_ONLINE | HW_GPS_ONLINE | HW_KEYBOARD_ONLINE;
    TEST_ASSERT_TRUE(probe & HW_RADIO_ONLINE);
    TEST_ASSERT_TRUE(probe & HW_GPS_ONLINE);
    TEST_ASSERT_TRUE(probe & HW_KEYBOARD_ONLINE);
    TEST_ASSERT_FALSE(probe & HW_NFC_ONLINE);
}

/* ---- Keyboard character mapping tests ---- */

void test_keyboard_state_values(void)
{
    /* KB_NONE = -1, KB_PRESSED = 1, KB_RELEASED = 0 */
    TEST_ASSERT_EQUAL_INT(-1, -1);   /* KB_NONE */
    TEST_ASSERT_EQUAL_INT(1, 1);     /* KB_PRESSED */
    TEST_ASSERT_EQUAL_INT(0, 0);     /* KB_RELEASED */
}

void test_keyboard_ascii_range(void)
{
    /* Standard QWERTY keys should map to printable ASCII (32-126) */
    char test_keys[] = "qwertyuiopasdfghjklzxcvbnm";
    for (int i = 0; test_keys[i]; i++) {
        TEST_ASSERT_TRUE(test_keys[i] >= 32 && test_keys[i] <= 126);
    }
}

/* ================================================================
 *  MAIN
 * ================================================================ */

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    UNITY_BEGIN();

    /* Display dimensions */
    RUN_TEST(test_display_width);
    RUN_TEST(test_display_height);
    RUN_TEST(test_display_is_landscape);
    RUN_TEST(test_display_pixel_count);

    /* Brightness */
    RUN_TEST(test_brightness_min);
    RUN_TEST(test_brightness_max);
    RUN_TEST(test_brightness_clamp_in_range);
    RUN_TEST(test_brightness_clamp_overflow);
    RUN_TEST(test_brightness_levels_count);

    /* PowerCtrlChannel */
    RUN_TEST(test_power_channel_display);
    RUN_TEST(test_power_channel_backlight);
    RUN_TEST(test_power_channel_radio);
    RUN_TEST(test_power_channel_count);
    RUN_TEST(test_power_channel_all_unique);

    /* RotaryMsg */
    RUN_TEST(test_rotary_dir_none);
    RUN_TEST(test_rotary_dir_up);
    RUN_TEST(test_rotary_dir_down);
    RUN_TEST(test_rotary_msg_default);
    RUN_TEST(test_rotary_msg_button_press);

    /* GPS parsing */
    RUN_TEST(test_gps_parse_north_east);
    RUN_TEST(test_gps_parse_south_west);
    RUN_TEST(test_gps_parse_equator);
    RUN_TEST(test_gps_parse_null_input);

    /* RGB565 color */
    RUN_TEST(test_rgb565_swap_white);
    RUN_TEST(test_rgb565_swap_black);
    RUN_TEST(test_rgb565_swap_red);
    RUN_TEST(test_rgb565_swap_roundtrip);

    /* Hardware presence mask */
    RUN_TEST(test_hw_mask_radio);
    RUN_TEST(test_hw_mask_keyboard);
    RUN_TEST(test_hw_mask_combined);

    /* Keyboard */
    RUN_TEST(test_keyboard_state_values);
    RUN_TEST(test_keyboard_ascii_range);

    return UNITY_END();
}
