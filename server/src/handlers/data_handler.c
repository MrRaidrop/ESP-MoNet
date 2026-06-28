#define _GNU_SOURCE
#include <openssl/ssl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "server/handlers.h"

#define DATA_RX_BUF 4096

static long parse_content_length(const char *hdr)
{
    const char *p = strcasestr(hdr, "Content-Length:");
    if (!p) return -1;
    return strtol(p + 15, NULL, 10);
}

/* 处理 ESP32 POST 上来的 JSON 数据：
 * 1. 按 Content-Length 把 body 读全（esp_http_client 把 header 和 body 分成
 *    两个 TLS record 发送，单次 SSL_read 只拿到 header，所以这里要续读）
 * 2. 打印（可替换为实际业务逻辑）
 * 3. 回 200 & {"result":"ok"}
 */
int data_handler(SSL *ssl, const http_request_t *req)
{
    long content_len = parse_content_length(req->raw_buf);

    /* body bytes already captured by the router's first SSL_read */
    size_t have = req->payload ? (req->raw_len - (req->payload - req->raw_buf)) : 0;

    if (content_len <= 0) {
        /* no Content-Length: fall back to whatever we already have */
        if (have == 0) {
            const char *bad = "HTTP/1.1 400 Bad Request\r\nContent-Length:0\r\n\r\n";
            SSL_write(ssl, bad, strlen(bad));
            return 0;
        }
        content_len = (long)have;
    }
    if (content_len > (1024 * 1024)) content_len = 1024 * 1024;  /* sane cap */

    char *body = malloc(content_len + 1);
    if (!body) return -1;
    if (have > (size_t)content_len) have = content_len;
    if (have) memcpy(body, req->payload, have);

    size_t off = have;
    while (off < (size_t)content_len) {
        size_t todo = ((content_len - off) > DATA_RX_BUF) ? DATA_RX_BUF : (content_len - off);
        int n = SSL_read(ssl, body + off, todo);
        if (n <= 0) break;
        off += n;
    }
    body[off] = '\0';

    printf("[DATA] JSON Payload (%zu bytes):\n%.200s\n", off, body);
    free(body);

    const char *ok =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Connection: close\r\n"
        "Content-Length: 15\r\n\r\n"
        "{\"result\":\"ok\"}";
    SSL_write(ssl, ok, strlen(ok));
    return 0;
}
