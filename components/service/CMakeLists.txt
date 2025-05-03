idf_component_register(
    SRCS
        src/data_reporter.c
        src/wifi_service.c
        src/light_sensor_service.c
        src/uart_service.c
        src/ble_service.c
        src/data_uploader_service.c
        src/camera_service.c
    INCLUDE_DIRS "include"
    PRIV_REQUIRES 
        bt 
        nvs_flash
    REQUIRES
        core
        my_hal                 
        net
        OTA
        utils
        esp_event           # FreeRTOS event lope
)
