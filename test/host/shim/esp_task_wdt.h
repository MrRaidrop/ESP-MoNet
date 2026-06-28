// SPDX-License-Identifier: MIT
// Host shim: the Task Watchdog API as no-ops returning success.
#ifndef SHIM_ESP_TASK_WDT_H
#define SHIM_ESP_TASK_WDT_H

#ifndef ESP_OK
typedef int esp_err_t;
#define ESP_OK 0
#endif

static inline esp_err_t esp_task_wdt_add(void* handle) {
    (void)handle;
    return ESP_OK;
}

static inline esp_err_t esp_task_wdt_reset(void) { return ESP_OK; }

#endif  // SHIM_ESP_TASK_WDT_H
