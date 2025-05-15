#include <openssl/ssl.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include "server/handlers.h"

/* 读取并发送二进制 OTA 包。
 * 兼容 Range: bytes=<start>-   断点续传，返回 206 Partial Content。
 */
int ota_handler(SSL *ssl, const http_request_t *req)
{
    FILE *fp = fopen("firmware.bin", "rb");
    if (!fp) {
        const char *nf = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
        SSL_write(ssl, nf, strlen(nf));
        return 0;
    }

    /* --- 获取文件大小 --- */
    struct stat st;
    fstat(fileno(fp), &st);
    const long filesize = st.st_size;
    long range_start = 0;                 /* 默认从头发 */
    long range_end   = filesize - 1;

    /* --- 解析 Range 头 (只支持 bytes=<start>- ) --- */
    if (req->raw_buf) {
        const char *range_hdr = strstr(req->raw_buf, "Range: bytes=");
        if (range_hdr) {
            sscanf(range_hdr, "Range: bytes=%ld-", &range_start);
            if (range_start < 0 || range_start >= filesize)
                range_start = filesize - 1;   /* 容错：超界则发最后 1 字节 */
        }
    }

    const long chunk_sz = range_end - range_start + 1;

    /* --- 发送响应头 --- */
    char hdr[256];
    if (range_start > 0) {   /* 断点续传 206 */
        snprintf(hdr, sizeof(hdr),
                 "HTTP/1.1 206 Partial Content\r\n"
                 "Content-Type: application/octet-stream\r\n"
                 "Content-Length: %ld\r\n"
                 "Content-Range: bytes %ld-%ld/%ld\r\n"
                 "Connection: close\r\n\r\n",
                 chunk_sz, range_start, range_end, filesize);
    } else {                 /* 全量下载 200 */
        snprintf(hdr, sizeof(hdr),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: application/octet-stream\r\n"
                 "Content-Length: %ld\r\n"
                 "Connection: close\r\n\r\n",
                 filesize);
    }
    SSL_write(ssl, hdr, strlen(hdr));

    /* --- 发送主体 --- */
    fseek(fp, range_start, SEEK_SET);
    char buf[1024];
    long sent = 0;
    while (sent < chunk_sz) {
        const int to_read = (chunk_sz - sent > (long)sizeof(buf)) ? sizeof(buf)
                                                                  : (int)(chunk_sz - sent);
        const int r = fread(buf, 1, to_read, fp);
        if (r <= 0) break;
        const int w = SSL_write(ssl, buf, r);
        if (w <= 0) break;
        sent += w;
    }
    fclose(fp);
    printf("[OTA] Sent firmware.bin: %ld bytes (range %ld-%ld)\n",
           sent, range_start, range_start + sent - 1);
    return 0;
}
