idf_component_register(
    SRCS
        src/adc_hal.c
        src/wifi_hal.c
        src/uart_hal.c
        src/camera_hal.c
    INCLUDE_DIRS "include"
    REQUIRES
        esp32-camera
        esp_timer
        driver
        esp_wifi
        esp_netif
        esp_event
        nvs_flash
    PRIV_REQUIRES
        utils
)
