#include <unity.h>
#include <string.h>
#include <stdlib.h>

// GPS parsing structures and functions
#define MAX_LATLON_LEN 32
#define MAX_FIX_DATE_LEN 64

typedef struct {
    double latitude;
    double longitude;
    int fix_quality;
    int satellites;
    double hdop;
    double altitude;
    char fix_date[MAX_FIX_DATE_LEN];
} GPS_Data_t;

typedef struct {
    double latitude;
    double longitude;
    int fix_quality;
    int satellites;
    double hdop;
    double altitude;
    char fix_date[MAX_FIX_DATE_LEN];
    int valid;
} GPS_State_t;

static void parse_latitude(const char *lat_str, char *hemisphere, double *lat) {
    if (!lat_str || !*lat_str) {
        *lat = 0.0;
        return;
    }
    
    // Format: DDMM.MMMM
    int deg = atoi(lat_str);
    char *dot = strchr(lat_str, '.');
    if (!dot) {
        *lat = (double)deg;
        return;
    }
    
    // Extract minutes from after decimal point
    char min_str[16];
    strncpy(min_str, dot + 1, sizeof(min_str) - 1);
    min_str[sizeof(min_str) - 1] = '\0';
    double min = atof(min_str) / 100.0;
    
    *lat = (double)deg + min;
    
    if (strcmp(hemisphere, "S") == 0) {
        *lat = -*lat;
    }
}

static void parse_longitude(const char *lon_str, char *hemisphere, double *lon) {
    if (!lon_str || !*lon_str) {
        *lon = 0.0;
        return;
    }
    
    // Format: DDDMM.MMMM
    int deg = atoi(lon_str);
    char *dot = strchr(lon_str, '.');
    if (!dot) {
        *lon = (double)deg;
        return;
    }
    
    // Extract minutes from after decimal point
    char min_str[16];
    strncpy(min_str, dot + 1, sizeof(min_str) - 1);
    min_str[sizeof(min_str) - 1] = '\0';
    double min = atof(min_str) / 100.0;
    
    *lon = (double)deg + min;
    
    if (strcmp(hemisphere, "S") == 0 || strcmp(hemisphere, "W") == 0) {
        *lon = -*lon;
    }
}

static void parse_nmea_gga(const char *nmea_sentence, GPS_Data_t *data) {
    if (!nmea_sentence || !data) {
        return;
    }
    
    memset(data, 0, sizeof(GPS_Data_t));
    
    // GPGGA format: $GPGGA,hhmmss.ss,DDMM.MM,A,DDDMM.MM,A,x,xx,x.x,M,M,M,A*hh
    char *ptr = (char *)nmea_sentence;
    
    // Skip to time field
    ptr = strchr(ptr, ',');
    if (!ptr) return;
    ptr++;
    
    // Skip to latitude
    ptr = strchr(ptr, ',');
    if (!ptr) return;
    ptr++;
    
    char lat_str[MAX_LATLON_LEN];
    char lat_hem[4];
    int i = 0;
    while (*ptr && *ptr != ',' && i < MAX_LATLON_LEN - 1) {
        lat_str[i++] = *ptr++;
    }
    lat_str[i] = '\0';
    
    // Latitude hemisphere
    i = 0;
    while (*ptr && *ptr != ',' && i < 3) {
        lat_hem[i++] = *ptr++;
    }
    lat_hem[i] = '\0';
    
    // Skip to longitude
    ptr = strchr(ptr, ',');
    if (!ptr) return;
    ptr++;
    
    char lon_str[MAX_LATLON_LEN];
    char lon_hem[4];
    i = 0;
    while (*ptr && *ptr != ',' && i < MAX_LATLON_LEN - 1) {
        lon_str[i++] = *ptr++;
    }
    lon_str[i] = '\0';
    
    // Longitude hemisphere
    i = 0;
    while (*ptr && *ptr != ',' && i < 3) {
        lon_hem[i++] = *ptr++;
    }
    lon_hem[i] = '\0';
    
    // Parse coordinates
    parse_latitude(lat_str, lat_hem, &data->latitude);
    parse_longitude(lon_str, lon_hem, &data->longitude);
    
    // Parse fix quality
    ptr = strchr(ptr, ',');
    if (ptr) {
        data->fix_quality = atoi(ptr + 1);
    }
    
    // Parse satellites
    ptr = strchr(ptr, ',');
    if (ptr) {
        data->satellites = atoi(ptr + 1);
    }
    
    // Parse HDOP
    ptr = strchr(ptr, ',');
    if (ptr) {
        data->hdop = atof(ptr + 1);
    }
    
    // Parse altitude
    ptr = strchr(ptr, ',');
    if (ptr) {
        ptr = strchr(ptr + 1, ',');
        if (ptr) {
            data->altitude = atof(ptr + 1);
        }
    }
    
    // Parse fix date (GPRMC format)
    ptr = (char *)nmea_sentence;
    ptr = strchr(ptr, ',');
    if (ptr) {
        ptr++;
        i = 0;
        while (*ptr && *ptr != ',' && i < MAX_FIX_DATE_LEN - 1) {
            data->fix_date[i++] = *ptr++;
        }
        data->fix_date[i] = '\0';
    }
}

static int is_valid_gga(const char *nmea) {
    if (!nmea) return 0;
    return (strstr(nmea, "$GPGGA") != NULL || strstr(nmea, "$GNGGA") != NULL);
}

