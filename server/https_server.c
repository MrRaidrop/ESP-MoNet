#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <sys/stat.h>
#include <time.h>
#include <cjson/cJSON.h> 

#define PORT 8443
#define MAX_CONN 10

static time_t start_time;

typedef struct {
    SSL *ssl;
    int fd;
} client_arg_t;

void *client_handler(void *arg);
void handle_request(SSL *ssl);

int main() {
    start_time = time(NULL);

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    const SSL_METHOD *method = TLS_server_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
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
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr.s_addr = INADDR_ANY
    };

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CONN) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("OTA HTTPS Server listening on https://0.0.0.0:%d\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client_fd);

        client_arg_t *client = malloc(sizeof(client_arg_t));
        client->ssl = ssl;
        client->fd = client_fd;

        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, client);
        pthread_detach(tid);
    }

    close(server_fd);
    SSL_CTX_free(ctx);
    return 0;
}

void *client_handler(void *arg) {
    client_arg_t *client = (client_arg_t *)arg;
    SSL *ssl = client->ssl;
    int fd = client->fd;
    free(client);

    if (SSL_accept(ssl) <= 0) {
        fprintf(stderr, "SSL_accept failed\n");
        ERR_print_errors_fp(stderr);
    } else {
        printf("[Client Connected] Handling request...\n");
        handle_request(ssl);  // 里面已经处理shutdown
    }

    SSL_free(ssl);  // 释放 TLS 会话资源
    close(fd);
    printf("[Client Disconnected]\n");
    return NULL;
}

