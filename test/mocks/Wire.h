/**
 * @file Wire.h
 * Mock Wire/I2C header for native unit testing.
 */

#ifndef WIRE_H_MOCK
#define WIRE_H_MOCK

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus

class TwoWire {
public:
    TwoWire() {}
    TwoWire(uint8_t bus) { (void)bus; }

    bool begin(int sda = -1, int scl = -1, uint32_t frequency = 100000) {
        (void)sda; (void)scl; (void)frequency;
        return true;
    }
    void end() {}

    void setClock(uint32_t freq) { (void)freq; }

    void beginTransmission(uint8_t addr) { (void)addr; }
    uint8_t endTransmission(bool sendStop = true) { (void)sendStop; return 0; }

    size_t write(uint8_t data) { (void)data; return 1; }
    size_t write(const uint8_t *data, size_t len) { (void)data; return len; }

    uint8_t requestFrom(uint8_t addr, uint8_t quantity, bool sendStop = true) {
        (void)addr; (void)quantity; (void)sendStop;
        return 0;
    }

    int available() { return 0; }
    int read() { return -1; }
    int peek() { return -1; }
    void flush() {}
};

extern TwoWire Wire;
extern TwoWire Wire1;

#endif /* __cplusplus */
#endif /* WIRE_H_MOCK */
