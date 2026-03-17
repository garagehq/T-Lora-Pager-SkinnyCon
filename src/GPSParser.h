/**
 * @file      GPSParser.h
 * @brief     Testable GPS NMEA parser for T-LoRa-Pager
 * @details   Pure C implementation of NMEA RMC sentence parsing.
 *            Parses latitude/longitude from GPS RMC sentences.
 *            Works on both ESP32 (real GPS) and native (simulated GPS).
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

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
 * @brief Parse GPS RMC sentence and extract coordinates
 * 
 * @param rmc_sentence Full RMC sentence (e.g., "$GPRMC,123519,A,3352.2568,N,11820.6389,W,...")
 * @param coord Output structure to receive parsed coordinates
 * @return 1 on success, 0 on parse error
 */
int gps_parse_rmc_sentence(const char *rmc_sentence, GPSCoord_t *coord);

/**
 * @brief Parse latitude/longitude strings and direction
 * 
 * @param lat_str Latitude string (e.g., "3352.2568")
 * @param lat_dir Latitude direction ('N' or 'S')
 * @param lon_str Longitude string (e.g., "11820.6389")
 * @param lon_dir Longitude direction ('E' or 'W')
 * @param coord Output structure to receive parsed coordinates
 * @return 1 on success, 0 on parse error
 */
int gps_parse_coordinates(const char *lat_str, char lat_dir,
                          const char *lon_str, char lon_dir,
                          GPSCoord_t *coord);

/**
 * @brief Parse NMEA latitude in DDMM.MMMM format to decimal degrees
 * 
 * @param raw_lat Raw NMEA latitude (e.g., 3352.2568 for 33°52.2568')
 * @param direction 'N' or 'S'
 * @return Latitude in decimal degrees (negative for South)
 */
double gps_parse_lat(double raw_lat, char direction);

/**
 * @brief Parse NMEA longitude in DDMM.MMMM format to decimal degrees
 * 
 * @param raw_lon Raw NMEA longitude (e.g., 11820.6389 for 118°20.6389')
 * @param direction 'E' or 'W'
 * @return Longitude in decimal degrees (negative for West)
 */
double gps_parse_lon(double raw_lon, char direction);

/**
 * @brief Get GPS location as formatted string
 * 
 * @param coord GPS coordinates
 * @param buffer Output buffer
 * @param buf_size Buffer size
 * @return Formatted string like "33°52.2568' N, 118°20.6389' W"
 */
void gps_format_coordinates(const GPSCoord_t *coord, char *buffer, size_t buf_size);

#ifdef __cplusplus
}
#endif
