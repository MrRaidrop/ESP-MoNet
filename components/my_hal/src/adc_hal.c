// hal/src/adc_hal.c
#include "my_hal/adc_hal.h"
#include "driver/adc.h"
#include "esp_log.h"
#include "utils/log.h"

#define TAG "ADC_HAL"

void adc_hal_init(adc1_channel_t channel)
{
    // 12-bit resolution 
    // Remember, if you use WIFI, you can not use ADC0 in esp32
    adc1_config_width(ADC_WIDTH_BIT_12);

    // Configure attenuation (0dB = 1.1V max input)
    adc1_config_channel_atten(channel, ADC_ATTEN_DB_0);

    LOGI(TAG, "ADC channel %d initialized", channel);
}

int adc_hal_read(adc1_channel_t channel)
{
    int raw = adc1_get_raw(channel);
    LOGI(TAG, "ADC raw value: %d", raw);
    return raw;
}
