// components/service/src/data_uploader_service.c
#include "service/data_uploader_service.h"
#include "core/msg_bus.h"
#include "utils/json_utils.h"
#include "utils/log.h"
#include "utils/cache.h"
#include "net/https_post_hal.h"
#include "service/ble_service.h"
#include "service/wifi_service.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "DATA_UPLOADER"
#define UPLOADER_STACK_SIZE 4096
#define UPLOADER_TASK_PRIORITY 5
#define UPLOADER_QUEUE_LENGTH 8

static void uploader_task(void* pv)
{
    QueueHandle_t queue = xQueueCreate(UPLOADER_QUEUE_LENGTH, sizeof(msg_t));
    if (!msg_bus_subscribe(EVENT_SENSOR_LIGHT, queue)) {
        LOGE(TAG, "Failed to subscribe to EVENT_SENSOR_LIGHT");
        vTaskDelete(NULL);
        return;
    }
    LOGI(TAG, "Uploader subscribed to light sensor events");

    msg_t msg;
    char json_buf[128];
    while (1) {
        if (xQueueReceive(queue, &msg, portMAX_DELAY) == pdTRUE) {
            json_utils_build_light_sensor_json(json_buf, sizeof(json_buf), msg.data.value_int, msg.ts_ms);
    
            bool success = false;
            if (wifi_service_is_connected()) {
                success = http_post_send(json_buf);
                if (success) LOGI(TAG, "[HTTP] sent: %s", json_buf);
            } else if (ble_service_is_connected()) {
                success = ble_service_notify_raw((const uint8_t *)json_buf, strlen(json_buf));
                if (success) LOGI(TAG, "[BLE] sent: %s", json_buf);
            }
    
            if (!success) {
                LOGW(TAG, "[CACHE] saving: %s", json_buf);
                cache_push(json_buf);
            }
        }
    
        // Try flushing cached items
        if (wifi_service_is_connected()) {
            cache_flush_once_with_sender(http_post_send);
        } else if (ble_service_is_connected()) {
            cache_flush_once_with_sender((bool (*)(const char *))ble_service_notify_raw);
        }
    }    
}
    

void data_uploader_service_start(void)
{
    xTaskCreate(uploader_task, "uploader_task", UPLOADER_STACK_SIZE,
                NULL, UPLOADER_TASK_PRIORITY, NULL);
}
