#include "monet_hal/dht22_hal.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define TAG "DHT22_HAL"

esp_err_t dht22_hal_init(void)
{
    // TODO: Configure GPIO and timing
    ESP_LOGI(TAG, "DHT22 HAL init (stub)");
    return ESP_OK;
}

esp_err_t dht22_hal_read(float *temperature_c, float *humidity_pct)
{
    // TODO: Implement actual DHT22 timing + bit decode
    *temperature_c = 25.0f;
    *humidity_pct = 50.0f;
    ESP_LOGW(TAG, "DHT22 read (stub values returned)");
    return ESP_OK;
}
