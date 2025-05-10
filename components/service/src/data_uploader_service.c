// components/service/src/data_uploader_service.c

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "service/data_uploader_service.h"
#include "monet_core/msg_bus.h"
#include "monet_codec/json_encoder.h"
#include "utils/log.h"
#include "utils/cache.h"
#include "utils/config.h"
#include "net/https_post_hal.h"
#include "monet_hal/camera_hal.h"
#include "service/ble_service.h"
#include "service/wifi_service.h"
#include "monet_core/service_registry.h"


#define TAG "DATA_UPLOADER"
#define STACK_SIZE 4096
#define TASK_PRIORITY 5
#define QUEUE_LENGTH 4



static void light_uploader_task(void* pv)
{
    QueueHandle_t queue = xQueueCreate(QUEUE_LENGTH, sizeof(msg_t));
    if (!msg_bus_subscribe(EVENT_SENSOR_LIGHT, queue)) {
        LOGE(TAG, "Failed to subscribe to EVENT_SENSOR_LIGHT");
        vTaskDelete(NULL);
        return;
    }
    LOGI(TAG, "Subscribed to light sensor events");

    msg_t msg;
    char json_buf[128];
    while (1) {
        if (xQueueReceive(queue, &msg, portMAX_DELAY) == pdTRUE) {
            // Convert light sensor data to JSON format
            if (!json_encode_msg(&msg, json_buf, sizeof(json_buf))) {
                LOGW(TAG, "Unsupported message topic: %d", msg.topic);
                continue;
            }
            bool success = false;

            // Try sending via Wi-Fi
            if (wifi_service_is_connected()) {
                success = http_post_send(json_buf);
                if (success) {
                    LOGI(TAG, "[HTTP] sent: %s", json_buf);
                }
            }
            // If Wi-Fi fails, try BLE
            else if (ble_service_is_connected()) {
                success = ble_service_notify_raw((const uint8_t *)json_buf, strlen(json_buf));
                if (success) {
                    LOGI(TAG, "[BLE] sent: %s", json_buf);
                }
            }

            // If neither succeeds, cache it
            if (!success) {
                LOGW(TAG, "[CACHE] saving: %s", json_buf);
                cache_push(json_buf);
            }
        }

        // Retry sending cached JSON entries
        if (wifi_service_is_connected()) {
            cache_flush_once_with_sender(http_post_send);
        } else if (ble_service_is_connected()) {
            // Note: BLE uses uint8_t*, so we typecast carefully
            cache_flush_once_with_sender((bool (*)(const char *))ble_service_notify_raw);
        }
    }
}

static void jpeg_uploader_task(void* pv)
{
    QueueHandle_t queue = xQueueCreate(QUEUE_LENGTH, sizeof(msg_t));
    if (!msg_bus_subscribe(EVENT_SENSOR_JPEG, queue)) {
        LOGE(TAG, "Failed to subscribe to EVENT_SENSOR_JPEG");
        vTaskDelete(NULL);
        return;
    }
    LOGI(TAG, "Subscribed to JPEG events");

    msg_t msg;
    while (1) {
        if (xQueueReceive(queue, &msg, portMAX_DELAY) == pdTRUE) {
            camera_fb_t *fb = msg.data.jpeg.fb;
            if (!fb || !fb->buf || fb->len == 0) {
                LOGW(TAG, "Invalid JPEG frame received");
                if (msg.release) msg.release(&msg);  // release fb
                continue;
            }

            bool success = false;

            // Try sending JPEG via Wi-Fi only
            if (wifi_service_is_connected()) {
                success = http_post_image(fb->buf, fb->len);
                if (success) {
                    LOGI(TAG, "[HTTP] JPEG sent (%u bytes)", (unsigned)fb->len);
                }
            }

            if (!success) {
                LOGW(TAG, "[CACHE] JPEG upload failed, saving to cache");
                cache_push_blob(fb->buf, fb->len);
            }

            if (wifi_service_is_connected()) {
                cache_flush_once_with_sender_ex(http_post_image);
            }

            if (msg.release) msg.release(&msg);
        }
    }
}


/**
 * @brief Legacy entry point for uploader tasks, for testing purposes.
 * @note Prefer using `service_registry` with get_*_uploader_service()
 */

void data_uploader_service_start(void)
{
#ifdef CONFIG_UPLOAD_LIGHT_ENABLED
    xTaskCreate(light_uploader_task, "light_uploader_task", STACK_SIZE, NULL, TASK_PRIORITY, NULL);
#endif

#ifdef CONFIG_UPLOAD_JPEG_ENABLED
    xTaskCreate(jpeg_uploader_task, "jpeg_uploader_task", STACK_SIZE, NULL, TASK_PRIORITY, NULL);
#endif
}

/**
 * @brief Descriptor for JPEG uploader service (uploads EVENT_SENSOR_JPEG)
 */
static void jpeg_service_entry(void *arg) { jpeg_uploader_task(arg); }

const service_desc_t data_uploader_jpeg_desc = {
    .name        = "jpeg_uploader",
    .task_fn     = jpeg_service_entry,
    .task_name   = "jpeg_uploader_task",
    .stack_size  = STACK_SIZE,
    .priority    = TASK_PRIORITY,
    .role        = SERVICE_ROLE_SUBSCRIBER,
    .topics      = (const msg_topic_t[]){ EVENT_SENSOR_JPEG, MSG_TOPIC_END }
};

/**
 * @brief Descriptor for Light uploader service (uploads EVENT_SENSOR_LIGHT)
 */
static void light_service_entry(void *arg) { light_uploader_task(arg); }

const service_desc_t data_uploader_light_desc = {
    .name        = "light_uploader",
    .task_fn     = light_service_entry,
    .task_name   = "light_uploader_task",
    .stack_size  = STACK_SIZE,
    .priority    = TASK_PRIORITY,
    .role        = SERVICE_ROLE_SUBSCRIBER,
    .topics      = (const msg_topic_t[]){ EVENT_SENSOR_LIGHT, MSG_TOPIC_END }
};

/**
 * @brief Get the service descriptor for JPEG uploader
 * @return Pointer to `service_desc_t` for registration
 */
const service_desc_t* get_jpeg_uploader_service(void)
{
    return &data_uploader_jpeg_desc;
}

/**
 * @brief Get the service descriptor for Light uploader
 * @return Pointer to `service_desc_t` for registration
 */
const service_desc_t* get_light_uploader_service(void)
{
    return &data_uploader_light_desc;
}
//should not be like that, if a sensor node is added, then user need to 
//add a uploader task, that's not clean at all.