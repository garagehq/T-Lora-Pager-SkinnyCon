/**
 * @file gps_signal.h
 * GPS signal parsing and quality assessment for T-LoRa-Pager
 * Handles NMEA GGA sentence parsing, signal quality calculation, and location formatting
 */

#ifndef GPS_SIGNAL_H
#define GPS_SIGNAL_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * GPS signal data structure
 */
typedef struct {
    double latitude;      // Degrees, positive = North
    double longitude;     // Degrees, positive = East
    uint8_t satellites_visible;  // Number of satellites in view
    uint8_t satellite_count;     // Number of satellites used in fix
    float hdop;           // Horizontal Dilution of Precision
    float altitude_m;     // Altitude in meters above sea level
    float geoid_separation; // Geoid separation in meters
    uint8_t fix_quality;  // GPS fix quality (0=invalid, 1=GPS, 2=DGPS, etc.)
    uint32_t age_corrections; // Age of differential GPS data in seconds
    uint32_t ref_station_id;  // Reference station ID
} gps_signal_data_t;

/**
 * Parse NMEA GGA sentence
 * @param nmea_line NMEA GGA sentence string
 * @param signal Output structure to store parsed data
 * @return 1 on success, 0 on failure
 */
int gps_parse_nmea_gga(const char *nmea_line, gps_signal_data_t *signal);

/**
 * Validate NMEA checksum
 * @param nmea_line NMEA sentence with checksum
 * @return 1 if checksum is valid, 0 otherwise
 */
int gps_validate_checksum(const char *nmea_line);

/**
 * Calculate GPS signal quality score (0-5)
 * @param signal GPS signal data
 * @return Quality score: 0=no fix, 1=poor, 2=fair, 3=good, 4=very good, 5=excellent
 */
int gps_signal_quality(const gps_signal_data_t *signal);

/**
 * Format GPS location as string
 * @param latitude Latitude in degrees
 * @param longitude Longitude in degrees
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return 0 on success, -1 on error
 */
int gps_format_location(double latitude, double longitude, char *buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* GPS_SIGNAL_H */
