#include <stdio.h>             // for snprintf
#include <string.h>            // for string ops if needed
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "light_sensor_service.h"   // for light_sensor_get_cached_value()
#include "json_utils.h"            // for build_json_payload()
#include "https_post.h"            // for http_post_json()


#define TAG "data_reporter"
#define POST_INTERVAL_MS 5000  // 每 5 秒发送一次


static int post_counter = 0;

static void post_task(void *pvParameters)
{

    while (1) {
        int light_val = light_sensor_get_cached_value();

        char data_str[64];
        snprintf(data_str, sizeof(data_str), "light: %d", light_val);

        char *json = build_json_payload(data_str, &post_counter);
        http_post_json(json);
        ESP_LOGI(TAG, "Posting data: %s", json);

        vTaskDelay(pdMS_TO_TICKS(POST_INTERVAL_MS));
    }
}

void data_reporter_start(void)
{
    xTaskCreate(post_task, "post_task", 4096, NULL, 5, NULL);
}
