#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "light_sensor_service.h"
#include "json_utils.h"
#include "https_post.h"
#include "wifi_manager.h"

#define TAG "data_reporter"

#define POST_INTERVAL_MS 5000
#define WIFI_SSID "zhenghao的iPhone"
#define WIFI_PASS "12345678"

static int post_counter = 0;

static void post_task(void *pvParameters)
{
    EventGroupHandle_t wifi_event = wifi_init_sta(WIFI_SSID, WIFI_PASS);
    EventBits_t bits = xEventGroupWaitBits(wifi_event, WIFI_CONNECTED_BIT,
                                           pdFALSE, pdTRUE, portMAX_DELAY);

    if (!(bits & WIFI_CONNECTED_BIT)) {
        ESP_LOGE(TAG, "Wi-Fi failed");
        vTaskDelete(NULL);
    }

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
