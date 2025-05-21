// components/hal/include/hal/uart_hal.h
#ifndef UART_HAL_H_
#define UART_HAL_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uart_hal.h
 * @brief Hardware abstraction layer for UART communication.
 *
 * This module encapsulates ESP-IDF UART driver calls and exposes a simplified
 * interface for receiving and sending UART strings. It runs a background
 * FreeRTOS task to receive data asynchronously and pushes received strings
 * to a message queue for further processing.
 */

/**
 * @brief Initialize UART hardware and launch receiver task.
 *
 * This sets up UART1 with default 115200 baud rate and configures TX/RX pins.
 * It creates an RX queue of size 5 and launches a FreeRTOS task that waits
 * for UART input and pushes it to the queue.
 */
void monet_uart_hal_init(void);

/**
 * @brief Get the UART RX queue.
 *
 * This queue holds `char *` pointers to strings received by UART. The
 * receiver is responsible for freeing the memory of dequeued strings.
 *
 * @return QueueHandle_t The internal receive queue.
 */
QueueHandle_t monet_uart_hal_get_rx_queue(void);

/**
 * @brief Write a string to the UART port, followed by CRLF.
 *
 * @param str Null-terminated string to transmit.
 */
void monet_uart_hal_write_string(const char *str);


/**
 * @brief Write a byte array to the UART port, and print the length.
 *
 * @param data Pointer to the byte array to transmit.
 * @param length Number of bytes to transmit.
 */
void monet_uart_hal_write_bytes(const uint8_t *data, size_t length);


#ifdef __cplusplus
}
#endif

#endif // UART_HAL_H_
