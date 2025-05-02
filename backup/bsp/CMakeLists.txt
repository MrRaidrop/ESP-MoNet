idf_component_register(
    SRCS
        src/light_sensor_driver.c
        src/uart_handler.c
        src/wifi_manager.c
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
