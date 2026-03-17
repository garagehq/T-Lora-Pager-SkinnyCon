/**
 * @file      GPSParser.c
 * @brief     GPS NMEA parser implementation
 * @details   Pure C implementation of NMEA RMC sentence parsing.
 *            Testable logic that works on both ESP32 and native builds.
 */

#include "GPSParser.h"
#include <string.h>
#include <stdlib.h>

/* ================================================================
 *  LATITUDE/LONGITUDE PARSE FUNCTIONS
 * ================================================================ */

double gps_parse_lat(double raw_lat, char direction)
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

double gps_parse_lon(double raw_lon, char direction)
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

int gps_parse_coordinates(const char *lat_str, char lat_dir,
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

/* ================================================================
 *  RMC SENTENCE PARSER
 * ================================================================ */

int gps_parse_rmc_sentence(const char *rmc_sentence, GPSCoord_t *coord)
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
     * Field indices (0-based from time): time=0, status=1, lat=2, lat_dir=3, lon=4, lon_dir=5
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
            case 1: /* Status: A=active, V=void */
                status = field;
                break;
            case 2: /* Latitude */
                lat = field;
                break;
            case 3: /* Latitude direction */
                lat_dir = field;
                break;
            case 4: /* Longitude */
                lon = field;
                break;
            case 5: /* Longitude direction */
                lon_dir = field;
                break;
        }
        field_num++;
    }

    /* Validate required fields */
    if (!status || !lat || !lat_dir || !lon || !lon_dir) {
        coord->valid = 0;
        return 0;
    }

    /* Check for valid status */
    if (status[0] != 'A') {
        coord->valid = 0;
        return 0;
    }

    /* Parse coordinates */
    return gps_parse_coordinates(lat, lat_dir[0], lon, lon_dir[0], coord);
}

/* ================================================================
 *  FORMATTING FUNCTIONS (simplified)
 * ================================================================ */

void gps_format_coordinates(const GPSCoord_t *coord, char *buffer, size_t buf_size)
{
    if (!coord || !buffer || buf_size < 10) {
        if (buffer && buf_size > 0) buffer[0] = '\0';
        return;
    }

    if (!coord->valid) {
        buffer[0] = 'N';
        buffer[1] = 'o';
        buffer[2] = ' ';
        buffer[3] = 'G';
        buffer[4] = 'P';
        buffer[5] = 'S';
        buffer[6] = ' ';
        buffer[7] = 'f';
        buffer[8] = 'i';
        buffer[9] = 'x';
        buffer[10] = '\0';
        return;
    }

    /* Indicate valid coordinates */
    buffer[0] = 'V';
    buffer[1] = 'a';
    buffer[2] = 'l';
    buffer[3] = 'i';
    buffer[4] = 'd';
    buffer[5] = '\0';
}
