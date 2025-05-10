/**
 * @file data_uploader_service.h
 * @brief Services for uploading sensor data (JSON and JPEG) to cloud via HTTP/BLE.
 *
 * This module defines two independent uploader services:
 * - **Light Uploader**: Converts `EVENT_SENSOR_LIGHT` to JSON and uploads via Wi-Fi or BLE.
 * - **JPEG Uploader**: Sends raw JPEG frames (`EVENT_SENSOR_JPEG`) over Wi-Fi.
 *
 * Both services can be registered via `service_registry` or manually started using legacy entry.
 */

 #pragma once

 #include "monet_core/service_registry.h"
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 /**
  * @brief Start the data uploader services (legacy entry point).
  *
  * This function manually starts light and/or JPEG upload tasks
  * depending on the compile-time config macros:
  * - `CONFIG_UPLOAD_LIGHT_ENABLED`
  * - `CONFIG_UPLOAD_JPEG_ENABLED`
  *
  * @note In newer projects, prefer using `service_registry_register(get_..._uploader_service())`.
  */
 void data_uploader_service_start(void);
 
 /**
  * @brief Get the service descriptor for the JPEG uploader.
  *
  * This service listens for `EVENT_SENSOR_JPEG` messages from the message bus,
  * and uploads JPEG frames via HTTP POST. If the upload fails, the frame is cached
  * for later retry, and the camera buffer is returned via `esp_camera_fb_return()`.
  *
  * @return Pointer to statically-defined JPEG service descriptor.
  */
 const service_desc_t* get_jpeg_uploader_service(void);
 
 /**
  * @brief Get the service descriptor for the Light uploader.
  *
  * This service listens for `EVENT_SENSOR_LIGHT` messages,
  * converts them to JSON format, and attempts to upload via:
  * 1. HTTP POST (if Wi-Fi is available), or
  * 2. BLE notify (if BLE is connected).
  *
  * On failure, messages are cached for retry.
  *
  * @return Pointer to statically-defined Light service descriptor.
  */
 const service_desc_t* get_light_uploader_service(void);
 
 #ifdef __cplusplus
 }
 #endif
 