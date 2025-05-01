#ifndef LIGHT_SENSOR_SERVICE_H_
#define LIGHT_SENSOR_SERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Start the light sensor service.
 *
 * This function initializes the ADC channel for the light sensor
 * and creates a FreeRTOS task to periodically read sensor values.
 */
void light_sensor_service_start(void);

/**
 * @brief Get the latest cached light sensor value.
 *
 * The value is updated every 500ms by the service task.
 *
 * @return Raw ADC value (0 ~ 4095).
 */
int light_sensor_get_cached_value(void);

#ifdef __cplusplus
}
#endif

#endif // LIGHT_SENSOR_SERVICE_H_
