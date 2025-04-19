idf_component_register(
    SRCS 
        main.c
        bsp/wifi_manager.c
        bsp/light_sensor_driver.c
        bsp/uart_handler.c
        utils/json_utils.c
        utils/ble_format_utils.c
        net/https_post.c
        service/data_reporter.c
        service/wifi_service.c
        service/light_sensor_service.c
        service/uart_service.c
        service/ble_service.c
        OTA/https_ota_service.c
        
    INCLUDE_DIRS "." "net" "bsp" "service" "utils" "OTA" "include"

    REQUIRES
        esp_https_ota
        app_update
        esp_http_client 
        esp-tls
        esp_wifi
        esp_netif
        nvs_flash
        esp_event
        lwip
        driver
        spi_flash
        json
        bt

# 添加 DEBUG_LOG 宏，只在 Debug 模式启用
#target_compile_definitions(${COMPONENT_LIB} PRIVATE
#    $<$<CONFIG:Debug>:DEBUG_LOG>
)
