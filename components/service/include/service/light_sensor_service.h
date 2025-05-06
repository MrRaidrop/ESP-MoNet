#ifndef LIGHT_SENSOR_SERVICE_H_
#define LIGHT_SENSOR_SERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "monet_core/service_registry.h"

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

/**
 * @brief Get the service descriptor for the light sensor service.
 *
 * This function returns a pointer to a statically defined `service_desc_t`
 * structure that describes the light sensor service. It includes the task 
 * entry function, task name, stack size, and priority. This descriptor can 
 * be used by `service_registry_register()` in the main application to 
 * explicitly register the service at runtime.
 *
 * @return Pointer to a `service_desc_t` structure representing the light sensor service.
 */
const service_desc_t* get_light_sensor_service(void);


#ifdef __cplusplus
}
#endif

#endif // LIGHT_SENSOR_SERVICE_H_
