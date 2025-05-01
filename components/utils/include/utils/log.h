/* utils/log.h */
#ifndef LOG_H_
#define LOG_H_

#include "esp_log.h"
#include "config.h"

#if CONFIG_LOG_ENABLE
#define LOGI(tag, fmt, ...) ESP_LOGI(tag, fmt, ##__VA_ARGS__)
#define LOGW(tag, fmt, ...) ESP_LOGW(tag, fmt, ##__VA_ARGS__)
#define LOGE(tag, fmt, ...) ESP_LOGE(tag, fmt, ##__VA_ARGS__)
#else
#define LOGI(tag, fmt, ...)
#define LOGW(tag, fmt, ...)
#define LOGE(tag, fmt, ...)
#endif

#endif /* LOG_H_ */


// ESP_LOGI("LIGHT", "ADC Value: %d", val);
// Change to
// LOGI("LIGHT", "ADC Value: %d", val);
