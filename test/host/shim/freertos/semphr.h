// SPDX-License-Identifier: MIT
// Host shim: a binary mutex as a single flag. Single-threaded tests never
// truly contend, but take/give still toggle the flag so LockGuard's
// acquire/release and "fails when already held with timeout 0" are observable.
#ifndef SHIM_FREERTOS_SEMPHR_H
#define SHIM_FREERTOS_SEMPHR_H

#include <stdlib.h>

#include "freertos/FreeRTOS.h"

typedef struct {
    int taken;
} StaticSemaphore_t;

typedef StaticSemaphore_t* SemaphoreHandle_t;

static inline SemaphoreHandle_t xSemaphoreCreateMutex(void) {
    StaticSemaphore_t* s = (StaticSemaphore_t*)malloc(sizeof(*s));
    if (s != NULL) {
        s->taken = 0;
    }
    return s;
}

static inline BaseType_t xSemaphoreTake(SemaphoreHandle_t s, TickType_t timeout) {
    (void)timeout;
    if (s == NULL || s->taken) {
        return pdFALSE;  // already held — in a 1-thread test this means misuse
    }
    s->taken = 1;
    return pdTRUE;
}

static inline BaseType_t xSemaphoreGive(SemaphoreHandle_t s) {
    if (s == NULL) {
        return pdFALSE;
    }
    s->taken = 0;
    return pdTRUE;
}

static inline void vSemaphoreDelete(SemaphoreHandle_t s) { free(s); }

#endif  // SHIM_FREERTOS_SEMPHR_H
