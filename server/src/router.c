#include "server/router.h"
#include "server/handlers.h"
#include <string.h>
#include <stdio.h>
#include <openssl/ssl.h>

route_t ROUTES[] = {
    {"GET",  "/status",          status_handler},
    {"POST", "/data",            data_handler},
    {"GET",  "/firmware.bin",    ota_handler},
    {"GET",  "/firmware.sha256", sha256_handler},
    {"POST", "/image",           image_handler},   // ← 打开
    {NULL,   NULL,               NULL}
};


int router_dispatch(SSL* ssl, const char* req_buf, size_t raw_len) {
    char method[8] = {0}, path[128] = {0};

    if (sscanf(req_buf, "%7s %127s", method, path) != 2) {
        const char* resp = "HTTP/1.1 400 Bad Request\r\n\r\n";
        SSL_write(ssl, resp, strlen(resp));
        return -1;
    }

    path[strcspn(path, "\r\n")] = 0;

    const char* payload = strstr(req_buf, "\r\n\r\n");
    if (payload) payload += 4;

http_request_t req = {
    .method   = method,
    .path     = path,
    .raw_buf  = req_buf,
    .raw_len  = (size_t)raw_len,          
    .payload  = payload
};

    for (int i = 0; ROUTES[i].method != NULL; i++) {
        if (strcmp(method, ROUTES[i].method) == 0 &&
            strcmp(path, ROUTES[i].path_prefix) == 0) {
            return ROUTES[i].handler(ssl, &req);
        }
    }

    const char* resp = "HTTP/1.1 404 Not Found\r\n\r\n";
    SSL_write(ssl, resp, strlen(resp));
    return -1;
}
