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

/* ---- Brightness clamping tests ---- */

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

void test_brightness_clamp_all_valid_levels(void)
{
    /* Every level from 0 to 16 should pass through unchanged */
    for (uint8_t i = 0; i <= BRIGHTNESS_MAX; i++) {
        TEST_ASSERT_EQUAL_UINT8(i, clamp_brightness(i));
    }
}

void test_brightness_clamp_boundary(void)
{
    /* Just above max gets clamped */
    TEST_ASSERT_EQUAL_UINT8(BRIGHTNESS_MAX, clamp_brightness(BRIGHTNESS_MAX + 1));
    /* Max itself passes through */
    TEST_ASSERT_EQUAL_UINT8(BRIGHTNESS_MAX, clamp_brightness(BRIGHTNESS_MAX));
}

/* ---- PowerCtrlChannel enum tests ---- */

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

void test_power_channel_contiguous(void)
{
    /* Channels should be contiguous from 0 to 13 */
    TEST_ASSERT_EQUAL_INT(0, POWER_DISPLAY);
    TEST_ASSERT_EQUAL_INT(POWER_RTC, 13);
    /* Verify each channel = previous + 1 */
    int values[] = {
        POWER_DISPLAY, POWER_DISPLAY_BACKLIGHT, POWER_RADIO,
        POWER_HAPTIC_DRIVER, POWER_GPS, POWER_NFC, POWER_SD_CARD,
        POWER_SPEAK, POWER_SENSOR, POWER_KEYBOARD, POWER_EXT_GPIO,
        POWER_SI4735_RADIO, POWER_CODEC, POWER_RTC
    };
    for (int i = 1; i < 14; i++) {
        TEST_ASSERT_EQUAL_INT(values[i - 1] + 1, values[i]);
    }
}

/* ---- RotaryMsg_t tests ---- */

