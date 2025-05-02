// json_utils.c
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>

#include "utils/json_utils.h"

void json_utils_build_light_sensor_json(char *out_buf, size_t buf_size, int light_val, uint32_t timestamp)
{
    snprintf(out_buf, buf_size,
        "{ \"type\": \"light\", \"value\": %d, \"ts\": %" PRIu32 " }",
        light_val, timestamp);
}

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
