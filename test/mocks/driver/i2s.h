/**
 * @file i2s.h
 * Mock ESP-IDF I2S driver types for native unit testing.
 */

#ifndef I2S_H_MOCK
#define I2S_H_MOCK

#include <stdint.h>
#include "esp_err.h"

typedef enum {
    I2S_NUM_0 = 0,
    I2S_NUM_1 = 1,
    I2S_NUM_MAX
} i2s_port_t;

typedef enum {
    I2S_MODE_MASTER = 1,
    I2S_MODE_SLAVE = 2,
    I2S_MODE_TX = 4,
    I2S_MODE_RX = 8,
    I2S_MODE_DAC_BUILT_IN = 16,
    I2S_MODE_ADC_BUILT_IN = 32,
    I2S_MODE_PDM = 64,
} i2s_mode_t;

typedef enum {
    I2S_BITS_PER_SAMPLE_8BIT = 8,
    I2S_BITS_PER_SAMPLE_16BIT = 16,
    I2S_BITS_PER_SAMPLE_24BIT = 24,
    I2S_BITS_PER_SAMPLE_32BIT = 32,
} i2s_bits_per_sample_t;

typedef enum {
    I2S_CHANNEL_FMT_RIGHT_LEFT = 0,
    I2S_CHANNEL_FMT_ALL_RIGHT = 1,
    I2S_CHANNEL_FMT_ALL_LEFT = 2,
    I2S_CHANNEL_FMT_ONLY_RIGHT = 3,
    I2S_CHANNEL_FMT_ONLY_LEFT = 4,
} i2s_channel_fmt_t;

typedef enum {
    I2S_COMM_FORMAT_STAND_I2S = 0x01,
    I2S_COMM_FORMAT_STAND_MSB = 0x02,
    I2S_COMM_FORMAT_STAND_PCM_SHORT = 0x04,
    I2S_COMM_FORMAT_STAND_PCM_LONG = 0x0C,
} i2s_comm_format_t;

#endif /* I2S_H_MOCK */
