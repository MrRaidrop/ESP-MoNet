#include <openssl/ssl.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "server/handlers.h"

extern time_t start_time;

int status_handler(SSL *ssl, const http_request_t *req)  {
    char body[128];
    snprintf(body, sizeof(body),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: application/json\r\n"
             "Connection: close\r\n\r\n"
             "{\"uptime\": %ld}",
             time(NULL) - start_time);

    return SSL_write(ssl, body, strlen(body));
}
