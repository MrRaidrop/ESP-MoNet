#include <stdint.h>  
#include <stddef.h> 
#include "sdkconfig.h"
#include "net/https_post_hal.h"
#include "esp_http_client.h"
#include "utils/config.h"
#include "utils/log.h"

#define TAG "HTTP_POST"

#ifndef CONFIG_HTTPS_SKIP_COMMON_NAME_CHECK
#define CONFIG_HTTPS_SKIP_COMMON_NAME_CHECK 0
#endif
#ifndef CONFIG_HTTPS_USE_GLOBAL_CA_STORE
#define CONFIG_HTTPS_USE_GLOBAL_CA_STORE 0
#endif

bool http_post_send(const char* json_str)
{
    esp_http_client_config_t config = {
        .url = CONFIG_HTTPS_SERVER_URL,
        .cert_pem = CONFIG_HTTPS_CA_CERT_PEM,
        .method = HTTP_METHOD_POST,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .skip_cert_common_name_check = CONFIG_HTTPS_SKIP_COMMON_NAME_CHECK,
        .use_global_ca_store        = CONFIG_HTTPS_USE_GLOBAL_CA_STORE,
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        LOGE(TAG, "Failed to init HTTP client");
        return false;
    }

    esp_http_client_set_header(client, "Content-Type", "application/json");
    
    esp_http_client_set_post_field(client, json_str, strlen(json_str));
    esp_http_client_set_header(client, "Connection", "close");   
    
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

#define POST_IMAGE_URL CONFIG_HTTPS_SERVER_URL

bool http_post_image(const uint8_t *data, size_t len)
{
    if (!data || len == 0) return false;

    esp_http_client_config_t config = {
        .url = POST_IMAGE_URL,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 5000,
        .cert_pem = NULL, // Add CA cert if needed
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .skip_cert_common_name_check = CONFIG_HTTPS_SKIP_COMMON_NAME_CHECK,
        .use_global_ca_store        = CONFIG_HTTPS_USE_GLOBAL_CA_STORE,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        LOGE(TAG, "Failed to init HTTP client");
        return false;
    }

    esp_http_client_set_header(client, "Content-Type", "image/jpeg");

    esp_err_t err = esp_http_client_open(client, len);
    if (err != ESP_OK) {
        LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return false;
    }

    LOGI(TAG, "Uploading %u bytes to %s", (unsigned)len, POST_IMAGE_URL);
    int written = esp_http_client_write(client, (const char *)data, len);
    esp_http_client_close(client);

    if (written != (int)len) {
        LOGW(TAG, "Mismatch in write size (%d != %d)", written, (int)len);
        esp_http_client_cleanup(client);
        return false;
    } else {
        LOGI(TAG, "HTTP write success (%d bytes)", written);
    }

    int status = esp_http_client_get_status_code(client);
    LOGI(TAG, "HTTP response code: %d", status);
    esp_http_client_cleanup(client);

    if (status >= 200 && status < 300) {
        return true;
    } else {
        LOGW(TAG, "HTTP response: %d", status);
        return false;
    }
}
