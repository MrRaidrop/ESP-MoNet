#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi_service.h"
#include "https_ota_service.h"

#define TAG "MAIN"

#define WIFI_SSID      "zhenghao的iPhone"
#define WIFI_PASS      "12345678"

// This is for OTA testing
#define FIRMWARE_VERSION "hello this version 2"

void counting_task(void *param) {
    for (int count = 1; count <= 20; count++) {
        ESP_LOGI(TAG, "%s, count: %d", FIRMWARE_VERSION, count);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ESP_LOGI(TAG, "Counting finished, starting OTA update...");
    ota_service_start();  // Starts OTA (defined in https_ota_service.c)
    vTaskDelete(NULL);
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting system...");
    ESP_ERROR_CHECK(nvs_flash_init());

    if (!wifi_service_start_and_wait(WIFI_SSID, WIFI_PASS, 10000)) {
        ESP_LOGE(TAG, "Wi-Fi failed");
        return;
    }
    

    ESP_LOGI(TAG, "Wi-Fi connected. Starting counting task...");
    xTaskCreate(&counting_task, "counting_task", 4096, NULL, 5, NULL);
}
