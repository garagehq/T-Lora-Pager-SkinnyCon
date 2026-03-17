/**
 * @file gps_parser_test_impl.h
 * @brief GPS parser implementation for testing (native build compatible)
 * @details This provides the GPSParser.c functions without Arduino dependencies
 */

#ifndef GPS_PARSER_TEST_IMPL_H
#define GPS_PARSER_TEST_IMPL_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief GPS coordinate with validity flag
 */
typedef struct {
    double latitude;   /* degrees, N positive, S negative */
    double longitude;  /* degrees, E positive, W negative */
    int valid;         /* 1 = valid fix, 0 = invalid */
} GPSCoord_t;

/**
 * @brief Parse NMEA latitude in DDMM.MMMM format to decimal degrees
 */
static double gps_parse_lat(double raw_lat, char direction)
{
    if (raw_lat < 0.0) return 0.0;

    int lat_deg = (int)(raw_lat / 100.0);
    double lat_min = raw_lat - (lat_deg * 100.0);
    double decimal = lat_deg + lat_min / 60.0;

    if (direction == 'S' || direction == 's') {
        decimal = -decimal;
    }

    return decimal;
}

/**
 * @brief Parse NMEA longitude in DDMM.MMMM format to decimal degrees
 */
static double gps_parse_lon(double raw_lon, char direction)
{
    if (raw_lon < 0.0) return 0.0;

    int lon_deg = (int)(raw_lon / 100.0);
    double lon_min = raw_lon - (lon_deg * 100.0);
    double decimal = lon_deg + lon_min / 60.0;

    if (direction == 'W' || direction == 'w') {
        decimal = -decimal;
    }

    return decimal;
}

/**
 * @brief Parse latitude/longitude strings and direction
 */
static int gps_parse_coordinates(const char *lat_str, char lat_dir,
                                  const char *lon_str, char lon_dir,
                                  GPSCoord_t *coord)
{
    if (!coord) return 0;
    if (!lat_str || !lon_str) {
        coord->valid = 0;
        return 0;
    }

    /* Validate direction characters */
    if (lat_dir != 'N' && lat_dir != 'S' && lat_dir != 'n' && lat_dir != 's') {
        coord->valid = 0;
        return 0;
    }
    if (lon_dir != 'E' && lon_dir != 'W' && lon_dir != 'e' && lon_dir != 'w') {
        coord->valid = 0;
        return 0;
    }

    double raw_lat = atof(lat_str);
    double raw_lon = atof(lon_str);

    coord->latitude = gps_parse_lat(raw_lat, lat_dir);
    coord->longitude = gps_parse_lon(raw_lon, lon_dir);
    coord->valid = 1;

    return 1;
}

/**
 * @brief Parse GPS RMC sentence and extract coordinates
 */
static int gps_parse_rmc_sentence(const char *rmc_sentence, GPSCoord_t *coord)
{
    if (!rmc_sentence || !coord) return 0;

    /* Check for valid RMC sentence start */
    if (rmc_sentence[0] != '$') return 0;
    if (rmc_sentence[1] != 'G' || rmc_sentence[2] != 'P' || rmc_sentence[3] != 'R' ||
        rmc_sentence[4] != 'M' || rmc_sentence[5] != 'C') {
        return 0;
    }

    /* Parse fields by comma separation
     * RMC format: $GPRMC,<time>,<status>,<lat>,<N/S>,<lon>,<E/W>,<speed>,<date>,<magvar>
     */
    const char *lat = NULL;
    const char *lat_dir = NULL;
    const char *lon = NULL;
    const char *lon_dir = NULL;
    const char *status = NULL;

    const char *ptr = rmc_sentence + 7; /* Skip "$GPRMC," */
    char field[64];
    int field_num = 0;

    while (*ptr && field_num < 10) {
        int i = 0;
        while (*ptr && *ptr != ',' && i < 63) {
            field[i++] = *ptr++;
        }
        field[i] = '\0';

        if (*ptr == ',') ptr++;

        switch (field_num) {
            case 1: status = field; break;
            case 2: lat = field; break;
            case 3: lat_dir = field; break;
            case 4: lon = field; break;
            case 5: lon_dir = field; break;
        }
        field_num++;
    }

    if (!status || !lat || !lat_dir || !lon || !lon_dir) {
        coord->valid = 0;
        return 0;
    }

    if (status[0] != 'A') {
        coord->valid = 0;
        return 0;
    }

    return gps_parse_coordinates(lat, lat_dir[0], lon, lon_dir[0], coord);
}

/**
 * @brief Get GPS location as formatted string
 */
static void gps_format_coordinates(const GPSCoord_t *coord, char *buffer, size_t buf_size)
{
    if (!coord || !buffer || buf_size < 10) {
        if (buffer && buf_size > 0) buffer[0] = '\0';
        return;
    }

    if (!coord->valid) {
        const char *no_fix = "No GPS fix";
        size_t len = strlen(no_fix);
        if (buf_size > len) {
            memcpy(buffer, no_fix, len + 1);
        }
        return;
    }

    const char *valid = "Valid";
    size_t len = strlen(valid);
    if (buf_size > len) {
        memcpy(buffer, valid, len + 1);
    }
}

#ifdef __cplusplus
}
#endif

#endif /* GPS_PARSER_TEST_IMPL_H */
