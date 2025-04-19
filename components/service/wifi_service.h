#ifndef WIFI_SERVICE_H
#define WIFI_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Start Wi-Fi as station using provided SSID and password.
 *        This wraps the underlying Wi-Fi manager (e.g., wifi_init_sta).
 *
 * @param ssid Wi-Fi SSID
 * @param password Wi-Fi password
 */
void wifi_service_start(const char *ssid, const char *password);

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
#ifdef __cplusplus
}
#endif

#endif // WIFI_SERVICE_H
