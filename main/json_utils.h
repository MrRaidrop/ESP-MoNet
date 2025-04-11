char *build_json_payload(const char *uart_data, int *counter);
// json_utils.h
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 构建一个包含时间戳、UART 数据与计数器的 JSON 字符串
 *
 * @param uart_data 串口数据内容
 * @param counter 发送计数器指针（每次调用会自增）
 * @return 指向静态 JSON 缓冲区的字符串指针（非线程安全）
 */
char *build_json_payload(const char *uart_data, int *counter);

#ifdef __cplusplus
}
#endif
