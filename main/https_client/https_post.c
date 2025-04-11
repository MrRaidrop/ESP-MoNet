// https_post.c
#include <string.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "https_post.h"

#define TAG "https_post"

void http_post_json(const char *json)
{
    esp_http_client_config_t config = {
        .url = "https://40.233.83.32:8443/data",
        .method = HTTP_METHOD_POST,
        .cert_pem = NULL,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
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
