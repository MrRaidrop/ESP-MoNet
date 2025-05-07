#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "monet_core/service_registry.h"
#include "monet_hal/wifi_hal.h"
#include "service/wifi_service.h"
#include "utils/log.h"

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

bool wifi_service_start_and_wait(const char *ssid, const char *pwd, uint32_t timeout_ms)
{
    wifi_service_start(ssid, pwd);
    return wifi_service_wait_connected(timeout_ms);
}

/**
 * @brief Task entry function for Wi-Fi service
 */
static void wifi_service_task(void *arg)
{
    wifi_service_start(CONFIG_WIFI_SSID, CONFIG_WIFI_PASSWORD);

    LOGI("WIFI_SERVICE", "Waiting for Wi-Fi...");
    if (wifi_service_wait_connected(10000)) {
        LOGI("WIFI_SERVICE", "Wi-Fi connected");
    } else {
        LOGW("WIFI_SERVICE", "Wi-Fi timeout");
    }

    vTaskDelete(NULL);  // Optional: one-shot init task
}

/**
 * @brief Wi-Fi service descriptor used by service_registry
 */
const service_desc_t wifi_service_desc = {
    .name       = "wifi_service",
    .task_fn    = wifi_service_task,
    .task_name  = "wifi_service_task",
    .stack_size = 2048,
    .priority   = 4,
    .role       = SERVICE_ROLE_NONE,
    .topics     = NULL  // Not subscribing to any topic
};

/**
 * @brief Accessor for Wi-Fi service descriptor (fallback registration)
 */
const service_desc_t* get_wifi_service(void)
{
    return &wifi_service_desc;
}
