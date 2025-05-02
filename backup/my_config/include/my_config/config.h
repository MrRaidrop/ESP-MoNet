/* config/include/config.h */
#ifndef CONFIG_H_
#define CONFIG_H_

/**
 * @brief Light sensor channel, GPIO3
 */
#define CONFIG_LIGHT_SENSOR_CHANNEL  ADC1_CHANNEL_2

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
 * @brief OTA endpoint
 */
#define CONFIG_OTA_UPDATE_URL    "https://40.233.83.32:8443/update"

/**
 * @brief Log level and switch
 */
#define CONFIG_LOG_ENABLE        1
#define CONFIG_LOG_LEVEL_DEFAULT ESP_LOG_INFO

#endif /* CONFIG_H_ */


