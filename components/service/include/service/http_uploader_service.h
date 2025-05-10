/**
 * @file http_uploader_service.h
 * @brief HTTP uploader service definition for modular ESP32 framework.
 *
 * This service automatically subscribes to all `EVENT_GROUP_SENSOR` topics.
 * It uploads JPEG frames via `http_post_image()` and JSON payloads via `http_post_send()`.
 * If Wi-Fi is not available, data will be pushed to the cache for later retry.
 */

 #ifndef HTTP_UPLOADER_SERVICE_H_
 #define HTTP_UPLOADER_SERVICE_H_
 
 #include <stdbool.h>
 #include "monet_core/service_registry.h"
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 /**
  * @brief Get the descriptor for the HTTP uploader service.
  *
  * This descriptor will be used by `service_registry_register()` to register the service.
  * It listens to all sensor topics (`EVENT_GROUP_SENSOR`) and forwards them to HTTP endpoints.
  * - JPEG images: via `http_post_image()`
  * - JSON strings: via `http_post_send()`
  * - On failure: push to PSRAM cache and retry on next delivery
  *
  * @return Pointer to a statically defined `service_desc_t` structure.
  */
 const service_desc_t* get_http_uploader_service(void);
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif  // HTTP_UPLOADER_SERVICE_H_
 