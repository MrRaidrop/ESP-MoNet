#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <time.h>

#define PORT 8443
#define MAX_CONN 10

time_t start_time;
SSL_CTX *ctx; // 全局 SSL context

void handle_request(SSL *ssl) {
    char buf[4096] = {0};
    int total_len = 0;

    while (1) {
        int len = SSL_read(ssl, buf + total_len, sizeof(buf) - total_len - 1);
        if (len <= 0) break;
        total_len += len;
        if (total_len >= sizeof(buf) - 1) break;
    }

    char method[16] = {0}, path[64] = {0};
    sscanf(buf, "%s %s", method, path);

    const char *response_headers = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n";
    char body[1024] = {0};

    if (strcmp(method, "GET") == 0 && strcmp(path, "/status") == 0) {
        time_t now = time(NULL);
        snprintf(body, sizeof(body), "{\"status\": \"ok\", \"uptime\": %ld}", now - start_time);
    } else if (strcmp(method, "POST") == 0 && strcmp(path, "/data") == 0) {
        char *body_start = strstr(buf, "\r\n\r\n");
        if (body_start) {
            body_start += 4;
            printf("=== POST body received ===\n%s\n===========================\n", body_start);
        } else {
            printf("POST body not found\n");
        }
        snprintf(body, sizeof(body), "{\"result\": \"received\"}");
    } else if (strcmp(method, "GET") == 0 && strcmp(path, "/firmware.bin") == 0) {
        FILE *fp = fopen("firmware.bin", "rb");
        if (fp) {
            fseek(fp, 0, SEEK_END);
            long filesize = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            char *bin_data = malloc(filesize);
            fread(bin_data, 1, filesize, fp);
            fclose(fp);

            char header_buf[512];
            snprintf(header_buf, sizeof(header_buf),
                     "HTTP/1.1 200 OK\r\n"
                     "Content-Type: application/octet-stream\r\n"
                     "Content-Length: %ld\r\n"
                     "Content-Disposition: attachment; filename=\"firmware.bin\"\r\n\r\n", filesize);
            SSL_write(ssl, header_buf, strlen(header_buf));
            SSL_write(ssl, bin_data, filesize);
            free(bin_data);
            return;
        } else {
            snprintf(body, sizeof(body), "{\"error\": \"firmware not found\"}");
        }
    } else {
        snprintf(body, sizeof(body), "{\"error\": \"not found\"}");
    }

    char response[2048] = {0};
    snprintf(response, sizeof(response), "%s%s", response_headers, body);
    SSL_write(ssl, response, strlen(response));
}

void *client_thread(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);

    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client_fd);

    if (SSL_accept(ssl) <= 0) {
        fprintf(stderr, "TLS handshake failed\n");
        ERR_print_errors_fp(stderr);
    } else {
        printf("📡 New client connected\n");
        handle_request(ssl);
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(client_fd);
    return NULL;
}

int main() {
    start_time = time(NULL);

    // 初始化 OpenSSL
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    const SSL_METHOD *method = TLS_server_method();
    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CONN) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("✅ HTTPS OTA Server running on https://0.0.0.0:%d (Max %d clients)\n", PORT, MAX_CONN);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int *client_fd = malloc(sizeof(int));
        *client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len);
        if (*client_fd < 0) {
            perror("accept failed");
            free(client_fd);
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, client_fd);
        pthread_detach(tid); // 自动回收线程资源
    }

    close(server_fd);
    SSL_CTX_free(ctx);
    return 0;
}
// a little bit of multi-thread