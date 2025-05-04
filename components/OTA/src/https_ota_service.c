#include "esp_log.h"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "OTA/https_ota_service.h"

#define OTA_URL "https://40.233.83.32:8443/firmware.bin"
#define TAG "OTA_SERVICE"

/**
 * @brief OTA任务（独立线程中运行�?
 */
static void ota_task(void *param) {
    ESP_LOGI(TAG, "启动 OTA 更新任务...");

    esp_http_client_config_t http_config = {
        .url = OTA_URL,
        .cert_pem = NULL,
        .timeout_ms = 10000,
    };
    
    esp_https_ota_config_t ota_config = {
        .http_config = &http_config,
    };
    
    esp_err_t ret = esp_https_ota(&ota_config);  
    

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "OTA success，reboot to update...");
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_restart();
    } else {
        ESP_LOGE(TAG, "OTA failed: %s", esp_err_to_name(ret));
    }

    vTaskDelete(NULL);  // 销毁任�?
}

/**
 * @brief 启动 OTA 服务（建议从主程序中调用�?
 */
void ota_service_start(void) {
    ESP_LOGI(TAG, "initialize OTA service...");
    xTaskCreate(&ota_task, "ota_task", 8192, NULL, 5, NULL);
}
