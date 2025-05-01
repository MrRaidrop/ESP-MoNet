idf_component_register(
    SRCS
        src/adc_hal.c
        src/wifi_hal.c
    INCLUDE_DIRS "include"
    REQUIRES                
        driver              # GPIO / ADC / UART driver
        esp_wifi
        esp_netif
        nvs_flash
        esp_event
    PRIV_REQUIRES           # inner
        utils               # for JSON utils
)
