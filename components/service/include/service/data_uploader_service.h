#ifndef DATA_UPLOADER_SERVICE_H_
#define DATA_UPLOADER_SERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file data_uploader_service.h
 * @brief Asynchronous uploader service for sensor data forwarding.
 *
 * This service subscribes to the internal message bus to receive sensor messages,
 * such as EVENT_SENSOR_LIGHT. It converts messages into JSON strings, and then
 * sends them over available transport layers:
 *
 * - If Wi-Fi is connected: sends via HTTP POST
 * - Else if BLE is connected: sends via BLE notify
 * - Else: logs offline warning and optionally caches the message for later retry
 *
 * Uploads are handled in a background FreeRTOS task.
 */

/**
 * @brief Start the uploader service task.
 *
 * This function initializes a queue, subscribes to sensor events via msg_bus,
 * and starts a FreeRTOS task to forward messages to the cloud or mobile clients.
 * It should be called after Wi-Fi and BLE services are started.
 */
void data_uploader_service_start(void);

#ifdef __cplusplus
}
#endif

#endif  // DATA_UPLOADER_SERVICE_H_
