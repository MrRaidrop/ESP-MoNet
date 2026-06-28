// SPDX-License-Identifier: MIT
// Host shim: minimal FreeRTOS base types so the C++ layer compiles and runs
// under plain gcc/g++ for unit tests. NOT a real kernel — single-threaded,
// store-and-forward semantics that are enough for deterministic logic tests.
#ifndef SHIM_FREERTOS_H
#define SHIM_FREERTOS_H

#include <stddef.h>
#include <stdint.h>

typedef uint32_t     TickType_t;
typedef int          BaseType_t;
typedef unsigned int UBaseType_t;

#define pdTRUE        ((BaseType_t)1)
#define pdFALSE       ((BaseType_t)0)
#define pdPASS        pdTRUE
#define pdFAIL        pdFALSE
#define portMAX_DELAY ((TickType_t)0xffffffffUL)
#define pdMS_TO_TICKS(ms) ((TickType_t)(ms))

#endif  // SHIM_FREERTOS_H
