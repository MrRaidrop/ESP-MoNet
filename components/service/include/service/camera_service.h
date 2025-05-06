// components/service/include/service/camera_service.h
#ifndef CAMERA_SERVICE_H_
#define CAMERA_SERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "monet_core/service_registry.h"

/**
 * @file camera_service.h
 * @brief Periodic JPEG capture service using camera_hal.
 *
 * This service captures a JPEG image from the camera every few seconds
 * and publishes it to the internal message bus as EVENT_SENSOR_JPEG.
 *
 * The JPEG data is stored in `msg_t.data.jpeg_buf`, and the length
 * in `msg_t.data.jpeg_len`. Subscribers (e.g., HTTP uploader, UART service)
 * can process the image payload directly.
 */

/**
 * @brief Get the service descriptor for the camera service.
 *
 * This function returns a pointer to a statically defined `service_desc_t` structure
 * that describes the camera service. The descriptor contains the task function,
 * task name, stack size, and priority required by the service registry to create and manage the task.
 *
 * This function is typically called in `main.c` during system initialization to register
 * the camera service via `service_registry_register()`.
 *
 * @return Pointer to a `service_desc_t` describing the camera service.
 */
const service_desc_t* get_camera_service(void);

#ifdef __cplusplus
}
#endif

#endif // CAMERA_SERVICE_H_
