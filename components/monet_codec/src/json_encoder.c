// components/monet_codec/src/json_encoder.c

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "monet_codec/json_encoder.h"
#include "monet_core/msg_bus.h"
#include "utils/log.h"

bool json_encode_msg(const msg_t *msg, char *out_buf, size_t buf_size)
{
    if (!msg || !out_buf || buf_size == 0) {
        return false;
    }

    switch (msg->topic) {
    case EVENT_SENSOR_LIGHT:
        snprintf(out_buf, buf_size,
                 "{ \"type\": \"light\", \"value\": %ld, \"ts\": %" PRIu32 " }",
                 msg->data.value_int, msg->ts_ms);
        LOGI("JSON_ENCODER", "Encoded JSON: %s", out_buf);
        return true;

    case EVENT_SENSOR_TEMP:
        snprintf(out_buf, buf_size,
                 "{ \"type\": \"temp\", \"temperature\": %.2f, \"humidity\": %.2f, \"ts\": %" PRIu32 " }",
                 msg->data.temp_hum.temperature,
                 msg->data.temp_hum.humidity,
                 msg->ts_ms);
        return true;

    default:
        snprintf(out_buf, buf_size,
                 "{ \"type\": \"unknown\", \"topic\": %d, \"ts\": %" PRIu32 " }",
                 msg->topic, msg->ts_ms);
        return false;
    }
}
