// SPDX-License-Identifier: MIT
// Host shim: a fixed-capacity ring buffer standing in for a FreeRTOS queue.
// Single-threaded — receive() never blocks; on empty it returns pdFALSE, on
// full send() returns pdFALSE. That is exactly the surface Queue<T,N> uses.
#ifndef SHIM_FREERTOS_QUEUE_H
#define SHIM_FREERTOS_QUEUE_H

#include <string.h>

#include "freertos/FreeRTOS.h"

typedef struct {
    uint8_t* storage;
    size_t   item_size;
    size_t   capacity;
    size_t   count;
    size_t   head;  // next read slot
    size_t   tail;  // next write slot
} StaticQueue_t;

typedef StaticQueue_t* QueueHandle_t;

static inline QueueHandle_t xQueueCreateStatic(UBaseType_t length,
                                               UBaseType_t item_size,
                                               uint8_t* storage,
                                               StaticQueue_t* buf) {
    buf->storage = storage;
    buf->item_size = item_size;
    buf->capacity = length;
    buf->count = 0;
    buf->head = 0;
    buf->tail = 0;
    return buf;
}

static inline BaseType_t xQueueSend(QueueHandle_t q, const void* item,
                                    TickType_t timeout) {
    (void)timeout;
    if (q->count == q->capacity) {
        return pdFALSE;  // errQUEUE_FULL
    }
    memcpy(q->storage + q->tail * q->item_size, item, q->item_size);
    q->tail = (q->tail + 1) % q->capacity;
    ++q->count;
    return pdTRUE;
}

static inline BaseType_t xQueueSendFromISR(QueueHandle_t q, const void* item,
                                           BaseType_t* woken) {
    if (woken != NULL) {
        *woken = pdFALSE;
    }
    return xQueueSend(q, item, 0);
}

static inline BaseType_t xQueueReceive(QueueHandle_t q, void* out,
                                       TickType_t timeout) {
    (void)timeout;
    if (q->count == 0) {
        return pdFALSE;
    }
    memcpy(out, q->storage + q->head * q->item_size, q->item_size);
    q->head = (q->head + 1) % q->capacity;
    --q->count;
    return pdTRUE;
}

static inline UBaseType_t uxQueueMessagesWaiting(QueueHandle_t q) {
    return (UBaseType_t)q->count;
}

static inline void vQueueDelete(QueueHandle_t q) { (void)q; }

#endif  // SHIM_FREERTOS_QUEUE_H
