#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

static const char *TAG = "wifi_scan";

void wifi_scan(void) {
    uint16_t number = 20;  // 最多扫20个
    wifi_ap_record_t ap_info[20];
    uint16_t ap_count = 0;

    // 配置为STA模式
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();

    // 扫描配置，主动模式（更快）
    wifi_scan_config_t scan_config = {
        .ssid = 0,
        .bssid = 0,
        .channel = 0, // 0 = 扫所有信道
        .show_hidden = true,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
    };
    esp_wifi_scan_start(&scan_config, true);  // true = 阻塞直到扫完

    esp_wifi_scan_get_ap_records(&number, ap_info);
    esp_wifi_scan_get_ap_num(&ap_count);
    ESP_LOGI(TAG, "扫描到 %d 个AP：", ap_count);

    for (int i = 0; i < ap_count; i++) {
        ESP_LOGI(TAG, "SSID: %s, RSSI: %d, Channel: %d, BSSID: %02x:%02x:%02x:%02x:%02x:%02x",
                 ap_info[i].ssid,
                 ap_info[i].rssi,
                 ap_info[i].primary,
                 ap_info[i].bssid[0], ap_info[i].bssid[1], ap_info[i].bssid[2],
                 ap_info[i].bssid[3], ap_info[i].bssid[4], ap_info[i].bssid[5]);
    }
}

void app_main(void) {
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();

    wifi_scan();
}
