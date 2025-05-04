#include "service/dht22_service.h"
#include "my_hal/dht22_hal.h"
#include "core/msg_bus.h"
#include "utils/log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "DHT22_SERVICE"
#define INTERVAL_MS 10000

static void dht22_task(void *param)
{
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
        }
        vTaskDelay(pdMS_TO_TICKS(INTERVAL_MS));
    }
}

void dht22_service_start(void)
{
    if (dht22_hal_init() != ESP_OK) {
        LOGE(TAG, "DHT22 init failed");
        return;
    }
    xTaskCreate(dht22_task, "dht22_task", 4096, NULL, 5, NULL);
}