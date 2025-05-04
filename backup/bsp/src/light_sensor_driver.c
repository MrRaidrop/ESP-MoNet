#include "bsp/light_sensor_driver.h"

static adc1_channel_t light_adc_channel;

void light_sensor_driver_init(adc1_channel_t channel)
{
    light_adc_channel = channel;
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(channel, ADC_ATTEN_DB_11);
}

int light_sensor_driver_read_raw(void)
{
    return adc1_get_raw(light_adc_channel);
}
