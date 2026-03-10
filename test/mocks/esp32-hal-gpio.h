/**
 * @file esp32-hal-gpio.h
 * Mock ESP32 GPIO HAL defines for native unit testing.
 */

#ifndef ESP32_HAL_GPIO_H_MOCK
#define ESP32_HAL_GPIO_H_MOCK

#include <stdint.h>

#define GPIO_NUM_0   0
#define GPIO_NUM_1   1
#define GPIO_NUM_2   2
#define GPIO_NUM_3   3
#define GPIO_NUM_4   4
#define GPIO_NUM_5   5
#define GPIO_NUM_6   6
#define GPIO_NUM_7   7
#define GPIO_NUM_8   8
#define GPIO_NUM_9   9
#define GPIO_NUM_10  10
#define GPIO_NUM_11  11
#define GPIO_NUM_12  12
#define GPIO_NUM_13  13
#define GPIO_NUM_14  14
#define GPIO_NUM_15  15
#define GPIO_NUM_16  16
#define GPIO_NUM_17  17
#define GPIO_NUM_18  18
#define GPIO_NUM_19  19
#define GPIO_NUM_20  20
#define GPIO_NUM_21  21

/* ESP32 GPIO interrupt types */
#define GPIO_INTR_DISABLE    0
#define GPIO_INTR_POSEDGE    1
#define GPIO_INTR_NEGEDGE    2
#define GPIO_INTR_ANYEDGE    3
#define GPIO_INTR_LOW_LEVEL  4
#define GPIO_INTR_HIGH_LEVEL 5

/* ESP32 pull modes */
#define GPIO_PULLUP_ONLY     0
#define GPIO_PULLDOWN_ONLY   1
#define GPIO_PULLUP_PULLDOWN 2
#define GPIO_FLOATING        3

#endif /* ESP32_HAL_GPIO_H_MOCK */
