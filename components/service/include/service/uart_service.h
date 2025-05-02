#ifndef UART_SERVICE_H_
#define UART_SERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uart_service.h
 * @brief UART bridge service for message bus input/output.
 *
 * This module integrates the UART with the system message bus. It runs two FreeRTOS tasks:
 *
 * - One subscribes to EVENT_SENSOR_LIGHT messages and forwards them as formatted strings over UART.
 * - The other reads incoming UART lines and publishes them as EVENT_SENSOR_UART messages.
 *
 * This service can be used for diagnostics, command interface, or transparent bridging to other systems.
 */

/**
 * @brief Start the UART service tasks.
 *
 * Initializes the UART hardware using uart_hal_init(), then launches two FreeRTOS tasks:
 * - One sends formatted messages from EVENT_SENSOR_LIGHT to UART.
 * - One receives UART text lines and publishes them to msg_bus.
 */
void uart_service_start(void);

#ifdef __cplusplus
}
#endif

#endif // UART_SERVICE_H_
