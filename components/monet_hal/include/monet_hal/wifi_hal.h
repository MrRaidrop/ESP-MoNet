#ifndef WIFI_HAL_H_
#define WIFI_HAL_H_

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WIFI_CONNECTED_BIT BIT0

/**
 * @brief Initialize Wi-Fi as station and return EventGroup for sync.
 *
 * @param ssid SSID to connect.
 * @param password Password.
 * @return EventGroupHandle_t to track connection state (via WIFI_CONNECTED_BIT).
 */
EventGroupHandle_t wifi_init_sta(const char* ssid, const char* password);

/**
 * @brief Get the current Wi-Fi RSSI (signal strength) in dBm.
 *
 * This function wraps esp_wifi_sta_get_ap_info() and returns the RSSI
 * of the currently connected AP. It can be used to adapt system behavior
 * based on signal quality (e.g., reduce image upload rate on weak Wi-Fi).
 *
 * @return RSSI in dBm (e.g., -50 to -100), or -100 if not connected or failed.
 */
int wifi_get_rssi(void);


#ifdef __cplusplus
}
#endif

#endif // WIFI_HAL_H_
