/**
 * @file gps_signal.c
 * GPS signal parsing and quality assessment for T-LoRa-Pager
 * Implements NMEA GGA sentence parsing, signal quality calculation, and location formatting
 */

#include "gps_signal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/**
 * Parse NMEA GGA sentence
 * Format: $GPGGA,time,lat,N/S,lon,E/W,fix,sats,hdop,alt,M,geoid,M,age,DGPS*hh
 */
int gps_parse_nmea_gga(const char *nmea_line, gps_signal_data_t *signal) {
    if (!nmea_line || !signal) {
        return 0;
    }
    
    // Validate checksum first
    if (!gps_validate_checksum(nmea_line)) {
        return 0;
    }
    
    // Find the data part (after $GPGGA,)
    const char *data_start = strstr(nmea_line, "GPGGA,");
    if (!data_start) {
        return 0;
    }
    data_start += 5; // Skip "GPGGA,"
    
    // Parse fields using strtok
    char line_copy[256];
    strncpy(line_copy, data_start, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';
    
    char *token;
    char *saveptr;
    int field_idx = 0;
    
    token = strtok_r(line_copy, ",", &saveptr);
    while (token != NULL && field_idx < 14) {
        switch (field_idx) {
            case 1: // Latitude
                signal->latitude = atof(token);
                break;
            case 2: // N/S
                if (strcmp(token, "S") == 0) {
                    signal->latitude = -signal->latitude;
                }
                break;
            case 3: // Longitude
                signal->longitude = atof(token);
                break;
            case 4: // E/W
                if (strcmp(token, "W") == 0) {
                    signal->longitude = -signal->longitude;
                }
                break;
            case 5: // Fix quality
                signal->fix_quality = (uint8_t)atoi(token);
                break;
            case 6: // Satellites visible
                signal->satellites_visible = (uint8_t)atoi(token);
                break;
            case 7: // HDOP
                signal->hdop = (float)atof(token);
                break;
            case 8: // Altitude
                signal->altitude_m = (float)atof(token);
                break;
            case 10: // Geoid separation
                signal->geoid_separation = (float)atof(token);
                break;
            case 11: // Age of DGPS
                signal->age_corrections = (uint32_t)atoi(token);
                break;
            case 12: // DGPS reference station ID
                signal->ref_station_id = (uint32_t)atoi(token);
                break;
        }
        
        field_idx++;
        token = strtok_r(NULL, ",", &saveptr);
    }
    
    // Calculate satellite_count from satellites_visible (simplified logic)
    // In real GPS, this would come from the NMEA sentence
    signal->satellite_count = signal->satellites_visible;
    
    // Return success if we have a valid fix
    return (signal->fix_quality > 0 && signal->satellites_visible > 0);
}

/**
 * Validate NMEA checksum
 * Checksum is XOR of all characters between $ and *
 */
int gps_validate_checksum(const char *nmea_line) {
    if (!nmea_line) {
        return 0;
    }
    
    // Find the checksum marker
    const char *checksum_marker = strstr(nmea_line, "*");
    if (!checksum_marker) {
        return 0;
    }
    
    // Extract expected checksum
    char checksum_str[3];
    checksum_str[0] = checksum_marker[1];
    checksum_str[1] = checksum_marker[2];
    checksum_str[2] = '\0';
    
    uint8_t expected_checksum = (uint8_t)strtoul(checksum_str, NULL, 16);
    
    // Calculate checksum from data (between $ and *)
    uint8_t calculated_checksum = 0;
    const char *data_start = nmea_line + 1; // Skip $
    const char *data_end = checksum_marker;
    
    while (data_start < data_end) {
        calculated_checksum ^= (uint8_t)*data_start;
        data_start++;
    }
    
    return (calculated_checksum == expected_checksum);
}

/**
 * Calculate GPS signal quality score (0-5)
 * Based on satellite count, HDOP, and fix quality
 */
int gps_signal_quality(const gps_signal_data_t *signal) {
    if (!signal) {
        return 0;
    }
    
    // No fix = quality 0
    if (signal->fix_quality == 0) {
        return 0;
    }
    
    // No satellites = poor quality
    if (signal->satellites_visible == 0) {
        return 1;
    }
    
    // Quality scoring based on satellites and HDOP
    int quality = 0;
    
    // Satellite count contribution
    if (signal->satellites_visible >= 10) {
        quality += 3;
    } else if (signal->satellites_visible >= 6) {
        quality += 2;
    } else if (signal->satellites_visible >= 4) {
        quality += 1;
    }
    
    // HDOP contribution (lower is better)
    if (signal->hdop < 1.0) {
        quality += 2;
    } else if (signal->hdop < 2.0) {
        quality += 1;
    } else if (signal->hdop < 5.0) {
        quality += 0;
    }
    
    // Cap at 5, minimum 1 if we have some fix
    if (quality > 5) quality = 5;
    if (quality < 1 && signal->fix_quality > 0) quality = 1;
    
    return quality;
}

/**
 * Format GPS location as string
 * Returns "latitude,longitude" format
 */
int gps_format_location(double latitude, double longitude, char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 32) {
        return -1;
    }
    
    int written = snprintf(buffer, buffer_size, "%.4f,%.4f", latitude, longitude);
    return (written > 0 && (size_t)written < buffer_size) ? 0 : -1;
}
