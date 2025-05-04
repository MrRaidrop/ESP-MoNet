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
        handle_request(ssl);
    }

    SSL_free(ssl);
    close(fd);
    printf("[Client Disconnected]\n");
    return NULL;
}

void handle_request(SSL *ssl) {
    char buf[4096] = {0};

    // �?只读一次请求头，防�?curl 卡死
    int len = SSL_read(ssl, buf, sizeof(buf) - 1);
    if (len <= 0) {
        fprintf(stderr, "�?SSL_read failed or client closed early\n");
        return;
    }
    buf[len] = '\0';

    printf("📥 Received request:\n%s\n", buf);

    // �?只解析首�?
    char method[16], path[256];
    if (sscanf(buf, "%15s %255s", method, path) != 2) {
        fprintf(stderr, "�?Failed to parse request line\n");
        return;
    }

    printf("➡️  Method: %s | Path: %s\n", method, path);

    // �?/firmware.bin 直接返回二进�?
    if (strcmp(method, "GET") == 0 && strncmp(path, "/firmware.bin", 13) == 0) {
        FILE *fp = fopen("firmware.bin", "rb");
        if (!fp) {
            perror("�?Failed to open firmware.bin");
            const char *notfound = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
            SSL_write(ssl, notfound, strlen(notfound));
            return;
        }

        struct stat st;
        fstat(fileno(fp), &st);
        long filesize = st.st_size;
        long range_start = 0, range_end = filesize - 1;

        // �?断点续传 Range 支持（可选）
        char *range_hdr = strstr(buf, "Range: bytes=");
        if (range_hdr) {
            sscanf(range_hdr, "Range: bytes=%ld-", &range_start);
            if (range_start >= filesize) range_start = filesize - 1;
            printf("📦 Range: %ld - %ld\n", range_start, range_end);
        }

        long chunk_size = range_end - range_start + 1;
        char header[512];
        if (range_hdr) {
            snprintf(header, sizeof(header),
                     "HTTP/1.1 206 Partial Content\r\n"
                     "Content-Type: application/octet-stream\r\n"
                     "Content-Length: %ld\r\n"
                     "Content-Range: bytes %ld-%ld/%ld\r\n"
                     "Connection: close\r\n\r\n",
                     chunk_size, range_start, range_end, filesize);
        } else {
            snprintf(header, sizeof(header),
                     "HTTP/1.1 200 OK\r\n"
                     "Content-Type: application/octet-stream\r\n"
                     "Content-Length: %ld\r\n"
                     "Connection: close\r\n\r\n",
                     filesize);
        }

        SSL_write(ssl, header, strlen(header));
        fseek(fp, range_start, SEEK_SET);

        char send_buf[1024];
        long sent = 0;
        while (sent < chunk_size) {
            int to_read = (chunk_size - sent > sizeof(send_buf)) ? sizeof(send_buf) : (chunk_size - sent);
            int read_len = fread(send_buf, 1, to_read, fp);
            if (read_len <= 0) break;

            int written = SSL_write(ssl, send_buf, read_len);
            if (written <= 0) break;

            sent += written;
        }

        fclose(fp);
        printf("�?Sent firmware.bin: %ld bytes\n", sent);
        return;  // �?VERY IMPORTANT! 防止继续执行 JSON 逻辑
    }

    // �?其他逻辑：返�?JSON
    char body[1024] = {0};
    const char *res_hdr = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n";

    if (strcmp(method, "GET") == 0 && strncmp(path, "/status", 7) == 0) {
        snprintf(body, sizeof(body), "{\"uptime\": %ld}", time(NULL) - start_time);

    } else if (strcmp(method, "POST") == 0 && strncmp(path, "/data", 5) == 0) {
        char *payload = strstr(buf, "\r\n\r\n");
        if (payload) {
            printf("📨 POST Payload:\n%s\n", payload + 4);
        }
        snprintf(body, sizeof(body), "{\"result\": \"ok\"}");

    } else if (strcmp(method, "GET") == 0 && strncmp(path, "/firmware.sha256", 16) == 0) {
        FILE *fp = fopen("firmware.bin", "rb");
        if (!fp) {
            snprintf(body, sizeof(body), "{\"error\": \"firmware not found\"}");
        } else {
            unsigned char hash[32];
            SHA256_CTX sha;
            SHA256_Init(&sha);

            char tmp[1024];
            size_t n;
            while ((n = fread(tmp, 1, sizeof(tmp), fp)) > 0)
                SHA256_Update(&sha, tmp, n);
            fclose(fp);
            SHA256_Final(hash, &sha);

            for (int i = 0; i < 32; ++i)
                sprintf(&body[i * 2], "%02x", hash[i]);
        }

    } else {
        snprintf(body, sizeof(body), "{\"error\": \"invalid route\"}");
    }

    // �?最后统一�?JSON 响应
    char response[2048];
    snprintf(response, sizeof(response), "%s%s", res_hdr, body);
    SSL_write(ssl, response, strlen(response));
}
