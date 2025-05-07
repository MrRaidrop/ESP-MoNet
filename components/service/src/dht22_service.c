#include "service/dht22_service.h"
#include "monet_hal/dht22_hal.h"
#include "monet_core/msg_bus.h"
#include "monet_core/service_registry.h"
#include "utils/log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "DHT22_SERVICE"
#define INTERVAL_MS 1000   // Polling interval in milliseconds, modify as needed

/**
 * @brief DHT22 sensor polling task
 *
 * Reads temperature and humidity, then publishes to EVENT_SENSOR_TEMP.
 * Start by the service registry.
 */
static void dht22_task(void *param)
{
    if (dht22_hal_init() != ESP_OK) {
        LOGE(TAG, "DHT22 init failed");
        vTaskDelete(NULL);
        return;
    }

    while (1) {
        float t, h;
        if (dht22_hal_read(&t, &h) == ESP_OK) {
            msg_t msg = {
                .topic = EVENT_SENSOR_TEMP,
                .ts_ms = esp_log_timestamp(),
            };
            msg.data.temp_hum.temperature = t;
            msg.data.temp_hum.humidity = h;
            msg_bus_publish(&msg);
            LOGI(TAG, "Published: %.2f°C %.2f%%", t, h);
        } else {
            LOGW(TAG, "Sensor read failed");
        }

        vTaskDelay(pdMS_TO_TICKS(INTERVAL_MS));
    }
}

/// Service descriptor for automatic registration
static const service_desc_t dht22_service_desc = {
    .name       = "dht22_service",
    .task_fn    = dht22_task,
    .task_name  = "dht22_task",
    .stack_size = 4096,
    .priority   = 5,
    .role       = SERVICE_ROLE_PUBLISHER,   // sensor is publisher
    .topics     = NULL                      // publisher doesn't subscribe
};

/**
 * @brief Getter for registry auto-registration.
 *
 * Should be called in main.c: service_registry_register(get_dht22_service());
 */
const service_desc_t* get_dht22_service(void)
{
    return &dht22_service_desc;
}
