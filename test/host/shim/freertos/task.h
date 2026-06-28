// SPDX-License-Identifier: MIT
// Host shim: task creation is a no-op that reports success WITHOUT running the
// task body. Tests drive the drain loop synchronously via drain_one(), so we
// never need a real scheduler.
#ifndef SHIM_FREERTOS_TASK_H
#define SHIM_FREERTOS_TASK_H

#include "freertos/FreeRTOS.h"

typedef void* TaskHandle_t;
typedef void (*TaskFunction_t)(void*);

static inline BaseType_t xTaskCreate(TaskFunction_t fn, const char* name,
                                     uint32_t stack, void* arg,
                                     UBaseType_t prio, TaskHandle_t* out) {
    (void)fn;
    (void)name;
    (void)stack;
    (void)arg;
    (void)prio;
    if (out != NULL) {
        *out = (TaskHandle_t)1;  // non-null "handle" so start() looks running
    }
    return pdPASS;
}

static inline void vTaskDelete(TaskHandle_t h) { (void)h; }
static inline void vTaskDelay(TickType_t ticks) { (void)ticks; }

#endif  // SHIM_FREERTOS_TASK_H
