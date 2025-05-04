// core/include/core/msg_bus.h
#ifndef MSG_BUS_H_
#define MSG_BUS_H_

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <stdint.h>
#include <stdbool.h>
#include "my_hal/camera_hal.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumeration of supported message topics on the bus.
 *
 * Each topic represents a logical channel for publishing/subscribing sensor or system events.
 * These can be extended as needed for new services.
 */
typedef enum {
    EVENT_SENSOR_TEMP,       ///< Your sensor here
    EVENT_SENSOR_LIGHT,  ///< Light sensor data (ADC)
    EVENT_SENSOR_UART,       ///< Structured UART payload from external sensor
    EVENT_SENSOR_JPEG,       ///< JPEG image captured from camera
    EVENT_SENSOR_MAX         ///< Sentinel value for bounds checking
} msg_topic_t;

#define MSG_JPEG_BUF_SIZE 32768  ///< Maximum size of JPEG buffer in bytes

/**
 * @brief Message payload structure supporting multiple data types.
 *
 * This union allows sharing a common bus structure while enabling flexibility.
 * For each topic, the payload format must be predefined and respected.
 */
typedef struct {
    msg_topic_t topic;   ///< Logical topic category
    uint32_t ts_ms;      ///< Timestamp in milliseconds (use esp_log_timestamp())

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
            float accel[3];
            float gyro[3];
        } imu;

        struct {
            camera_fb_t *fb;
        } jpeg;
        // This is definatily not enough for a full image, but we can add package splicing later.
    } data;

} msg_t;

/**
 * @brief Publishes a message to all subscribers of the specified topic.
 *
 * The message is copied into the subscribers' queues. The caller should
 * construct and fill a `msg_t` object before calling.
 *
 * @param msg Pointer to the message to publish
 */
void msg_bus_publish(const msg_t* msg);

/**
 * @brief Subscribes a FreeRTOS queue to receive messages of a specific topic.
 *
 * Multiple queues can subscribe to a topic (up to internal limits).
 *
 * @param topic Topic to subscribe to
 * @param queue A FreeRTOS queue created by the caller to receive `msg_t`
 * @return true on success, false if maximum subscribers reached
 */
bool msg_bus_subscribe(msg_topic_t topic, QueueHandle_t queue);

#ifdef __cplusplus
}
#endif

#endif // MSG_BUS_H_
