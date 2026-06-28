idf_component_register(
    SRCS
        src/https_post_hal.c
    INCLUDE_DIRS "include"
    REQUIRES
        esp_http_client
        esp-tls
        esp_wifi
        esp_netif
        nvs_flash
    PRIV_REQUIRES
        utils               # every component should include utils for config or sth
    EMBED_TXTFILES
        certs/server_ca.pem # self-signed server cert used as trusted CA (regen via server/make-certs.sh)
)
