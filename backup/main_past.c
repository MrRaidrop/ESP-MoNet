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
#include "data_reporter.h"
#include "ble_service.h"

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

static void ble_notify_task(void *pvParameters)
{
    while (1) {
        ble_service_notify_light_value();
        vTaskDelay(pdMS_TO_TICKS(3000));  // 每 3 秒通知一次
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    light_sensor_service_start();
    //uart_service_start();

    //data_reporter_start(); 
    ble_service_start();  // 启动 BLE 栈和服务

    xTaskCreate(ble_notify_task,           // Task 函数
                "ble_notify_task",         // 任务名
                2048,                      // 堆栈大小（可调）
                NULL,                      // 参数
                5,                         // 优先级
                NULL);                     // 任务句柄（可为 NULL）

    // 其他任务可继续创建...
}
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
#include "data_reporter.h"
#include "ble_service.h"

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


void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    light_sensor_service_start();
    //uart_service_start();

    //data_reporter_start(); 
    vTaskDelay(pdMS_TO_TICKS(5000));
    ble_service_start();  
}

