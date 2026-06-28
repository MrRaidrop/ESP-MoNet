// SPDX-License-Identifier: MIT
//
// logger_demo.cpp — wires the LoggerService + TaskHealthMonitor into the live
// system so the abstractions actually run in the message flow (not just a
// one-shot demo). Gated by CONFIG_MONET_CPP_EXPERIMENTAL via the build.
//
// What boots here:
//   * the async LoggerService drain task (producers log via MLOG*, never block)
//   * a heartbeat task that logs and feeds BOTH the soft monitor and the
//     hardware Task Watchdog every second
//   * a worker task that logs and checks in with the soft monitor
//   * a supervisor task that periodically runs health.check()
//
// With CONFIG_MONET_CPP_DEMO_HANG the worker stops checking in after a few
// loops; the supervisor then reports it as having missed its deadline.
//
#include "monet_core_cpp/cpp_demo.h"

#include "esp_http_server.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "monet_core_cpp/LoggerService.hpp"
#include "monet_core_cpp/TaskHealthMonitor.hpp"
#include "monet_core_cpp/WebLoggerBackend.hpp"

namespace {

constexpr const char* kTag = "LOGGER_DEMO";

// Web log sink. Swapped in as the LoggerService backend (it also echoes to the
// console), and served to the browser by a small HTTP server on port 8080.
monet::WebLoggerBackend g_web_log;

esp_err_t log_index_handler(httpd_req_t* req) {
    static const char* kPage =
        "<!doctype html><html><head><meta charset=utf-8><title>ESP-MoNet logs</title>"
        "<style>body{background:#0d0d0f;color:#cfcfd4;font:13px/1.45 ui-monospace,monospace;"
        "margin:0;padding:12px}h1{font-size:14px;color:#8ab4f8;margin:0 0 10px}"
        "pre{white-space:pre-wrap;word-break:break-word;margin:0}</style></head>"
        "<body><h1>ESP-MoNet — live logs</h1><pre id=log>loading…</pre>"
        "<script>async function t(){try{const r=await fetch('/log.txt?'+Date.now());"
        "const x=await r.text();const atBottom=window.innerHeight+window.scrollY>="
        "document.body.scrollHeight-40;document.getElementById('log').textContent=x;"
        "if(atBottom)window.scrollTo(0,document.body.scrollHeight);}catch(e){}}"
        "setInterval(t,1000);t();</script></body></html>";
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, kPage, HTTPD_RESP_USE_STRLEN);
}

esp_err_t log_text_handler(httpd_req_t* req) {
    static char buf[monet::WebLoggerBackend::kCap + 1];  // tracks the backend
    size_t n = g_web_log.snapshot(buf, sizeof(buf));
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    return httpd_resp_send(req, buf, n);
}

// Start a tiny HTTP server for the log view. Separate port + ctrl_port from the
// camera's httpd (80 / 32768) so the two servers coexist.
//
// Altitude note: in production the HTTP-serving half of this would live as a
// service in components/service/ (next to mjpeg_service, which already owns an
// httpd), reaching WebLoggerBackend through a small C API — leaving monet_core
// free of esp_http_server. It lives here because this whole file is the gated
// experimental demo (CONFIG_MONET_CPP_EXPERIMENTAL), kept self-contained.
void start_log_http() {
    httpd_handle_t server = nullptr;
    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
    cfg.server_port = 8080;
    cfg.ctrl_port = 32769;
    cfg.lru_purge_enable = true;
    if (httpd_start(&server, &cfg) != ESP_OK) {
        ESP_LOGE(kTag, "log http server failed to start");
        return;
    }
    httpd_uri_t idx = {.uri = "/",        .method = HTTP_GET, .handler = log_index_handler};
    httpd_uri_t txt = {.uri = "/log.txt", .method = HTTP_GET, .handler = log_text_handler};
    httpd_register_uri_handler(server, &idx);
    httpd_register_uri_handler(server, &txt);
    ESP_LOGI(kTag, "log web view: http://<board-ip>:8080/");
}

// Shared monitor. Registration of the worker happens once at init (single
// thread); the heartbeat self-registers (only one task does, so no race).
monet::TaskHealthMonitor g_health(nullptr, monet::FailSafe::Log);
int g_worker_id = -1;

// Heartbeat: opts into the hardware TWDT and feeds it every second.
void heartbeat_task(void*) {
    int id = g_health.register_task("heartbeat", /*deadline_ms=*/3000,
                                    /*use_wdt=*/true);
    for (;;) {
        MLOGI("HEARTBEAT", "tick t=%lums",
              static_cast<unsigned long>(esp_log_timestamp()));
        g_health.kick(id);  // also resets the TWDT for this task
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Worker: soft-monitored only. Under DEMO_HANG it deliberately wedges.
void worker_task(void*) {
    int loops = 0;
    for (;;) {
        MLOGI("WORKER", "working loop=%d", loops);
        g_health.kick(g_worker_id);
        ++loops;
#if CONFIG_MONET_CPP_DEMO_HANG
        if (loops >= 3) {
            MLOGW("WORKER", "simulating hang: will stop checking in");
            for (;;) {
                vTaskDelay(pdMS_TO_TICKS(1000));  // never kicks again
            }
        }
#endif
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Supervisor: periodically evaluates deadlines and reports offenders.
void supervisor_task(void*) {
    for (;;) {
        g_health.check();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

}  // namespace

extern "C" void monet_cpp_logger_demo_start(void) {
    // Route logs to the web backend (interface swap at runtime — call sites
    // unchanged); it also echoes to the console. Then start the drain task.
    monet::default_logger().set_backend(g_web_log);
    monet::default_logger().start();
    start_log_http();  // needs the network up — call this after Wi-Fi
    MLOGI(kTag, "async LoggerService started (logger task priority %u)",
          static_cast<unsigned>(monet::kLogTaskPriority));

    // Register the worker up front (single-threaded init avoids a table race).
    g_worker_id = g_health.register_task("worker", /*deadline_ms=*/3000,
                                         /*use_wdt=*/false);

    xTaskCreate(heartbeat_task, "hb_task", 3072, nullptr, 3, nullptr);
    xTaskCreate(worker_task, "worker_task", 3072, nullptr, 3, nullptr);
    xTaskCreate(supervisor_task, "health_sup", 2048, nullptr, 4, nullptr);  // check() only, no formatting
}
