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

#ifdef __cplusplus
}
#endif

#endif // WIFI_HAL_H_
