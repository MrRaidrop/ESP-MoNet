// json_utils.c
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "json_utils.h"

char *build_json_payload(const char *uart_data, int *counter)
{
    static char json_buf[256];

    time_t now;
    time(&now);
    struct tm *timeinfo = localtime(&now);

    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);

    snprintf(json_buf, sizeof(json_buf),
             "{\"esp32\": \"%s\", \"uart_data\": \"%s\", \"hello\": %d}",
             time_str, uart_data, *counter);
    (*counter)++;

    return json_buf;
}
