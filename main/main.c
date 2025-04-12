#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "uart_handler.h"
#include "wifi_manager.h"
#include "json_utils.h"
#include "log_wrapper.h"
#include "https_post.h"
#include "light_sensor_service.h"

#define WIFI_SSID      "zhenghao的iPhone"
#define WIFI_PASS      "12345678"
#define MAX_RETRY      100

static int post_counter = 0;

static void uart_light_send_task(void *pvParameters)
{
    ESP_LOGI(TAG, "UART light send task started");
    while (1) {
        int val = light_sensor_get_cached_value();
        char msg[64];
        snprintf(msg, sizeof(msg), "Light ADC: %d", val);
        uart_write_string(msg);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void uart_service_start(void)
{
    uart_init();
    xTaskCreate(uart_light_send_task, "uart_light_send_task", 2048, NULL, 5, NULL);
    ESP_LOGI("uart", "UART service started");
}

static void post_task(void *pvParameters)
{
    EventGroupHandle_t wifi_event = wifi_init_sta(WIFI_SSID, WIFI_PASS);

    EventBits_t bits = xEventGroupWaitBits(wifi_event, WIFI_CONNECTED_BIT,
                                           pdFALSE, pdTRUE, portMAX_DELAY);
    if (!(bits & WIFI_CONNECTED_BIT)) {
        ESP_LOGE(TAG, "Failed to connect to Wi-Fi after %d retries.", MAX_RETRY);
        vTaskDelete(NULL);
    }

    static char latest_data[128] = "default";
    const TickType_t delay_ticks = pdMS_TO_TICKS(5000);
    QueueHandle_t queue = uart_get_queue();

    while (1) {
        vTaskDelay(delay_ticks);

        uart_write_string(latest_data);

        char *uart_data = NULL;
        if (xQueueReceive(queue, &uart_data, 0)) {
            strncpy(latest_data, uart_data, sizeof(latest_data) - 1);
            latest_data[sizeof(latest_data) - 1] = '\0';
            free(uart_data);
        }

        char *json = build_json_payload(latest_data, &post_counter);
        http_post_json(json);
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    light_sensor_service_start();
    uart_service_start();

    xTaskCreate(post_task, "post_task", 8192, NULL, 5, NULL);
}
