/**
 * @file camera_service.c
 * @brief Captures JPEG frames and publishes them on EVENT_SENSOR_JPEG.
 *        Frame interval can be *fixed* or *RSSI-adaptive* (menuconfig).
 */
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "service/camera_service.h"
#include "monet_hal/camera_hal.h"
#include "monet_hal/wifi_hal.h"
#include "monet_core/msg_bus.h"
#include "utils/log.h"
#include <inttypes.h>

#define TAG "CAMERA_SERVICE"

/* ──────────────────────────────────────────
 * Helper – choose interval
 * ────────────────────────────────────────── */
static uint32_t choose_interval_ms(void)
{
#if CONFIG_CAMERA_DYNAMIC_INTERVAL
    int rssi = wifi_get_rssi();                          /* HAL wrapper */
    uint32_t interval;
    if (rssi >= CONFIG_CAMERA_RSSI_FAST_THRESH) {
        interval = CONFIG_CAMERA_INTERVAL_FAST_MS;
    } else if (rssi >= CONFIG_CAMERA_RSSI_MED_THRESH) {
        interval = CONFIG_CAMERA_INTERVAL_MED_MS;
    } else {
        interval = CONFIG_CAMERA_INTERVAL_SLOW_MS;
    }
    LOGI(TAG, "RSSI %d dBm → interval %" PRIu32 " ms", rssi, interval);
    return interval;
#else
    return CONFIG_CAMERA_CAPTURE_INTERVAL_MS;
#endif
}

/* ──────────────────────────────────────────
 * Release helper – zero-copy JPEG buffer
 * ────────────────────────────────────────── */
static void release_camera_frame(msg_t *msg)
{
    if (msg && msg->topic == EVENT_SENSOR_JPEG && msg->data.jpeg.fb) {
        esp_camera_fb_return(msg->data.jpeg.fb);
        msg->data.jpeg.fb = NULL;
    }
}

/* ──────────────────────────────────────────
 * Main task
 * ────────────────────────────────────────── */
static void camera_service_task(void *param)
{
    if (camera_hal_init() != ESP_OK) {
        LOGE(TAG, "Camera init failed – task not started");
        vTaskDelete(NULL);
        return;
    }

    while (true) {
        /* 1. Capture */
        camera_fb_t *fb = camera_hal_capture();
        if (!fb) {
            LOGW(TAG, "Capture failed");
            vTaskDelay(pdMS_TO_TICKS(choose_interval_ms()));
            continue;
        }

        /* 2. Publish zero-copy */
        msg_t msg = {
            .topic   = EVENT_SENSOR_JPEG,
            .ts_ms   = esp_log_timestamp(),
            .data.jpeg.fb = fb,
            .release = release_camera_frame
        };
        msg_bus_publish(&msg);

        /* 3. Wait */
        vTaskDelay(pdMS_TO_TICKS(choose_interval_ms()));
    }
}

/* ──────────────────────────────────────────
 * Service descriptor
 * ────────────────────────────────────────── */
static const service_desc_t camera_service_desc = {
    .name       = "camera_service",
#if CONFIG_CAMERA_SERVICE_ENABLE     /* 由之前的 bool 控制整体启停 */
    .task_fn    = camera_service_task,
#else
    .task_fn    = NULL,
#endif
    .task_name  = "camera_service_task",
    .stack_size = 8192,
    .priority   = 5,
    .role       = SERVICE_ROLE_PUBLISHER
};

const service_desc_t *get_camera_service(void)
{
    return &camera_service_desc;
}
