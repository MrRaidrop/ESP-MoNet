#include <openssl/ssl.h>
#include <string.h>
#include <stdio.h>
#include "server/handlers.h"

/* 处理 ESP32 POST 上来的 JSON 数据：
 * 1. 打印（可替换为实际业务逻辑）
 * 2. 回 200 & {"result":"ok"}
 */
int data_handler(SSL *ssl, const http_request_t *req)
{
    if (!req->payload || req->payload[0] == '\0') {
        const char *bad = "HTTP/1.1 400 Bad Request\r\nContent-Length:0\r\n\r\n";
        SSL_write(ssl, bad, strlen(bad));
        return 0;
    }

    printf("[DATA] JSON Payload (max 200 chars):\n%.200s\n", req->payload);

    const char *ok =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Connection: close\r\n"
        "Content-Length: 15\r\n\r\n"
        "{\"result\":\"ok\"}";
    SSL_write(ssl, ok, strlen(ok));
    return 0;
}
