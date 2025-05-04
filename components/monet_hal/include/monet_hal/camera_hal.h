// components/hal/include/hal/camera_hal.h
#ifndef CAMERA_HAL_H_
#define CAMERA_HAL_H_

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "esp_camera.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file camera_hal.h
 * @brief Hardware abstraction for OV2640 camera capture (JPEG frames).
 */

/**
 * @brief Initialize the OV2640 camera using config from config.h.
 *
 * This wraps esp_camera_init() with predefined GPIO and format settings.
 * @return ESP_OK on success, or camera init error
 */
esp_err_t camera_hal_init(void);

/**
 * @brief Capture a single JPEG frame from the camera.
 *
 * Returned buffer is owned by esp_camera and must be released using
 * esp_camera_fb_return().
 *
 * @return Pointer to captured frame buffer (or NULL on failure)
 */
camera_fb_t *camera_hal_capture(void);

/**
 * @brief camera configuration
 * 
 * I have think of it, and I decided to put the camera configuration here not in the config.h file.
 * This is because the camera configuration is not a general configuration, it is a specific configuration for the camera.
 */
// Pin mapping for Freenove ESP32‑S3 WROOM DevKit w/ OV2640
#define CAM_PIN_PWDN   -1  // Power down is not used
#define CAM_PIN_RESET  -1  // Software reset
#define CAM_PIN_XCLK   15
#define CAM_PIN_SIOD   4
#define CAM_PIN_SIOC   5
#define CAM_PIN_D7     16
#define CAM_PIN_D6     17
#define CAM_PIN_D5     18
#define CAM_PIN_D4     12
#define CAM_PIN_D3     10
#define CAM_PIN_D2     8
#define CAM_PIN_D1     9
#define CAM_PIN_D0     11
#define CAM_PIN_VSYNC  6
#define CAM_PIN_HREF   7
#define CAM_PIN_PCLK   13

// JPEG QVGA (~8�?5 KB) is small enough for 5 s cadence
#define CAM_FRAME_SIZE   FRAMESIZE_QVGA
#define CAM_JPEG_QUALITY 12
#define CAM_FB_COUNT     2
#define CAM_XCLK_FREQ_HZ 20 * 1000 * 1000  // 20 MHz
#define CAM_I2C_PORT     1         // I2C port for camera (default is 0)


#ifdef __cplusplus
}
#endif

#endif // CAMERA_HAL_H_
