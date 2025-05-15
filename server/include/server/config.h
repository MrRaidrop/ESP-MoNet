#pragma once

typedef struct {
    unsigned short port;
    const char* cert_file;
    const char* key_file;
    const char* storage_root;
    unsigned max_clients;
} server_config_t;

server_config_t config_load(const char* path);
