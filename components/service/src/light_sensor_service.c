#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#include "bsp/light_sensor_driver.h"
#include "service/light_sensor_service.h"

#define TAG "LIGHT_SENSOR"
#define LIGHT_SENSOR_TASK_STACK_SIZE 2048
#define LIGHT_SENSOR_TASK_PRIORITY   5
#define LIGHT_SENSOR_THRESHOLD       2000
#define LIGHT_SENSOR_ADC_CHANNEL     ADC1_CHANNEL_2 // GPIO3

static int latest_light_value = 0;

int light_sensor_get_cached_value(void) {
    return latest_light_value;
}

static void light_sensor_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Light sensor task started");
    while (1) {
        latest_light_value = light_sensor_driver_read_raw();
        //ESP_LOGI(TAG, "Raw ADC: %d", latest_light_value);

        if (latest_light_value > LIGHT_SENSOR_THRESHOLD) {
            //ESP_LOGW(TAG, "🌑 Dark or covered");
        } else {
            //ESP_LOGI(TAG, "🌞 Bright environment");
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void light_sensor_service_start(void)
{
    light_sensor_driver_init(LIGHT_SENSOR_ADC_CHANNEL);
    ESP_LOGI(TAG, "Driver initialized on channel %d", LIGHT_SENSOR_ADC_CHANNEL);
    xTaskCreate(light_sensor_task, "light_sensor_task",
                LIGHT_SENSOR_TASK_STACK_SIZE, NULL,
                LIGHT_SENSOR_TASK_PRIORITY, NULL);
}
