#pragma once

#include "esp_log.h"

// 如果你想开启日志，在编译选项中添加 -DDEBUG_LOG
#ifdef DEBUG_LOG

// 不做处理，使用 esp_log 原始定义
// nothing to do

#else
// 重定义 ESP_LOGX 宏为空宏，只影响当前包含此头文件的 .c 文件

#undef ESP_LOGI
#undef ESP_LOGW
#undef ESP_LOGE
#undef ESP_LOGD
#undef ESP_LOGV

#define ESP_LOGI(tag, fmt, ...) do {} while (0)
#define ESP_LOGW(tag, fmt, ...) do {} while (0)
#define ESP_LOGE(tag, fmt, ...) do {} while (0)
#define ESP_LOGD(tag, fmt, ...) do {} while (0)
#define ESP_LOGV(tag, fmt, ...) do {} while (0)

#endif
