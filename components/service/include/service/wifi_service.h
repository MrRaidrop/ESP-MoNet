#ifndef WIFI_SERVICE_H
#define WIFI_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "monet_core/service_registry.h" 

/**
 * @brief Start Wi-Fi as station using provided SSID and password.
 *        This wraps the underlying Wi-Fi manager (e.g., wifi_init_sta).
 *
 * @param ssid Wi-Fi SSID
 * @param password Wi-Fi password
 */
void wifi_service_start(const char *ssid, const char *password);

/**
 * @brief Check whether Wi-Fi is currently connected.
 *
 * This is a non-blocking version of wifi_service_wait_connected().
 * It reads the event group bit directly without waiting.
 *
 * @return true if connected to Wi-Fi (WIFI_CONNECTED_BIT is set)
 * @return false if not connected
 */
bool wifi_service_is_connected(void);

/**
 * @brief Block until Wi-Fi connection is established or timeout occurs.
 *
 * @return true if connected, false on failure or timeout
 */
bool wifi_service_wait_connected(uint32_t timeout_ms);
/**
 * @brief Start Wi-Fi and block until connection is established or timeout occurs.
 *
 * This is a convenience function that internally calls wifi_service_start() 
 * followed by wifi_service_wait_connected() with the specified timeout.
 * It encapsulates the full process of initiating a Wi-Fi connection and waiting
 * for it to complete, making it suitable for use in application initialization code.
 *
 * @param ssid Wi-Fi SSID
 * @param pwd Wi-Fi password
 * @param timeout_ms Timeout in milliseconds to wait for connection
 *
 * @return true if Wi-Fi connected successfully, false if timeout or failure occurred
 */
bool wifi_service_start_and_wait(const char *ssid, const char *pwd, uint32_t timeout_ms);

/**
 * @brief Retrieve the descriptor for the Wi-Fi service.
 *
 * This function returns a pointer to the statically defined
 * `service_desc_t` for the Wi-Fi service, which can be passed to
 * `service_registry_register()` or used in fallback scenarios where
 * `SERVICE_REGISTER()` is not available.
 *
 * @return const service_desc_t* Pointer to Wi-Fi service descriptor.
 */
const service_desc_t* get_wifi_service(void);

#ifdef __cplusplus
}
#endif

#endif // WIFI_SERVICE_H
