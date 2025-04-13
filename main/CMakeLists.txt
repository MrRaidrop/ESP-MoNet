idf_component_register(
    SRCS 
        main.c
        wifi_manager.c
        service/data_reporter.c
        utils/json_utils.c
        service/light_sensor_service.c
        bsp/light_sensor_driver.c
        service/uart_handler.c
        https_client/https_post.c
        service/ble_service.c
        
    INCLUDE_DIRS "." "https_client" "bsp" "service" "utils"

    REQUIRES
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