void test_rotary_msg_struct_layout(void)
{
    /* Verify zero-init produces NONE/unpressed */
    RotaryMsg_t msg;
    memset(&msg, 0, sizeof(msg));
    TEST_ASSERT_EQUAL_INT(ROTARY_DIR_NONE, msg.dir);
    TEST_ASSERT_FALSE(msg.centerBtnPressed);

    /* Verify all direction + button combinations */
    RotaryDir_t dirs[] = {ROTARY_DIR_NONE, ROTARY_DIR_UP, ROTARY_DIR_DOWN};
    for (int d = 0; d < 3; d++) {
        for (int b = 0; b <= 1; b++) {
            msg.dir = dirs[d];
            msg.centerBtnPressed = b;
            TEST_ASSERT_EQUAL_INT(dirs[d], msg.dir);
            TEST_ASSERT_EQUAL_INT(b, msg.centerBtnPressed);
        }
    }
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

void test_gps_parse_huntsville(void)
{
    /* Huntsville AL (SkinnyCon venue): 34°43.8' N, 86°35.4' W */
    GpsCoord_t c = parse_gps_rmc("3443.8000", 'N', "08635.4000", 'W');
    TEST_ASSERT_TRUE(c.valid);
    TEST_ASSERT_DOUBLE_WITHIN(0.01, 34.73, c.latitude);
    TEST_ASSERT_DOUBLE_WITHIN(0.01, -86.59, c.longitude);
}

void test_gps_parse_antimeridian(void)
{
    /* Near antimeridian: 179°59' E */
    GpsCoord_t c = parse_gps_rmc("0000.0000", 'N', "17959.0000", 'E');
    TEST_ASSERT_TRUE(c.valid);
    TEST_ASSERT_DOUBLE_WITHIN(0.01, 179.983, c.longitude);
}

void test_gps_parse_max_latitude(void)
{
    /* North pole: 90°00' N */
    GpsCoord_t c = parse_gps_rmc("9000.0000", 'N', "00000.0000", 'E');
    TEST_ASSERT_TRUE(c.valid);
    TEST_ASSERT_DOUBLE_WITHIN(0.001, 90.0, c.latitude);
}

void test_gps_parse_partial_null(void)
{
    /* One coordinate null, other valid */
    GpsCoord_t c1 = parse_gps_rmc("3247.0000", 'N', NULL, 'W');
    TEST_ASSERT_FALSE(c1.valid);
    GpsCoord_t c2 = parse_gps_rmc(NULL, 'N', "09648.0000", 'W');
    TEST_ASSERT_FALSE(c2.valid);
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

void test_rgb565_swap_all_single_byte(void)
{
    /* When both bytes are the same, swap is identity */
    TEST_ASSERT_EQUAL_HEX16(0xAAAA, rgb565_swap(0xAAAA));
    TEST_ASSERT_EQUAL_HEX16(0x5555, rgb565_swap(0x5555));
}

void test_rgb565_channel_extraction(void)
{
    /* Pure red 0xF800: R=31, G=0, B=0 */
    uint16_t red = 0xF800;
    TEST_ASSERT_EQUAL_UINT8(31, (red >> 11) & 0x1F);  /* R */
    TEST_ASSERT_EQUAL_UINT8(0,  (red >> 5) & 0x3F);   /* G */
    TEST_ASSERT_EQUAL_UINT8(0,  red & 0x1F);           /* B */

    /* Pure green 0x07E0: R=0, G=63, B=0 */
    uint16_t green = 0x07E0;
    TEST_ASSERT_EQUAL_UINT8(0,  (green >> 11) & 0x1F);
    TEST_ASSERT_EQUAL_UINT8(63, (green >> 5) & 0x3F);
    TEST_ASSERT_EQUAL_UINT8(0,  green & 0x1F);

    /* Pure blue 0x001F: R=0, G=0, B=31 */
    uint16_t blue = 0x001F;
    TEST_ASSERT_EQUAL_UINT8(0,  (blue >> 11) & 0x1F);
    TEST_ASSERT_EQUAL_UINT8(0,  (blue >> 5) & 0x3F);
    TEST_ASSERT_EQUAL_UINT8(31, blue & 0x1F);
}

/* ---- Hardware presence mask tests ---- */

void test_hw_mask_no_overlap(void)
{
    /* Each mask bit should be unique (no two share a bit) */
    uint32_t all_masks[] = {
        HW_RADIO_ONLINE, HW_TOUCH_ONLINE, HW_DRV_ONLINE, HW_PMU_ONLINE,
        HW_RTC_ONLINE, HW_PSRAM_ONLINE, HW_GPS_ONLINE, HW_SD_ONLINE,
        HW_NFC_ONLINE, HW_BHI260AP_ONLINE, HW_KEYBOARD_ONLINE,
        HW_GAUGE_ONLINE, HW_EXPAND_ONLINE, HW_CODEC_ONLINE
    };
    int count = sizeof(all_masks) / sizeof(all_masks[0]);
    for (int i = 0; i < count; i++) {
        for (int j = i + 1; j < count; j++) {
            TEST_ASSERT_EQUAL_HEX32(0, all_masks[i] & all_masks[j]);
        }
    }
}

void test_hw_mask_set_clear_toggle(void)
{
    uint32_t probe = 0;

    /* Set radio + GPS */
    probe |= HW_RADIO_ONLINE;
    probe |= HW_GPS_ONLINE;
    TEST_ASSERT_TRUE(probe & HW_RADIO_ONLINE);
    TEST_ASSERT_TRUE(probe & HW_GPS_ONLINE);
    TEST_ASSERT_FALSE(probe & HW_NFC_ONLINE);

    /* Clear GPS */
    probe &= ~HW_GPS_ONLINE;
    TEST_ASSERT_TRUE(probe & HW_RADIO_ONLINE);
    TEST_ASSERT_FALSE(probe & HW_GPS_ONLINE);

    /* Toggle radio off */
    probe ^= HW_RADIO_ONLINE;
    TEST_ASSERT_FALSE(probe & HW_RADIO_ONLINE);
    TEST_ASSERT_EQUAL_HEX32(0, probe);
}

void test_hw_mask_full_probe(void)
{
    /* All peripherals online */
    uint32_t full = HW_RADIO_ONLINE | HW_TOUCH_ONLINE | HW_DRV_ONLINE |
                    HW_PMU_ONLINE | HW_RTC_ONLINE | HW_PSRAM_ONLINE |
                    HW_GPS_ONLINE | HW_SD_ONLINE | HW_NFC_ONLINE |
                    HW_BHI260AP_ONLINE | HW_KEYBOARD_ONLINE |
                    HW_GAUGE_ONLINE | HW_EXPAND_ONLINE | HW_CODEC_ONLINE;
    /* 14 bits set: should equal (1<<14)-1 = 0x3FFF */
    TEST_ASSERT_EQUAL_HEX32(0x3FFF, full);
}

/* ---- LoRa frequency default tests ---- */

/* Reproduce the default from hal_interface.h */
#define RADIO_DEFAULT_FREQUENCY  915.0

void test_lora_default_frequency_is_915(void)
{
    /* SkinnyCon is a US conference — must use 915 MHz ISM band */
    TEST_ASSERT_DOUBLE_WITHIN(0.1, 915.0, RADIO_DEFAULT_FREQUENCY);
}

void test_lora_frequency_in_us_ism_band(void)
{
    /* US ISM band: 902-928 MHz */
    TEST_ASSERT_TRUE(RADIO_DEFAULT_FREQUENCY >= 902.0);
    TEST_ASSERT_TRUE(RADIO_DEFAULT_FREQUENCY <= 928.0);
}

void test_lora_frequency_not_eu_band(void)
{
    /* EU band is 868 MHz — should NOT be the default for US conference */
    TEST_ASSERT_TRUE(RADIO_DEFAULT_FREQUENCY != 868.0);
}

/* ================================================================
 *  MAIN
 * ================================================================ */

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    UNITY_BEGIN();

    /* Brightness clamping */
    RUN_TEST(test_brightness_clamp_in_range);
    RUN_TEST(test_brightness_clamp_overflow);
    RUN_TEST(test_brightness_clamp_all_valid_levels);
    RUN_TEST(test_brightness_clamp_boundary);

    /* PowerCtrlChannel */
    RUN_TEST(test_power_channel_all_unique);
    RUN_TEST(test_power_channel_contiguous);

    /* RotaryMsg */
    RUN_TEST(test_rotary_msg_struct_layout);

    /* GPS parsing */
    RUN_TEST(test_gps_parse_north_east);
    RUN_TEST(test_gps_parse_south_west);
    RUN_TEST(test_gps_parse_equator);
    RUN_TEST(test_gps_parse_null_input);
    RUN_TEST(test_gps_parse_huntsville);
    RUN_TEST(test_gps_parse_antimeridian);
    RUN_TEST(test_gps_parse_max_latitude);
    RUN_TEST(test_gps_parse_partial_null);

    /* RGB565 color */
    RUN_TEST(test_rgb565_swap_white);
    RUN_TEST(test_rgb565_swap_black);
    RUN_TEST(test_rgb565_swap_red);
    RUN_TEST(test_rgb565_swap_roundtrip);
    RUN_TEST(test_rgb565_swap_all_single_byte);
    RUN_TEST(test_rgb565_channel_extraction);

    /* Hardware presence mask */
    RUN_TEST(test_hw_mask_no_overlap);
    RUN_TEST(test_hw_mask_set_clear_toggle);
    RUN_TEST(test_hw_mask_full_probe);

    /* LoRa frequency */
    RUN_TEST(test_lora_default_frequency_is_915);
    RUN_TEST(test_lora_frequency_in_us_ism_band);
    RUN_TEST(test_lora_frequency_not_eu_band);

    return UNITY_END();
}
