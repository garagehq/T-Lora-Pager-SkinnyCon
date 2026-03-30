/**
 * @file Arduino.h
 * Mock Arduino header for native unit testing.
 * Provides basic types, macros, and stubs used by LilyGoLib.
 */

#ifndef ARDUINO_H_MOCK
#define ARDUINO_H_MOCK

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Basic Arduino types ---- */
typedef uint8_t byte;
typedef bool boolean;

/* ---- Pin modes ---- */
#define INPUT           0x0
#define OUTPUT          0x1
#define INPUT_PULLUP    0x2
#define INPUT_PULLDOWN  0x3

/* ---- Digital values ---- */
#define HIGH            0x1
#define LOW             0x0

/* ---- Math macros ---- */
#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef abs
#define abs(x) ((x)>0?(x):-(x))
#endif
#ifndef constrain
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#endif
#ifndef map
#define map(x,in_min,in_max,out_min,out_max) \
    ((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)
#endif

/* ---- Bit manipulation ---- */
#define _BV(bit) (1UL << (bit))
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))
#define lowByte(w) ((uint8_t)((w) & 0xff))
#define highByte(w) ((uint8_t)((w) >> 8))

/* ---- Time functions (stubs) ---- */
static inline void delay(uint32_t ms) { (void)ms; }
static inline void delayMicroseconds(uint32_t us) { (void)us; }

static uint32_t _mock_millis = 0;
static inline uint32_t millis(void) { return _mock_millis; }
static inline uint32_t micros(void) { return _mock_millis * 1000; }
static inline void mock_set_millis(uint32_t ms) { _mock_millis = ms; }

/* ---- GPIO stubs ---- */
static inline void pinMode(uint8_t pin, uint8_t mode) { (void)pin; (void)mode; }
static inline void digitalWrite(uint8_t pin, uint8_t val) { (void)pin; (void)val; }
static inline int digitalRead(uint8_t pin) { (void)pin; return LOW; }
static inline int analogRead(uint8_t pin) { (void)pin; return 0; }
static inline void analogWrite(uint8_t pin, int val) { (void)pin; (void)val; }

/* ---- Default pin definitions ---- */
#ifndef SDA
#define SDA 21
#endif
#ifndef SCL
#define SCL 22
#endif
#ifndef SS
#define SS 5
#endif
#ifndef MOSI
#define MOSI 23
#endif
#ifndef MISO
#define MISO 19
#endif
#ifndef SCK
#define SCK 18
#endif

#ifdef __cplusplus
}

/* ---- String class (minimal) ---- */
#include <string>
class String : public std::string {
public:
    String() : std::string() {}
    String(const char *s) : std::string(s ? s : "") {}
    String(const std::string &s) : std::string(s) {}
    String(int val) : std::string(std::to_string(val)) {}
    String(unsigned int val) : std::string(std::to_string(val)) {}
    String(float val, int decimalPlaces = 2) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.*f", decimalPlaces, val);
        assign(buf);
    }
    int toInt() const { return atoi(c_str()); }
    float toFloat() const { return atof(c_str()); }
    int length() const { return (int)size(); }
    bool equals(const String &s) const { return *this == s; }
    bool startsWith(const String &prefix) const {
        return size() >= prefix.size() && compare(0, prefix.size(), prefix) == 0;
    }
    String substring(int from, int to = -1) const {
        if (to < 0) to = (int)size();
        return String(substr(from, to - from));
    }
    int indexOf(char c) const {
        size_t pos = find(c);
        return pos == npos ? -1 : (int)pos;
    }
};

/* ---- Serial class (minimal) ---- */
class HardwareSerial {
public:
    void begin(unsigned long baud) { (void)baud; }
    void end() {}
    int available() { return 0; }
    int read() { return -1; }
    size_t write(uint8_t c) { (void)c; return 1; }
    size_t write(const uint8_t *buf, size_t len) { (void)buf; return len; }
    void print(const char *s) { (void)s; }
    void println(const char *s) { (void)s; }
    void print(int v) { (void)v; }
    void println(int v) { (void)v; }
    void printf(const char *fmt, ...) { (void)fmt; }
    void flush() {}
};

/* ---- Stream class (minimal) ---- */
class Stream {
public:
    virtual int available() { return 0; }
    virtual int read() { return -1; }
    virtual size_t write(uint8_t c) { (void)c; return 1; }
    virtual size_t write(const uint8_t *buf, size_t len) { (void)buf; return len; }
    virtual ~Stream() {}
};

extern HardwareSerial Serial;

#endif /* __cplusplus */

#endif /* ARDUINO_H_MOCK */