static int is_valid_rmc(const char *nmea) {
    if (!nmea) return 0;
    return (strstr(nmea, "$GPRMC") != NULL || strstr(nmea, "$GNRMC") != NULL);
}

void test_parse_dallas_gps(void) {
    const char *nmea = "$GPGGA,123519,3723.2475,N,12202.8875,W,1,08,0.9,545.4,M,46.0,M,,*47";
    GPS_Data_t data;
    
    parse_nmea_gga(nmea, &data);
    
    TEST_ASSERT_EQUAL_STRING("3723.2475", "");
    TEST_ASSERT_EQUAL_STRING("12202.8875", "");
    TEST_ASSERT_EQUAL_INT(1, data.fix_quality);
    TEST_ASSERT_EQUAL_INT(8, data.satellites);
    TEST_ASSERT_EQUAL_DOUBLE(0.9, data.hdop);
    TEST_ASSERT_EQUAL_DOUBLE(545.4, data.altitude);
}

void test_parse_sao_paulo_gps(void) {
    const char *nmea = "$GPGGA,144512,2333.4715,S,04637.5432,W,1,06,1.2,732.1,M,-23.5,M,,*6A";
    GPS_Data_t data;
    
    parse_nmea_gga(nmea, &data);
    
    TEST_ASSERT_EQUAL_INT(1, data.fix_quality);
    TEST_ASSERT_EQUAL_INT(6, data.satellites);
    TEST_ASSERT_EQUAL_DOUBLE(1.2, data.hdop);
    TEST_ASSERT_EQUAL_DOUBLE(732.1, data.altitude);
}

void test_parse_equator_gps(void) {
    const char *nmea = "$GPGGA,120000,0000.0000,N,00000.0000,E,1,04,2.0,0.0,M,0.0,M,,*54";
    GPS_Data_t data;
    
    parse_nmea_gga(nmea, &data);
    
    TEST_ASSERT_EQUAL_DOUBLE(0.0, data.latitude);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, data.longitude);
    TEST_ASSERT_EQUAL_INT(1, data.fix_quality);
    TEST_ASSERT_EQUAL_INT(4, data.satellites);
}

void test_parse_huntsville_gps(void) {
    const char *nmea = "$GPGGA,153045,3433.1234,N,08633.4567,W,1,10,0.8,210.5,M,-27.0,M,,*6B";
    GPS_Data_t data;
    
    parse_nmea_gga(nmea, &data);
    
    TEST_ASSERT_EQUAL_INT(1, data.fix_quality);
    TEST_ASSERT_EQUAL_INT(10, data.satellites);
    TEST_ASSERT_EQUAL_DOUBLE(0.8, data.hdop);
    TEST_ASSERT_EQUAL_DOUBLE(210.5, data.altitude);
}

void test_parse_antimeridian_gps(void) {
    const char *nmea = "$GPGGA,100000,5130.1234,N,17959.9999,E,1,05,1.5,100.0,M,0.0,M,,*5C";
    GPS_Data_t data;
    
    parse_nmea_gga(nmea, &data);
    
    TEST_ASSERT_EQUAL_INT(1, data.fix_quality);
    TEST_ASSERT_EQUAL_INT(5, data.satellites);
    TEST_ASSERT_EQUAL_DOUBLE(1.5, data.hdop);
}

void test_parse_null_input(void) {
    GPS_Data_t data;
    memset(&data, 0, sizeof(data));
    
    parse_nmea_gga(NULL, &data);
    
    TEST_ASSERT_EQUAL_DOUBLE(0.0, data.latitude);
    TEST_ASSERT_EQUAL_DOUBLE(0.0, data.longitude);
    TEST_ASSERT_EQUAL_INT(0, data.fix_quality);
    TEST_ASSERT_EQUAL_INT(0, data.satellites);
}

void test_is_valid_gga(void) {
    TEST_ASSERT_TRUE(is_valid_gga("$GPGGA,123519,3723.2475,N,12202.8875,W,1,08,0.9,545.4,M,46.0,M,,*47"));
    TEST_ASSERT_TRUE(is_valid_gga("$GNGGA,123519,3723.2475,N,12202.8875,W,1,08,0.9,545.4,M,46.0,M,,*48"));
    TEST_ASSERT_FALSE(is_valid_gga(NULL));
    TEST_ASSERT_FALSE(is_valid_gga("$GPRMC,123519,A,3723.2475,N,12202.8875,W,022.4,084.4,230394,003.1,W*6C"));
}

void test_is_valid_rmc(void) {
    TEST_ASSERT_TRUE(is_valid_rmc("$GPRMC,123519,A,3723.2475,N,12202.8875,W,022.4,084.4,230394,003.1,W*6C"));
    TEST_ASSERT_TRUE(is_valid_rmc("$GNRMC,123519,A,3723.2475,N,12202.8875,W,022.4,084.4,230394,003.1,W*6D"));
    TEST_ASSERT_FALSE(is_valid_rmc(NULL));
    TEST_ASSERT_FALSE(is_valid_rmc("$GPGGA,123519,3723.2475,N,12202.8875,W,1,08,0.9,545.4,M,46.0,M,,*47"));
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_parse_dallas_gps);
    RUN_TEST(test_parse_sao_paulo_gps);
    RUN_TEST(test_parse_equator_gps);
    RUN_TEST(test_parse_huntsville_gps);
    RUN_TEST(test_parse_antimeridian_gps);
    RUN_TEST(test_parse_null_input);
    RUN_TEST(test_is_valid_gga);
    RUN_TEST(test_is_valid_rmc);
    
    return UNITY_END();
}
