/**
 * @file SPI.h
 * Mock SPI header for native unit testing.
 */

#ifndef SPI_H_MOCK
#define SPI_H_MOCK

#include <stdint.h>

#ifdef __cplusplus

#define SPI_MODE0 0
#define SPI_MODE1 1
#define SPI_MODE2 2
#define SPI_MODE3 3

#define MSBFIRST 1
#define LSBFIRST 0

class SPISettings {
public:
    SPISettings() : _clock(1000000), _bitOrder(MSBFIRST), _dataMode(SPI_MODE0) {}
    SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode)
        : _clock(clock), _bitOrder(bitOrder), _dataMode(dataMode) {}
private:
    uint32_t _clock;
    uint8_t _bitOrder;
    uint8_t _dataMode;
};

class SPIClass {
public:
    SPIClass() {}
    SPIClass(uint8_t bus) { (void)bus; }

    void begin(int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1, int8_t ss = -1) {
        (void)sck; (void)miso; (void)mosi; (void)ss;
    }
    void end() {}

    void beginTransaction(SPISettings settings) { (void)settings; }
    void endTransaction() {}

    uint8_t transfer(uint8_t data) { (void)data; return 0; }
    uint16_t transfer16(uint16_t data) { (void)data; return 0; }
    void transfer(void *buf, size_t count) { (void)buf; (void)count; }

    void setBitOrder(uint8_t bitOrder) { (void)bitOrder; }
    void setDataMode(uint8_t dataMode) { (void)dataMode; }
    void setFrequency(uint32_t freq) { (void)freq; }
};

extern SPIClass SPI;

#endif /* __cplusplus */
#endif /* SPI_H_MOCK */
