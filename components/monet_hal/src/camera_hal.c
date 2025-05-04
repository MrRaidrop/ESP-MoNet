// components/monet_hal/src/camera_hal.c
#include "monet_hal/camera_hal.h"
#include "esp_camera.h"
#include "utils/log.h"
#include "utils/config.h"

#define TAG "CAMERA_HAL"

esp_err_t camera_hal_init(void)
{
    camera_config_t cfg = {};
    cfg.pin_pwdn       = CAM_PIN_PWDN;
    cfg.pin_reset      = CAM_PIN_RESET;
    cfg.pin_xclk       = CAM_PIN_XCLK;
    cfg.pin_sccb_sda   = CAM_PIN_SIOD;
    cfg.pin_sccb_scl   = CAM_PIN_SIOC;
    cfg.pin_d7         = CAM_PIN_D7;
    cfg.pin_d6         = CAM_PIN_D6;
    cfg.pin_d5         = CAM_PIN_D5;
    cfg.pin_d4         = CAM_PIN_D4;
    cfg.pin_d3         = CAM_PIN_D3;
    cfg.pin_d2         = CAM_PIN_D2;
    cfg.pin_d1         = CAM_PIN_D1;
    cfg.pin_d0         = CAM_PIN_D0;
    cfg.pin_vsync      = CAM_PIN_VSYNC;
    cfg.pin_href       = CAM_PIN_HREF;
    cfg.pin_pclk       = CAM_PIN_PCLK;

    cfg.xclk_freq_hz   = CAM_XCLK_FREQ_HZ;
    cfg.ledc_timer     = LEDC_TIMER_0;
    cfg.ledc_channel   = LEDC_CHANNEL_0;
    cfg.pixel_format   = PIXFORMAT_JPEG;
    cfg.frame_size     = CAM_FRAME_SIZE;
    cfg.jpeg_quality   = CAM_JPEG_QUALITY;
    cfg.fb_count       = CAM_FB_COUNT;
    cfg.fb_location    = CAMERA_FB_IN_PSRAM;
    cfg.grab_mode      = CAMERA_GRAB_WHEN_EMPTY;
    cfg.sccb_i2c_port  = CAM_I2C_PORT;

    esp_err_t err = esp_camera_init(&cfg);
    if (err != ESP_OK) {
        LOGE(TAG, "Camera init failed: 0x%x", err);
    } else {
        LOGI(TAG, "Camera initialized successfully");
    }
    return err;
}

camera_fb_t *camera_hal_capture(void)
{
    return esp_camera_fb_get();
}
