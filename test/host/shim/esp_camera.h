// SPDX-License-Identifier: MIT
// Host shim: a minimal camera_fb_t and a counting esp_camera_fb_return() so the
// CameraFrame RAII/move tests can assert the buffer is returned exactly once.
// The counter and function are DEFINED in the test translation unit.
#ifndef SHIM_ESP_CAMERA_H
#define SHIM_ESP_CAMERA_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t* buf;
    size_t   len;
    size_t   width;
    size_t   height;
} camera_fb_t;

// Number of times the driver was asked to reclaim a frame buffer.
extern int g_fb_return_count;

void esp_camera_fb_return(camera_fb_t* fb);

#endif  // SHIM_ESP_CAMERA_H
