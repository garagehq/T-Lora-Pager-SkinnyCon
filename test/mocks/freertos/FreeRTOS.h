/**
 * @file FreeRTOS.h
 * Minimal FreeRTOS mock types for native unit testing.
 */

#ifndef FREERTOS_H_MOCK
#define FREERTOS_H_MOCK

#include <stdint.h>
#include <stddef.h>

/* ---- Basic FreeRTOS types ---- */
typedef uint32_t TickType_t;
typedef void * TaskHandle_t;
typedef void * TimerHandle_t;
typedef void * SemaphoreHandle_t;
typedef void * EventGroupHandle_t;
typedef void * xSemaphoreHandle;
typedef void * QueueHandle_t;

typedef int32_t BaseType_t;
typedef uint32_t UBaseType_t;

#define pdTRUE   ((BaseType_t)1)
#define pdFALSE  ((BaseType_t)0)
#define pdPASS   (pdTRUE)
#define pdFAIL   (pdFALSE)

#define portMAX_DELAY  ((TickType_t)0xFFFFFFFF)
#define portTICK_PERIOD_MS  ((TickType_t)1)

/* ---- Tick conversion macros ---- */
#define pdMS_TO_TICKS(xTimeInMs) ((TickType_t)(xTimeInMs))

/* ---- Timer API stubs ---- */
typedef void (*TimerCallbackFunction_t)(TimerHandle_t xTimer);

static inline TimerHandle_t xTimerCreate(const char *name, TickType_t period,
    BaseType_t autoReload, void *pvTimerID, TimerCallbackFunction_t callback) {
    (void)name; (void)period; (void)autoReload; (void)pvTimerID; (void)callback;
    return NULL;
}

static inline BaseType_t xTimerStart(TimerHandle_t xTimer, TickType_t xTicksToWait) {
    (void)xTimer; (void)xTicksToWait;
    return pdPASS;
}

static inline BaseType_t xTimerStop(TimerHandle_t xTimer, TickType_t xTicksToWait) {
    (void)xTimer; (void)xTicksToWait;
    return pdPASS;
}

static inline BaseType_t xTimerDelete(TimerHandle_t xTimer, TickType_t xTicksToWait) {
    (void)xTimer; (void)xTicksToWait;
    return pdPASS;
}

static inline BaseType_t xTimerIsTimerActive(TimerHandle_t xTimer) {
    (void)xTimer;
    return pdFALSE;
}

static inline void *pvTimerGetTimerID(TimerHandle_t xTimer) {
    (void)xTimer;
    return NULL;
}

/* ---- Semaphore stubs ---- */
static inline SemaphoreHandle_t xSemaphoreCreateMutex(void) { return NULL; }
static inline BaseType_t xSemaphoreTake(SemaphoreHandle_t sem, TickType_t wait) {
    (void)sem; (void)wait;
    return pdTRUE;
}
static inline BaseType_t xSemaphoreGive(SemaphoreHandle_t sem) {
    (void)sem;
    return pdTRUE;
}

/* ---- Task stubs ---- */
static inline void vTaskDelay(TickType_t ticks) { (void)ticks; }

/* ---- Event Group stubs ---- */
static inline EventGroupHandle_t xEventGroupCreate(void) { return NULL; }

/* ---- FreeRTOS timers header ---- */
/* Often included separately but we bundle it here */

#endif /* FREERTOS_H_MOCK */