void handle_request(SSL *ssl) {
    char buf[4096] = {0};
    int total_len = 0;

    while (1) {
        int len = SSL_read(ssl, buf + total_len, sizeof(buf) - total_len - 1);
        if (len <= 0) break;
        total_len += len;
    }

    printf("Received request:\n%s\n", buf);

    char method[16], path[128];
    sscanf(buf, "%15s %127s", method, path);
    printf("Parsed Method: %s, Path: %s\n", method, path);
    printf("Hex Path: ");
    for (size_t i = 0; i < strlen(path); ++i)
        printf("%02X ", (unsigned char)path[i]);
    printf("\n");

    const char *res_hdr = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n";
    char body[1024] = {0};

    if (strcmp(method, "GET") == 0 && strncmp(path, "/status", 7) == 0) {
        snprintf(body, sizeof(body), "{\"uptime\": %ld}", time(NULL) - start_time);

    } else if (strcmp(method, "POST") == 0 && strncmp(path, "/data", 5) == 0) {
        char *payload = strstr(buf, "\r\n\r\n");
        if (payload) {
            const char *json = payload + 4;
            printf("Received POST Payload:\n%s\n", json);
    
            cJSON *root = cJSON_Parse(json);
            if (root) {
                const cJSON *type = cJSON_GetObjectItem(root, "type");
                if (type && strcmp(type->valuestring, "light") == 0) {
                    const cJSON *value = cJSON_GetObjectItem(root, "value");
                    const cJSON *ts = cJSON_GetObjectItem(root, "ts");
                    if (cJSON_IsNumber(value) && cJSON_IsNumber(ts)) {
                        printf("Light = %d  (ts=%u)\n", value->valueint, ts->valueint);
                    }
                }
                cJSON_Delete(root);
            } else {
                printf("Failed to parse JSON\n");
            }
        }
        snprintf(body, sizeof(body), "{\"result\": \"ok\"}");
    }else if (strcmp(method, "GET") == 0 && strncmp(path, "/firmware.sha256", 16) == 0) {
        FILE *fp = fopen("firmware.bin", "rb");
        if (!fp) {
            perror("Failed to open firmware.bin");
            snprintf(body, sizeof(body), "{\"error\": \"not found\"}");
        } else {
            printf("Calculating SHA256 for firmware.bin...\n");
            SHA256_CTX sha;
        // Calculates SHA256 hash of "firmware.bin" and returns it as hex string
        // The hash is returned in the HTTP response body as a lowercase hex string (64 chars)

        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
            SHA256_Init(&sha);  // Initialize SHA256 context (deprecated but used for compatibility)
        #pragma GCC diagnostic pop

        unsigned char tmp[1024]; // Temporary buffer to read file chunks
        size_t n;

        // Read the firmware file in chunks and update the SHA256 hash
        while ((n = fread(tmp, 1, sizeof(tmp), fp)) > 0) {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
            SHA256_Update(&sha, tmp, n);  // Feed each chunk to the SHA256 algorithm
        #pragma GCC diagnostic pop
        }

        fclose(fp); // Close firmware file

        unsigned char hash[32]; // SHA256 produces a 256-bit hash = 32 bytes

        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
            SHA256_Final(hash, &sha);  // Finalize the SHA256 hash computation
        #pragma GCC diagnostic pop

        // Convert the binary hash to hex string format
        for (int i = 0; i < 32; ++i)
            sprintf(&body[i * 2], "%02x", hash[i]);  // Write two hex digits for each byte

        }

    } else if (strcmp(method, "GET") == 0 && strncmp(path, "/firmware.bin", 13) == 0) {
        FILE *fp = fopen("firmware.bin", "rb");
        if (!fp) {
            perror("Failed to open firmware.bin");
            snprintf(body, sizeof(body), "{\"error\": \"firmware not found\"}");
        } else {
            struct stat st;
            stat("firmware.bin", &st);
            long filesize = st.st_size;
            printf("Firmware size: %ld bytes\n", filesize);

            long range_start = 0, range_end = filesize - 1;
            char *range_hdr = strstr(buf, "Range: bytes=");
            if (range_hdr) {
                sscanf(range_hdr, "Range: bytes=%ld-", &range_start);
                if (range_start >= filesize) range_start = filesize - 1;
                range_end = filesize - 1;
                printf("Range request: start = %ld, end = %ld\n", range_start, range_end);
            }

            long chunk_size = range_end - range_start + 1;
            char header[512];
            if (range_hdr) {
                snprintf(header, sizeof(header),
                         "HTTP/1.1 206 Partial Content\r\n"
                         "Content-Type: application/octet-stream\r\n"
                         "Content-Length: %ld\r\n"
                         "Content-Range: bytes %ld-%ld/%ld\r\n\r\n",
                         chunk_size, range_start, range_end, filesize);
            } else {
                snprintf(header, sizeof(header),
                         "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: %ld\r\n\r\n",
                         filesize);
            }

            int header_len = strlen(header);
            int header_written = SSL_write(ssl, header, header_len);
            if (header_written <= 0) {
                int ssl_err = SSL_get_error(ssl, header_written);
                printf("Header SSL_write failed: SSL_get_error = %d\n", ssl_err);
                if (ssl_err == SSL_ERROR_SYSCALL) perror("SSL_write syscall error (header)");
                else if (ssl_err == SSL_ERROR_SSL) ERR_print_errors_fp(stderr);
                fclose(fp);
                return;
            }
            printf("Header sent: %d bytes\n", header_written);

            fseek(fp, range_start, SEEK_SET);

            char send_buf[1024];
            long sent = 0;
            while (sent < chunk_size) {
                int to_read = (chunk_size - sent > sizeof(send_buf)) ? sizeof(send_buf) : (chunk_size - sent);
                int read_len = fread(send_buf, 1, to_read, fp);
                if (read_len <= 0) {
                    printf("Read error or EOF reached.\n");
                    break;
                }

                printf("Read %d bytes from firmware.bin\n", read_len);

                int written = SSL_write(ssl, send_buf, read_len);
                if (written <= 0) {
                    int ssl_err = SSL_get_error(ssl, written);
                    printf("SSL_write failed: SSL_get_error = %d\n", ssl_err);
                    if (ssl_err == SSL_ERROR_SYSCALL) perror("SSL_write syscall error");
                    else ERR_print_errors_fp(stderr);
                    break;
                }

                sent += written;
                printf("Sent chunk: %d bytes (total sent: %ld/%ld)\n", written, sent, chunk_size);
            }

            fclose(fp);
            printf("Finished sending firmware.bin: %ld bytes\n", sent);

            // 安全关闭 TLS 会话
            int ret = SSL_shutdown(ssl);
            if (ret == 0) {
                ret = SSL_shutdown(ssl);
            }
            printf("SSL_shutdown result: %d\n", ret);
            return;
        }

    } else {
        printf("Unrecognized request path: '%s'\n", path);
        snprintf(body, sizeof(body), "{\"error\": \"invalid route\"}");
    }

    char response[2048];
    snprintf(response, sizeof(response), "%s%s", res_hdr, body);
    SSL_write(ssl, response, strlen(response));
}
