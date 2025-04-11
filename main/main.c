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

#define WIFI_SSID      "zhenghao的iPhone"
#define WIFI_PASS      "12345678"
#define MAX_RETRY      100

static int post_counter = 0;

static void uart_hello_task(void *pvParameters)
{
    const char *hello_str = "hello from esp32";
    while (1) {
        uart_write_string(hello_str);  // 自动添加 \r\n
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void post_task(void *pvParameters)
{
    EventGroupHandle_t wifi_event = wifi_init_sta(WIFI_SSID, WIFI_PASS);

    // 等待 Wi-Fi 连接
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

    // 初始化 UART 模块（含任务）
    uart_init();

    // 启动 Hello 串口打印任务
    xTaskCreate(uart_hello_task, "uart_hello_task", 2048, NULL, 5, NULL);

    // 启动周期 POST 任务
    xTaskCreate(post_task, "post_task", 8192, NULL, 5, NULL);
}
