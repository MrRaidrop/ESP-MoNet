idf_component_register(
    SRCS
        src/data_reporter.c
        src/wifi_service.c
        src/light_sensor_service.c
        src/uart_service.c
        src/ble_service.c
    INCLUDE_DIRS "include"
    REQUIRES
        bsp                 
        net
        OTA
        utils
        esp_event           # FreeRTOS event lope
        bt                  # esp‑idf BLE stack  :wq
)
