// https_post.c
#include <string.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "net/https_post.h"

#define TAG "https_post"

void http_post_json(const char *json)
{
    esp_http_client_config_t config = {
        .url = "https://40.233.83.32:8443/data",
        .method = HTTP_METHOD_POST,
        .cert_pem = NULL,  // 不传入 CA 证书
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .skip_cert_common_name_check = true,  //  跳过 CN 校验
        .use_global_ca_store = false,         //  不使用全局 CA
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Connection", "close");
    esp_http_client_set_post_field(client, json, strlen(json));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "POST OK, status=%d", esp_http_client_get_status_code(client));
    } else {
        ESP_LOGE(TAG, "POST FAILED: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
}



