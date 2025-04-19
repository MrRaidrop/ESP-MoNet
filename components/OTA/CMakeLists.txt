idf_component_register(
    SRCS
        src/https_ota_service.c
    INCLUDE_DIRS "include"
    REQUIRES
        esp_https_ota
        app_update
        esp_http_client
        esp-tls
        esp_wifi
        esp_netif
        nvs_flash
    PRIV_REQUIRES
        utils
)
