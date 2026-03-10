/**
 * @file spi_master.h
 * Mock ESP-IDF SPI master driver for native unit testing.
 */

#ifndef SPI_MASTER_H_MOCK
#define SPI_MASTER_H_MOCK

#include <stdint.h>
#include "esp_err.h"

typedef void *spi_device_handle_t;
typedef int spi_host_device_t;

#define SPI2_HOST 1
#define SPI3_HOST 2

#endif /* SPI_MASTER_H_MOCK */
