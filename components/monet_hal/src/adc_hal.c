// hal/src/adc_hal.c
#include "driver/adc.h"
#include "monet_hal/adc_hal.h"
#include "utils/log.h"

#define TAG "ADC_HAL"

/**
 * @brief Convert integer config value into adc1_channel_t enum
 * @param chan_cfg Value from Kconfig (expected: 0 ~ 7)
 * @return Valid adc1_channel_t, or ADC1_CHANNEL_MAX if invalid
 */
static adc1_channel_t resolve_adc_channel(int chan_cfg)
{
    switch (chan_cfg) {
        case 0: return ADC1_CHANNEL_0;
        case 1: return ADC1_CHANNEL_1;
        case 2: return ADC1_CHANNEL_2;
        case 3: return ADC1_CHANNEL_3;
        case 4: return ADC1_CHANNEL_4;
        case 5: return ADC1_CHANNEL_5;
        case 6: return ADC1_CHANNEL_6;
        case 7: return ADC1_CHANNEL_7;
        case 8: return ADC1_CHANNEL_8;
        case 9: return ADC1_CHANNEL_9;
        default:
            LOGE(TAG, "Invalid ADC channel cfg = %d (expected 0-7)", chan_cfg);
            return ADC1_CHANNEL_MAX;
    }
}


void adc_hal_init(int chan_cfg)
{

    adc1_channel_t channel = resolve_adc_channel(chan_cfg);
    if (channel >= ADC1_CHANNEL_MAX) {
        return;  // Do not continue with invalid config
    }
    // 12-bit resolution 
    // Remember, if you use WIFI, you can not use ADC0 in esp32
    adc1_config_width(ADC_WIDTH_BIT_12);

    // Configure attenuation (0dB = 1.1V max input)
    adc1_config_channel_atten(channel, ADC_ATTEN_DB_0);

    LOGI(TAG, "ADC channel %d initialized", channel);
}

int adc_hal_read(int chan_cfg)
{
    adc1_channel_t channel = resolve_adc_channel(chan_cfg);
    if (channel >= ADC1_CHANNEL_MAX) {
        return -1;
    }
    int raw = adc1_get_raw(channel);
    LOGI(TAG, "ADC raw value: %d", raw);
    return raw;
}

