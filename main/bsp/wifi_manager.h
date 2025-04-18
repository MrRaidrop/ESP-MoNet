#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WIFI_CONNECTED_BIT BIT0 // 其他模块要用这个判断wifi是否连接

/**
 * @brief 初始化 Wi-Fi，并开始连接
 *
 * @param ssid Wi-Fi SSID
 * @param password Wi-Fi 密码
 * @return EventGroupHandle_t 事件组句柄，外部可以通过此句柄监听 WIFI_CONNECTED_BIT
 */
EventGroupHandle_t wifi_init_sta(const char *ssid, const char *password);

#ifdef __cplusplus
}
#endif

#endif // WIFI_MANAGER_H
