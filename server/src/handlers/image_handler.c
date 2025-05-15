#define _GNU_SOURCE
#include "server/handlers.h"
#include "server/file_store.h"
#include "server/log.h"
#include <openssl/ssl.h>

#include <string.h>
#include <stdlib.h>

#define TAG "IMG_HANDLER"
#define RX_BUF 4096

/* 只支持:  POST /image  Content-Type: image/jpeg  + Content-Length  */
static long parse_content_length(const char *hdr)
{
    const char *p = strcasestr(hdr, "Content-Length:");
    if (!p) return -1;
    return strtol(p + 15, NULL, 10);
}

int image_handler(SSL *ssl, const http_request_t *req)
{
    static int inited = 0;
    if (!inited) { file_store_init(); inited = 1; }

    long content_len = parse_content_length(req->raw_buf);
    if (content_len <= 0) {
        const char *resp = "HTTP/1.1 411 Length Required\r\n\r\n";
        SSL_write(ssl, resp, strlen(resp));
        return -1;
    }

    if ((size_t)content_len > (10*1024*1024)) {   // 10 MB 上限，OV2640 QVGA 远小于此
    const char *resp = "HTTP/1.1 413 Payload Too Large\r\n\r\n";
    SSL_write(ssl, resp, strlen(resp));
    return -1;
    }

    /* 已经在 router 里读到一部分 (payload 指针给出)，剩下继续读 */
    size_t have = req->payload ? (req->raw_len - (req->payload - req->raw_buf)) : 0;
    
    unsigned char *buf = malloc(content_len);
    if (!buf) return -1;
    if (have) memcpy(buf, req->payload, have);

    size_t off = have;
    while (off < (size_t)content_len) {
        size_t todo = ((content_len - off) > RX_BUF) ? RX_BUF : (content_len - off);
        int n = SSL_read(ssl, buf + off, todo);
        if (n <= 0) { free(buf); return -1; }
        off += n;
    }

    char fname[128];
    if (file_store_save(buf, content_len, fname, sizeof(fname)) == 0) {
        char resp[256];
        snprintf(resp,sizeof(resp),
                 "HTTP/1.1 201 Created\r\n"
                 "Content-Type: text/plain\r\n"
                 "Content-Length: %zu\r\n"
                 "Connection: close\r\n"
                 "Location: /image/%s\r\n\r\n"
                 "%s",
                 strlen(fname), fname, fname);
        SSL_write(ssl, resp, strlen(resp));
        free(buf);
        return 0;
    }

    free(buf);
    const char *err = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
    SSL_write(ssl, err, strlen(err));
    return -1;
}
