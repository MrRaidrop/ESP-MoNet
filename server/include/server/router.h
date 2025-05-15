#pragma once
#include <openssl/ssl.h>
#include "server/handlers.h"

typedef int (*handler_fn)(SSL* ssl, const http_request_t* req);

typedef struct {
    const char* method;
    const char* path_prefix;
    handler_fn handler;
} route_t;

extern route_t ROUTES[];

int router_dispatch(SSL* ssl, const char* req_buf, size_t raw_len);
