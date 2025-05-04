// components/service/include/service/camera_service.h
#ifndef CAMERA_SERVICE_H_
#define CAMERA_SERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

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
 * @brief Start the camera service task.
 *
 * Initializes the camera using `camera_hal_init()` and starts a FreeRTOS
 * task to capture JPEG frames at regular intervals defined in config.h file.
 * Captured frames are published to the `msg_bus` with `EVENT_SENSOR_JPEG` topic.
 */
void camera_service_start(void);

#ifdef __cplusplus
}
#endif

#endif // CAMERA_SERVICE_H_
