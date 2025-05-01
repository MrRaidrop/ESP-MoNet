// hal/include/hal/adc_hal.h
#ifndef ADC_HAL_H_
#define ADC_HAL_H_

#include "driver/adc.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize ADC channel.
 *
 * @param channel ADC1 channel to configure.
 */
void adc_hal_init(adc1_channel_t channel);

/**
 * @brief Read raw ADC value from the given channel.
 *
 * @param channel ADC1 channel to read.
 * @return Raw ADC value (0~4095).
 */
int adc_hal_read(adc1_channel_t channel);

#ifdef __cplusplus
}
#endif

#endif /* ADC_HAL_H_ */
