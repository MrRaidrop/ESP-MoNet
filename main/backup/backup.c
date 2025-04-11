#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "esp_http_client.h"
#include "driver/uart.h" 

#define WIFI_SSID      "Ambre"
#define WIFI_PASS      "12345678"
#define MAX_RETRY      100


static const char *TAG = "HTTPS_POST";
static int retry_num = 0;
static int post_counter = 0;
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0


// uartlog
#define UART_BUF_SIZE 128
static QueueHandle_t uart_queue;


void uart_rx_task(void *pvParameters)
{
    uint8_t data[UART_BUF_SIZE];
    while (1) {
        int len = uart_read_bytes(UART_NUM_0, data, UART_BUF_SIZE - 1, pdMS_TO_TICKS(1000));
        if (len > 0) {
            data[len] = '\0';
            ESP_LOGI("UART", "Received: %s", data);

            // 发到 queue（注意 copy）
            char *copy = strdup((char *)data);
            if (copy) {
                xQueueSend(uart_queue, &copy, portMAX_DELAY);
            }
        }
    }
}


// ===== 连接事件处理器 =====
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (retry_num < MAX_RETRY) {
            esp_wifi_connect();
            retry_num++;
            ESP_LOGI(TAG, "Retrying WiFi...");
        } else {
            xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
            ESP_LOGE(TAG, "Failed to connect to Wi-Fi after %d retries.", MAX_RETRY);
        }
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// ===== 初始化 WiFi =====
static void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);
    
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
    
    ESP_LOGI(TAG, "WiFi init finished");
}
// --- build json---
char *build_json_payload(const char *uart_data, int *counter)
{
    static char json_buf[256];

    time_t now;
    time(&now);
    struct tm *timeinfo = localtime(&now);

    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);

    // 拼 JSON 字符串：{"esp32": "2025-04-07 16:58:30", "uart_data": "xxx", "hello": 12}
    snprintf(json_buf, sizeof(json_buf),
             "{\"esp32\": \"%s\", \"uart_data\": \"%s\", \"hello\": %d}",
             time_str, uart_data, *counter);
    (*counter)++;

    return json_buf;
}


void http_post_json(const char *json)
{
    esp_http_client_config_t config = {
        .url = "https://40.233.83.32:8443/data",
        .method = HTTP_METHOD_POST,
        .cert_pem = NULL,  // 跳过证书校验
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .timeout_ms = 5000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Connection", "close");
    esp_http_client_set_post_field(client, json, strlen(json));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        printf("POST OK, status=%d\n", esp_http_client_get_status_code(client));
    } else {
        printf("POST FAILED: %s\n", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
}

// --schdule tasks--
static void post_task(void *pvParameters)
{
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT,
                                           pdFALSE, pdTRUE, portMAX_DELAY);
    if (!(bits & WIFI_CONNECTED_BIT)) {
        printf("WiFi not connected\n");
        vTaskDelete(NULL);
    }

    static char latest_data[128] = "default";  // 默认要发送的内容
    const TickType_t delay_ticks = pdMS_TO_TICKS(5000);

    while (1) {
        vTaskDelay(delay_ticks);

        // ⏬ 每次都发串口数据（向 PC）
        uart_write_bytes(UART_NUM_0, (const char *)latest_data, strlen(latest_data));
        uart_write_bytes(UART_NUM_0, "\r\n", 2);  // 换行可读

        // ⏬ 每次检查串口有没有新数据（非阻塞）
        char *uart_data = NULL;
        if (xQueueReceive(uart_queue, &uart_data, 0)) {
            strncpy(latest_data, uart_data, sizeof(latest_data) - 1);
            latest_data[sizeof(latest_data) - 1] = '\0';  // 确保结束符
            free(uart_data);  // 别忘释放
        }

        // ⏬ POST 当前要发的数据
        char *json = build_json_payload(latest_data, &post_counter);
        http_post_json(json);
    }
}


void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_sta();

    // 初始化 UART0（CDC）
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_driver_install(UART_NUM_0, 1024 * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_0, &uart_config);

    // 创建 queue
    uart_queue = xQueueCreate(5, sizeof(char *));
    
    // 启动任务
    xTaskCreate(uart_rx_task, "uart_rx_task", 4096, NULL, 10, NULL);
    xTaskCreate(post_task, "post_task", 8192, NULL, 5, NULL);
}
