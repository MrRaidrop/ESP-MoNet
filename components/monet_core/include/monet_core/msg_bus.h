#ifndef MONET_CORE_MSG_BUS_H
#define MONET_CORE_MSG_BUS_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <stdint.h>
#include <stdbool.h>
#include "monet_hal/camera_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumeration of supported message topics on the bus.
 *
 * Each topic represents a logical channel for publishing/subscribing sensor or system events.
 * Topics can be extended for new sensors or control systems.
 */
typedef enum {
    EVENT_SENSOR_TEMP,           ///< Temperature and humidity sensor
    EVENT_SENSOR_LIGHT,          ///< Light sensor data (ADC)
    EVENT_SENSOR_UART,           ///< Structured UART payload from external sensor
    EVENT_SENSOR_JPEG,           ///< JPEG image captured from camera

    EVENT_SENSOR_MAX,            ///< Sentinel value for topic range

    EVENT_GROUP_SENSOR = 0x1000  ///< Wildcard group ID for all sensor topics
} msg_topic_t;

#define MSG_JPEG_BUF_SIZE 32768  ///< Maximum size of JPEG buffer in bytes

/**
 * @brief Message structure used to exchange data across services.
 */
typedef struct {
    msg_topic_t topic;           ///< Logical topic category
    uint32_t ts_ms;              ///< Timestamp in milliseconds (use esp_log_timestamp())

    union {
        int32_t value_int;       ///< Integer value (e.g. raw ADC)
        float value_float;       ///< Floating-point value (e.g. voltage, sensor float)

        struct {
            float temperature;   ///< Celsius degrees
            float humidity;      ///< Relative humidity percent
        } temp_hum;

        struct {
            char str[32];        ///< Text message received from UART, null-terminated
        } uart_text;

        struct {
            uint8_t data[64];    ///< Raw binary payload (if needed in future)
        } raw;

        struct {
            float accel[3];     ///< Accelerometer and gyroscope data, I have one in my stock
            float gyro[3];
        } imu;                   

        struct {
            camera_fb_t *fb;     ///< Frame buffer pointer for camera images
        } jpeg;
    } data;

} msg_t;

/**
 * @brief Publish a message to the corresponding topic's subscribers.
 *
 * The message is copied to all queues subscribed to this specific topic
 * and to any group-level subscriber (e.g. EVENT_GROUP_SENSOR).
 *
 * @param msg Pointer to the message object to broadcast
 */
void msg_bus_publish(const msg_t* msg);

/**
 * @brief Subscribe a queue to receive messages for a specific topic.
 *
 * @param topic Specific topic to subscribe to
 * @param queue Queue handle that will receive `msg_t`
 * @return true if successfully subscribed, false otherwise
 */
bool msg_bus_subscribe(msg_topic_t topic, QueueHandle_t queue);

/**
 * @brief Subscribe a queue to receive all messages within a topic group.
 *
 * For example, subscribing to EVENT_GROUP_SENSOR will receive all sensor-related events.
 * This allows transport services (UART, BLE, etc.) to be sink-only modules without editing
 * per-sensor logic.
 *
 * @param group_id Identifier of the event group (e.g., EVENT_GROUP_SENSOR)
 * @param queue Queue handle to receive messages
 * @return true if successfully added, false otherwise
 */
bool msg_bus_subscribe_group(uint16_t group_id, QueueHandle_t queue);

/**
 * @brief Legacy: Subscribe to all topics regardless of group.
 *
 * Should only be used for diagnostic/debugging purposes.
 *
 * @param q Queue to receive all traffic
 * @return true on success, false if registration fails
 */
bool msg_bus_subscribe_any(QueueHandle_t q);

#ifdef __cplusplus
}
#endif

#endif // MONET_CORE_MSG_BUS_H
