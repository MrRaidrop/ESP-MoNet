/**
 * @file mjpeg_service.c
 * @brief Live MJPEG stream over a single persistent HTTP connection.
 *
 * Classic ESP32-CAM approach: serve multipart/x-mixed-replace and push JPEG
 * frames grabbed straight from the camera driver. One connection, no per-frame
 * handshake -> smooth ~15-25 fps at QVGA on the LAN.
 */
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "service/mjpeg_service.h"
#include "monet_hal/camera_hal.h"
#include "utils/log.h"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "esp_timer.h"
#include "lwip/sockets.h"
#include <string.h>

#define TAG "MJPEG_SERVICE"

/* Target stream frame rate. The camera + WiFi can do ~65 fps QVGA once the
 * XCLK is off the WiFi channel (see camera_hal.h), but we pace to a steady,
 * lighter rate that leaves the radio plenty of headroom for other services. */
#ifndef MJPEG_TARGET_FPS
#define MJPEG_TARGET_FPS 12
#endif

#define PART_BOUNDARY "monetframe"
static const char *STREAM_CONTENT_TYPE =
    "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;

static const char *INDEX_HTML =
    "<!doctype html><html><head><meta charset=utf-8><title>ESP-MoNet cam</title>"
    "<style>html,body{margin:0;height:100%;background:#000;display:flex;"
    "align-items:center;justify-content:center}img{max-width:100%;max-height:100%;"
    "image-rendering:pixelated}</style></head>"
    "<body><img src=\"/stream\"></body></html>";

static esp_err_t index_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, INDEX_HTML, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t stream_handler(httpd_req_t *req)
{
    esp_err_t res = httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
    if (res != ESP_OK) return res;
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");

    /* Disable Nagle on this socket: frames are small and latency-sensitive,
     * so we must not wait to coalesce them with delayed ACKs. */
    int sockfd = httpd_req_to_sockfd(req);
    int one = 1;
    setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

    LOGI(TAG, "stream client connected");
    /* boundary + part header coalesced into one send to cut TCP round-trips */
    char hdr[96];
    const int64_t frame_us = 1000000 / MJPEG_TARGET_FPS;
    int64_t next = esp_timer_get_time();
    while (true) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            LOGW(TAG, "fb get failed");
            res = ESP_FAIL;
            break;
        }
        int hl = snprintf(hdr, sizeof(hdr),
                          "\r\n--" PART_BOUNDARY "\r\n"
                          "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n",
                          fb->len);

        res = httpd_resp_send_chunk(req, hdr, hl);
        if (res == ESP_OK)
            res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);

        esp_camera_fb_return(fb);
        if (res != ESP_OK) break;   /* client disconnected */

        /* pace to a steady frame rate */
        next += frame_us;
        int64_t now = esp_timer_get_time();
        if (next > now)
            vTaskDelay(pdMS_TO_TICKS((next - now) / 1000));
        else
            next = now;             /* fell behind; don't accumulate lag */
    }
    LOGI(TAG, "stream client disconnected");
    return res;
}

static void mjpeg_service_task(void *param)
{
    if (camera_hal_init() != ESP_OK) {
        LOGE(TAG, "Camera init failed - stream not started");
        vTaskDelete(NULL);
        return;
    }

    /* Disable Wi-Fi modem power-save: with it on, every TCP round-trip waits
     * for the next DTIM beacon (~100ms), throttling the stream to <1 fps. */
    esp_wifi_set_ps(WIFI_PS_NONE);

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.ctrl_port   = 32768;
    config.stack_size  = 8192;
    config.lru_purge_enable = true;

    if (httpd_start(&server, &config) != ESP_OK) {
        LOGE(TAG, "httpd_start failed");
        vTaskDelete(NULL);
        return;
    }

    httpd_uri_t index_uri  = { .uri = "/",       .method = HTTP_GET, .handler = index_handler };
    httpd_uri_t stream_uri = { .uri = "/stream", .method = HTTP_GET, .handler = stream_handler };
    httpd_register_uri_handler(server, &index_uri);
    httpd_register_uri_handler(server, &stream_uri);

    LOGI(TAG, "MJPEG stream ready: open http://<board-ip>/ in a browser");
    vTaskDelete(NULL);   /* esp_http_server runs in its own task */
}

static const service_desc_t mjpeg_service_desc = {
    .name       = "mjpeg_service",
    .task_fn    = mjpeg_service_task,
    .task_name  = "mjpeg_service_task",
    .stack_size = 4096,
    .priority   = 5,
    .role       = SERVICE_ROLE_NONE,
    .topics     = NULL,
    .sink_cb    = NULL,
};

const service_desc_t *get_mjpeg_service(void)
{
    return &mjpeg_service_desc;
}
