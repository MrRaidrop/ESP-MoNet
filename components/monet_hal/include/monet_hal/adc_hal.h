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
 * @param chan_cfg ADC1 channel configuration.
 *                 This should be a value from `adc1_channel_t`.
 */
void adc_hal_init(int chan_cfg);

/**
 * @brief Read raw ADC value from the given channel.
 *
 * @param chan_cfg ADC1 channel to read.
 * @return Raw ADC value (0~4095).
 */
int adc_hal_read(int chan_cfg);

#ifdef __cplusplus
}
#endif

#endif /* ADC_HAL_H_ */
