#ifndef UART_SERVICE_H
#define UART_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Starts UART service for light sensor streaming via UART.
 *        This creates a background task that sends sensor values periodically.
 */
void uart_service_start(void);

#ifdef __cplusplus
}
#endif

#endif // UART_SERVICE_H
