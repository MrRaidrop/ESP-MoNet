// uart_handler.h
#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UART_PORT_NUM      UART_NUM_1
#define TXD_PIN            GPIO_NUM_17
#define RXD_PIN            GPIO_NUM_18
#define UART_BUF_SIZE      128

/**
 * @brief 初始化 UART 硬件、创建队列和接收任务
 */
void uart_init(void);

/**
 * @brief 获取内部接收队列的句柄
 */
QueueHandle_t uart_get_queue(void);

/**
 * @brief 向串口写入一串字符串（会自动加 \r\n）
 */
void uart_write_string(const char *str);

#ifdef __cplusplus
}
#endif
