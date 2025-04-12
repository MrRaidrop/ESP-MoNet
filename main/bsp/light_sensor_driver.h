#pragma once

#include <stdint.h>
#include <driver/adc.h>

#ifdef __cplusplus
extern "C" {
#endif

// 驱动初始化：指定 GPIO 对应的 ADC 通道
void light_sensor_driver_init(adc1_channel_t channel);

// 读取 ADC 原始值
int light_sensor_driver_read_raw(void);

#ifdef __cplusplus
}
#endif