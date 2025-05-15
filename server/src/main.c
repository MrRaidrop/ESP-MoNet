#include "server/ssl_server.h"
#include <stdio.h>

time_t start_time;

int main(void) {

    start_time = time(NULL);
    
    server_config_t cfg = {
        .port = 8443,
        .cert_file = "cert.pem",
        .key_file = "key.pem",
        .max_clients = 32};

    printf("Starting HTTPS server...\n");
    return ssl_server_start(&cfg);
}
