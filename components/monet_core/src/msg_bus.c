// monet_core/src/msg_bus.c
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "utils/log.h"
#include "monet_core/msg_bus.h"
#define TAG "MSG_BUS"
#define MAX_SUBSCRIBERS_PER_TOPIC 4


#define MAX_GLOBAL_SUBS 8
static QueueHandle_t g_global_subs[MAX_GLOBAL_SUBS] = {0};
static QueueHandle_t subscriber_queues[EVENT_SENSOR_MAX][MAX_SUBSCRIBERS_PER_TOPIC] = {0};
static SemaphoreHandle_t bus_mutex;

bool msg_bus_subscribe_any(QueueHandle_t q)
{
    if (!q) return false;
    if (bus_mutex == NULL) bus_mutex = xSemaphoreCreateMutex();
    xSemaphoreTake(bus_mutex, portMAX_DELAY);
    for (int i = 0; i < MAX_GLOBAL_SUBS; ++i) {
        if (!g_global_subs[i]) { g_global_subs[i] = q; goto ok; }
    }
    xSemaphoreGive(bus_mutex); return false;
ok:
    xSemaphoreGive(bus_mutex); return true;
}





void msg_bus_publish(const msg_t* msg)
{
    if (!msg || msg->topic >= EVENT_SENSOR_MAX) return;

    if (bus_mutex == NULL) bus_mutex = xSemaphoreCreateMutex();
    xSemaphoreTake(bus_mutex, portMAX_DELAY);

    for (int i = 0; i < MAX_SUBSCRIBERS_PER_TOPIC; ++i) {
        QueueHandle_t q = subscriber_queues[msg->topic][i];
        if (q) {
            xQueueSend(q, msg, 0);
        }
    }

    // all time subscribers can receive all messages
    for (int i = 0; i < MAX_GLOBAL_SUBS; ++i) {
        QueueHandle_t q = g_global_subs[i];
        if (q) {
            xQueueSend(q, msg, 0);
        }
    }

    xSemaphoreGive(bus_mutex);
}

bool msg_bus_subscribe(msg_topic_t topic, QueueHandle_t queue)
{
    if (!queue || topic >= EVENT_SENSOR_MAX) return false;

    if (bus_mutex == NULL) bus_mutex = xSemaphoreCreateMutex();
    xSemaphoreTake(bus_mutex, portMAX_DELAY);

    for (int i = 0; i < MAX_SUBSCRIBERS_PER_TOPIC; ++i) {
        if (subscriber_queues[topic][i] == NULL) {
            subscriber_queues[topic][i] = queue;
            xSemaphoreGive(bus_mutex);
            LOGI(TAG, "Subscribed to topic %d", topic);
            return true;
        }
    }

    xSemaphoreGive(bus_mutex);
    LOGW(TAG, "Topic %d has too many subscribers", topic);
    return false;
}
