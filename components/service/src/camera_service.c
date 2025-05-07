#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "service/camera_service.h"
#include "monet_hal/camera_hal.h"
#include "monet_hal/wifi_hal.h"
#include "monet_core/msg_bus.h"
#include "utils/log.h"
#include "utils/config.h"
#include <string.h>
#include <inttypes.h>

#define TAG "CAMERA_SERVICE"
static uint32_t dynamic_capture_interval_ms = 1000;
// Let's change that into: if wifi is good, then upload fps bigger


static void update_capture_interval(void)
{
    int rssi = wifi_get_rssi();  // esp_wifi_sta_get_rssi() in wifi_hal.c
    if (rssi >= -60) {
        dynamic_capture_interval_ms = 333;
    } else if (rssi >= -70) {
        dynamic_capture_interval_ms = 1000;
    } else {
        dynamic_capture_interval_ms = 5000;
    }
    LOGI(TAG, "RSSI = %d, interval = %" PRIu32 " ms", rssi, dynamic_capture_interval_ms);
}


// Camera service task function
static void camera_service_task(void *param)
{
    if (camera_hal_init() != ESP_OK) {
        LOGE(TAG, "Camera initialization failed. Service not started.");
        return;
    }
    while (1) {
        // Capture a frame from the camera
        camera_fb_t *fb = camera_hal_capture();
        if (!fb) {
            LOGW(TAG, "Capture failed");
            vTaskDelay(pdMS_TO_TICKS(dynamic_capture_interval_ms));
            continue;
        }

        // Prepare the message to publish
        msg_t msg = {
            .topic = EVENT_SENSOR_JPEG,
            .ts_ms = esp_log_timestamp(),
        };

        // Zero copy
        msg.data.jpeg.fb = fb;

        msg_bus_publish(&msg);

        update_capture_interval();
        vTaskDelay(pdMS_TO_TICKS(dynamic_capture_interval_ms));
        vTaskDelay(pdMS_TO_TICKS(10000)); // 10 seconds delay for testing
    }
}

static const service_desc_t camera_service_desc = {
    .name = "camera_service",
    .task_fn = camera_service_task,
    .task_name = "camera_service_task",
    .stack_size = 8192,
    .priority = 5
};

const service_desc_t* get_camera_service(void)
{
    return &camera_service_desc;
}
