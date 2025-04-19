idf_component_register(
    SRCS
        src/https_post.c
    INCLUDE_DIRS "include"
    REQUIRES
        esp_http_client
        esp-tls
        esp_wifi
        esp_netif
        nvs_flash
    PRIV_REQUIRES
        utils               # for json utils
)
