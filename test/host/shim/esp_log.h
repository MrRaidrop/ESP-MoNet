// SPDX-License-Identifier: MIT
// Host shim: ESP-IDF logging reduced to no-ops, plus a zero timestamp source.
#ifndef SHIM_ESP_LOG_H
#define SHIM_ESP_LOG_H

#include <stdint.h>

#define ESP_LOGE(tag, ...) do { (void)(tag); } while (0)
#define ESP_LOGW(tag, ...) do { (void)(tag); } while (0)
#define ESP_LOGI(tag, ...) do { (void)(tag); } while (0)
#define ESP_LOGD(tag, ...) do { (void)(tag); } while (0)

static inline uint32_t esp_log_timestamp(void) { return 0; }

#endif  // SHIM_ESP_LOG_H
