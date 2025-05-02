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

#include "utils/json_utils.h"
#include "utils/log_wrapper.h"
#include "net/https_post.h"
#include "service/light_sensor_service.h"
#include "service/data_reporter.h"
#include "service/ble_service.h"
#include "service/wifi_service.h"
#include "service/uart_service.h"
#include "ota/https_ota_service.h" 
#include "service/data_uploader_service.h"

#define TAG "MAIN"

#define WIFI_SSID      "zhenghao的iPhone"
#define WIFI_PASS      "12345678"

// For testing OTA update logic after 30s
#define FIRMWARE_VERSION "hello this version 2"



// ====================== OTA test task =========================== //
/**
 * @brief This task is used to simulate a firmware update after 30 seconds.
 * It will print version info + counter every second, and then trigger OTA.
 */
// the firmware.bin on the server just contain a simple task: hello version 1.
static void ota_test_task(void *param)
{
    for (int i = 1; i <= 30; i++) {
        ESP_LOGI(TAG, "%s, count: %d", FIRMWARE_VERSION, i);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ESP_LOGI(TAG, "30s reached, starting OTA...");
    ota_service_start();  // 调用你已有的 OTA 启动函数
    vTaskDelete(NULL);
}
// ============================================================= //

void app_main(void)
{
    // Initialize NVS safely
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    if (!wifi_service_start_and_wait(WIFI_SSID, WIFI_PASS, 10000)) {
        ESP_LOGE(TAG, "Wi-Fi failed");
        return;
    }

    light_sensor_service_start();
    ble_service_start();
    //uart_service_start();
    //data_reporter_start();
    vTaskDelay(pdMS_TO_TICKS(500));
    data_uploader_service_start();

    //xTaskCreate(&ota_test_task, "ota_test_task", 4096, NULL, 5, NULL);
}
