#pragma once
#include <openssl/ssl.h>

typedef struct http_request {
    const char *method;
    const char *path;
    const char *raw_buf;     /* full HTTP request, not 0-terminated */
    size_t      raw_len;     /* === first SSL_read() 返回值          */
    const char *payload;     /* may be NULL */
} http_request_t;


int status_handler(SSL *ssl, const http_request_t *req_buf);
int data_handler(SSL *ssl, const http_request_t *req_buf);
int ota_handler(SSL *ssl, const http_request_t *req_buf);
int sha256_handler(SSL *ssl, const http_request_t *req_buf);
int image_handler(SSL *ssl, const http_request_t *req_buf);
int image_handler  (SSL *ssl, const http_request_t *req);   // NEW
