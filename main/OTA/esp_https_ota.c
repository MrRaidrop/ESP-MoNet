#include "ota_service.h"
#include "esp_log.h"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define OTA_URL "https://40.233.83.32:8443/firmware.bin"
#define TAG "OTA_SERVICE"

/**
 * @brief OTA任务（独立线程中运行）
 */
static void ota_task(void *param) {
    ESP_LOGI(TAG, "启动 OTA 更新任务...");

    esp_http_client_config_t config = {
        .url = OTA_URL,
        .cert_pem = NULL,  // ⚠️ 自签名证书时禁用验证，等同 curl -k，测试环境专用
        .timeout_ms = 10000,
    };

    esp_err_t ret = esp_https_ota(&config);

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "OTA 更新成功，重启以应用更新...");
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_restart();
    } else {
        ESP_LOGE(TAG, "OTA 更新失败: %s", esp_err_to_name(ret));
    }

    vTaskDelete(NULL);  // 销毁任务
}

/**
 * @brief 启动 OTA 服务（建议从主程序中调用）
 */
void ota_service_start(void) {
    ESP_LOGI(TAG, "初始化 OTA 服务...");
    xTaskCreate(&ota_task, "ota_task", 8192, NULL, 5, NULL);
}
