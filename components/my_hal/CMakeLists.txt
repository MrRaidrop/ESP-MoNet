idf_component_register(
    SRCS
        src/adc_hal.c
        src/wifi_hal.c
        src/uart_hal.c
    INCLUDE_DIRS "include"
    REQUIRES
        driver
        esp_wifi
        esp_netif
        esp_event
        nvs_flash
    PRIV_REQUIRES
        utils
)
