// http_uploader_service.c
#include "freertos/FreeRTOS.h"
#include "monet_core/service_registry.h"
#include "monet_core/msg_bus.h"
#include "service/wifi_service.h"
#include "utils/cache.h"
#include "net/https_post_hal.h"
#include "utils/log.h"

#define TAG "HTTP_UPLOADER"

static bool http_sink_handler(const msg_t *m)
{
    bool ok = false;

    /* Net availablity check */
    const bool wifi_ok = wifi_service_is_connected();

    switch (m->topic) {
    case EVENT_SENSOR_JPEG: {
        /* --- JPEG upload --- */
        const camera_fb_t *fb = m->data.jpeg.fb;
        if (!fb || !fb->buf || fb->len == 0) goto EXIT;

        if (wifi_ok) {
            ok = http_post_image(fb->buf, fb->len);
        }
        if (!ok) {  /* If fail then push the binary */
            cache_push_blob(fb->buf, fb->len);
        }
        break;
    }

/** This is the entry of user want to add his own output logic */
/** -------------------------------------------------------------------------- */

    default: {   /* upload json as default */
        const char *json = m->data.json_str.json;
        if (json[0] == '\0') goto EXIT;  // throw out 

        if (wifi_ok) {
            ok = http_post_send(json);
        }
        if (!ok) {  /* If fail then push into cache */
            cache_push(json);
        }
        break;
    }
    }

EXIT:
    /* Optional, flush out cache every time */
    if (wifi_ok) {
        cache_flush_once_with_sender(http_post_send);
        cache_flush_once_with_sender_ex(http_post_image);
    }
    return ok;
}

static const msg_topic_t http_topics[] = {
#if CONFIG_ROUTE_SENSOR_GROUP_HTTP
    EVENT_GROUP_SENSOR,
#endif
#if CONFIG_ROUTE_SENSOR_LIGHT_HTTP
    EVENT_SENSOR_LIGHT,
#endif
#if CONFIG_ROUTE_JPEG_HTTP
    EVENT_SENSOR_JPEG,
#endif
    MSG_TOPIC_END
};


static const service_desc_t http_uploader_desc = {
    .name       = "http_uploader",
    .task_fn    = NULL,                   
    .stack_size = 4096,
    .priority   = 5,
    .role       = SERVICE_ROLE_SUBSCRIBER,
    .topics     = http_topics,
    .sink_cb    = http_sink_handler
};

_Static_assert(sizeof(http_topics)/sizeof(http_topics[0]) > 1, "No topics configured for HTTP uploader");

/* accessor – for main.c / auto‑register */
const service_desc_t* get_http_uploader_service(void)
{
#if CONFIG_HTTP_UPLOADER_SERVICE_ENABLE
    return &http_uploader_desc;
#else
    return NULL;
#endif
}