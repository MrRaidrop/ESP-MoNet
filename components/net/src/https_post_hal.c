#include "net/https_post_hal.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "utils/config.h"
#include "utils/log.h"

#define TAG "HTTP_POST"

bool http_post_send(const char* json_str)
{
    esp_http_client_config_t config = {
        .url = CONFIG_HTTPS_SERVER_URL,
        .cert_pem = CONFIG_HTTPS_CA_CERT,
        .method = HTTP_METHOD_POST,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .skip_cert_common_name_check = CONFIG_HTTPS_SKIP_COMMON_NAME_CHECK,
        .use_global_ca_store = CONFIG_HTTPS_USE_GLOBAL_CA_STORE,
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        LOGE(TAG, "Failed to init HTTP client");
        return false;
    }

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json_str, strlen(json_str));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        LOGI(TAG, "POST succeeded, status = %d", esp_http_client_get_status_code(client));
        LOGI(TAG, "Sending POST to %s", CONFIG_HTTPS_SERVER_URL);
        LOGW(TAG, "Payload: %s", json_str);
    } else {
        LOGE(TAG, "POST failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
    return (err == ESP_OK);
}
