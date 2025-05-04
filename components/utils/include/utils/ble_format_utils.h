#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 构建一个包含时间戳、UART 数据与计数器�?JSON 字符�?
 *
 * @param uart_data 串口数据内容
 */
void ble_format_notify_data(int value, uint8_t *out);

#ifdef __cplusplus
}
#endif
