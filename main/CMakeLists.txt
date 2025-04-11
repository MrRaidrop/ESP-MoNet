idf_component_register(
    SRCS 
        main.c
        wifi_manager.c
        uart_handler.c
        json_utils.c
        https_client/https_post.c
        
    INCLUDE_DIRS "."
                 "https_client"

    PRIV_REQUIRES
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

# 添加 DEBUG_LOG 宏，只在 Debug 模式启用
#target_compile_definitions(${COMPONENT_LIB} PRIVATE
#    $<$<CONFIG:Debug>:DEBUG_LOG>
)
