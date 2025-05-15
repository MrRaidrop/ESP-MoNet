#include <openssl/ssl.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <string.h>
#include "server/handlers.h"

int sha256_handler(SSL *ssl, const http_request_t *req)  {
    char body[128];
    FILE *fp = fopen("firmware.bin", "rb");
    if (!fp) {
        snprintf(body, sizeof(body), "{\"error\": \"firmware not found\"}");
    } else {
        unsigned char hash[32];
        SHA256_CTX sha;
        SHA256_Init(&sha);
        char tmp[1024];
        size_t n;
        while ((n = fread(tmp, 1, sizeof(tmp), fp)) > 0) {
            SHA256_Update(&sha, tmp, n);
        }
        fclose(fp);
        SHA256_Final(hash, &sha);
        for (int i = 0; i < 32; ++i)
            sprintf(&body[i * 2], "%02x", hash[i]);
    }

    char resp[256];
    snprintf(resp, sizeof(resp),
             "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n%s",
             body);

    return SSL_write(ssl, resp, strlen(resp));
}
