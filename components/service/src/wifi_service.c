#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "my_hal/wifi_hal.h"
#include "service/wifi_service.h"



static EventGroupHandle_t wifi_event_group = NULL;

void wifi_service_start(const char *ssid, const char *pwd)
{
    wifi_event_group = wifi_init_sta(ssid, pwd);
}

bool wifi_service_is_connected(void)
{
    EventBits_t bits = xEventGroupGetBits(wifi_event_group);
    return (bits & WIFI_CONNECTED_BIT) != 0;
}

bool wifi_service_wait_connected(uint32_t timeout_ms)
{
    EventBits_t bits = xEventGroupWaitBits(
        wifi_event_group,
        WIFI_CONNECTED_BIT,
        pdFALSE,
        pdTRUE,
        pdMS_TO_TICKS(timeout_ms)
    );
    return bits & WIFI_CONNECTED_BIT;
}

bool wifi_service_start_and_wait(const char *ssid, const char *pwd, uint32_t timeout_ms) {
    wifi_service_start(ssid, pwd);
    return wifi_service_wait_connected(timeout_ms);
}

