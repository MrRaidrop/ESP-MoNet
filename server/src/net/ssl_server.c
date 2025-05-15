#include "server/ssl_server.h"
#include "server/router.h"
#include "server/log.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


#define TAG "SSL-SERVER"
#define REQ_BUF_SIZE 4096

static SSL_CTX* ctx = NULL;
static int listen_fd = -1;


/* --- 每个连接线程 --- */
static void* client_thread(void* arg) {
    SSL* ssl = (SSL*)arg;

    char buf[REQ_BUF_SIZE] = {0};
    int len = SSL_read(ssl, buf, sizeof(buf) - 1);
    printf("[DEBUG] SSL_read len=%d\n", len);
    if (len > 0) printf("[DEBUG] >>> %.80s\n", buf);
    if (len <= 0) goto EXIT;

    /* 仅做一次调度：交给 router */
    router_dispatch(ssl, buf, (size_t)len);

EXIT:
    SSL_shutdown(ssl);
    SSL_free(ssl);
    return NULL;
}

static void handle_sigint(int signo) {
    if (listen_fd != -1) close(listen_fd);
    if (ctx) SSL_CTX_free(ctx);
    LOGI(TAG,"Server stopped");
    exit(0);
}


/* --- 创建 socket 并绑定端口 --- */
static int create_listen_socket(uint16_t port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(port)};
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) return -1;
    if (listen(fd, 16) < 0) return -1;
    return fd;
}

int ssl_server_start(const server_config_t* cfg) {
    OpenSSL_add_ssl_algorithms();
    SSL_load_error_strings();

    ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) {
        LOGE(TAG,"SSL_CTX_new failed");
        return -1;
    }
    if (SSL_CTX_use_certificate_file(ctx, cfg->cert_file, SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(ctx, cfg->key_file, SSL_FILETYPE_PEM) <= 0) {
        LOGE(TAG,"load cert/key failed");
        return -1;
    }

    listen_fd = create_listen_socket(cfg->port);
    if (listen_fd < 0) {
        LOGE(TAG,"failed to bind port %d", cfg->port);
        return -1;
    }
    LOGI(TAG,"Listening on https://0.0.0.0:%d ...", cfg->port);

    /* Ctrl‑C 优雅退出 */
    
    signal(SIGINT, handle_sigint);

    while (1) {
        int cli = accept(listen_fd, NULL, NULL);
        if (cli < 0) continue;

        SSL* ssl = SSL_new(ctx);
        SSL_set_fd(ssl, cli);
        if (SSL_accept(ssl) <= 0) {
            SSL_free(ssl);
            close(cli);
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, ssl);
        pthread_detach(tid);  // 方便回收
    }
    return 0;
}
