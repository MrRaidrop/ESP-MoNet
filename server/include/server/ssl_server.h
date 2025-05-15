#pragma once
#include <openssl/ssl.h>
#include <stdint.h>

#include "server/router.h"   // 需要 handler 定义

typedef struct {
    uint16_t port;           // 监听端口
    const char* cert_file;   // PEM 证书
    const char* key_file;    // PEM 私钥
    unsigned   max_clients;  // 线程上限
} server_config_t;

/* 启动阻塞式服务器。返回值：
 *  0 = 正常退出
 * -1 = 初始化失败
 */
int ssl_server_start(const server_config_t* cfg);
