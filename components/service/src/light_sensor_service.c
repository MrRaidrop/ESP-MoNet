#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>

#include "service/light_sensor_service.h"
#include "monet_hal/adc_hal.h"
#include "utils/config.h"
#include "utils/log.h"
#include "monet_core/msg_bus.h"


#undef light_sensor_task


#define TAG "LIGHT_SENSOR"
#define LIGHT_SENSOR_TASK_STACK_SIZE 4096
#define LIGHT_SENSOR_TASK_PRIORITY   5
#define LIGHT_SENSOR_THRESHOLD       2000

static int latest_light_value = 0;

int light_sensor_get_cached_value(void) {
    return latest_light_value;
}

static void light_sensor_task(void *pvParameters)
{
    adc_hal_init(CONFIG_LIGHT_SENSOR_CHANNEL);
    LOGI(TAG, "Driver initialized on channel %d", CONFIG_LIGHT_SENSOR_CHANNEL);
    LOGI(TAG, "Light sensor task started");
    while (1) {
        latest_light_value = adc_hal_read(CONFIG_LIGHT_SENSOR_CHANNEL);
        LOGI(TAG, "Raw ADC: %d", latest_light_value);

        // Publish to message bus
        msg_t msg = {
            .topic = EVENT_SENSOR_LIGHT,
            .ts_ms = esp_log_timestamp(),
            .data.value_int = latest_light_value
        };
        msg_bus_publish(&msg);

        // Optional: local threshold log
        if (latest_light_value > LIGHT_SENSOR_THRESHOLD) {
            LOGW(TAG, "Dark or covered");
        } else {
            LOGI(TAG, "Bright environment");
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


static const service_desc_t light_sensor_service_desc = {
    .name = "light_sensor_service",
    .task_fn = light_sensor_task,
    .task_name = "light_sensor_task",
    .stack_size = LIGHT_SENSOR_TASK_STACK_SIZE,
    .priority = LIGHT_SENSOR_TASK_PRIORITY
};

// Register the service on main
const service_desc_t* get_light_sensor_service(void) {
    return &light_sensor_service_desc;
}
