/**
 * @file dht22_hal.h
 * @brief HAL interface for DHT22 temperature and humidity sensor.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"

/**
 * @brief Initialize the DHT22 hardware.
 * 
 * @return esp_err_t ESP_OK on success, ESP_FAIL on error
 */
esp_err_t dht22_hal_init(void);

/**
 * @brief Read temperature and humidity values from DHT22.
 * 
 * @param[out] temperature_c Output: Temperature in Celsius
 * @param[out] humidity_pct  Output: Relative humidity in percent
 * @return esp_err_t ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t dht22_hal_read(float *temperature_c, float *humidity_pct);

#ifdef __cplusplus
}
#endif