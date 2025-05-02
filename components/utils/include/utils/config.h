/* config/include/config.h */
#ifndef CONFIG_H_
#define CONFIG_H_

/**
 * @brief Light sensor channel, GPIO3
 */
#define CONFIG_LIGHT_SENSOR_CHANNEL  ADC1_CHANNEL_2

/**
 * @brief Cache size and retry period for upload
 */
#define CONFIG_UPLOAD_CACHE_SIZE    20
#define CONFIG_UPLOAD_CACHE_ITEM_SIZE 128
#define CONFIG_UPLOAD_RETRY_PERIOD_MS 2000

/**
 * @brief Wi-Fi configuration
 */
#define CONFIG_WIFI_SSID      "zhenghao的iPhone"
#define CONFIG_WIFI_PASSWORD  "12345678"

/**
 * @brief Server configuration
 */
#define CONFIG_HTTPS_SERVER_URL   "https://40.233.83.32:8443/data"
#define CONFIG_HTTPS_CA_CERT       NULL  // Replace with real cert if needed

/**
 * @brief HTTPS client behavior
 */
#define CONFIG_HTTPS_SKIP_COMMON_NAME_CHECK   1  // Skip CN for now
#define CONFIG_HTTPS_USE_GLOBAL_CA_STORE      0  // local CA

/**
 * @brief OTA endpoint
 */
#define CONFIG_OTA_UPDATE_URL    "https://40.233.83.32:8443/update"

/**
 * @brief Log level and switch
 */
#define CONFIG_LOG_ENABLE        1
#define CONFIG_LOG_LEVEL_DEFAULT ESP_LOG_INFO

#endif /* CONFIG_H_ */


