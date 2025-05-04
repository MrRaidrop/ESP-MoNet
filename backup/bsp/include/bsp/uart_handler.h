#ifndef UART_HANDLER_H
#define UART_HANDLER_H

#include "freertos/queue.h"

// UART 配置参数
#define UART_PORT_NUM    UART_NUM_1
#define UART_BUF_SIZE    256
#define TXD_PIN          (GPIO_NUM_17)
#define RXD_PIN          (GPIO_NUM_16)

// 初始�?UART 及其接收任务
void uart_init(void);

// 启动 UART 服务：初始化 + 启动周期发送任务（建议 main 中调用）
void uart_service_start(void);

// �?UART 模块中获取接收队列句�?
QueueHandle_t uart_get_queue(void);

// �?UART 发送一个字符串（自动添�?\r\n�?
void uart_write_string(const char *str);

#endif // UART_HANDLER_H
