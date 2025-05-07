#include "monet_core/msg_bus.h"
#include "freertos/semphr.h"
#include "utils/log.h"
#include <string.h>

#define TAG "MSG_BUS"
#define MAX_SUBSCRIBERS_PER_TOPIC 8 // Maximum number of subscribers allowed per individual topic
#define MAX_SUBSCRIBERS_PER_GROUP 8 // Maximum number of subscribers allowed per topic group
// could be 4, but let's keep it 8 for now, there are uart, ble, http for now, and mqtt, sd cart for future

// Per-topic subscriber queues
static QueueHandle_t subscriber_queues[EVENT_SENSOR_MAX][MAX_SUBSCRIBERS_PER_TOPIC];
// group-wide subscriber queues (e.g. for EVENT_GROUP_SENSOR)
static QueueHandle_t group_queues[MAX_SUBSCRIBERS_PER_GROUP];
// for bounds checking
static size_t group_queue_count = 0;

static SemaphoreHandle_t bus_mutex = NULL;

/**
 * @brief Determine group ID from a given topic.
 *
 * Currently all sensor topics map to the EVENT_GROUP_SENSOR group.
 *
 * @param topic The message topic to classify.
 * @return Corresponding group ID, or 0xFFFF if no group found.
 */
static inline uint16_t get_group_id(msg_topic_t topic) {
    if (topic < EVENT_SENSOR_MAX) return EVENT_GROUP_SENSOR;  // default grouping
    return 0xFFFF;
}

void msg_bus_publish(const msg_t* msg)
{
    if (!msg) return;

    if (bus_mutex == NULL) bus_mutex = xSemaphoreCreateMutex();
    xSemaphoreTake(bus_mutex, portMAX_DELAY); // wait for mutex forever

    // Publish to topic-specific subscribers
    if (msg->topic < EVENT_SENSOR_MAX) {
        for (int i = 0; i < MAX_SUBSCRIBERS_PER_TOPIC; ++i) {
            if (subscriber_queues[msg->topic][i] != NULL) {
                xQueueSend(subscriber_queues[msg->topic][i], msg, 0);
            }
        }
    }

    // Publish to group-wide subscribers
    uint16_t group = get_group_id(msg->topic);
    if (group == EVENT_GROUP_SENSOR) {
        for (int i = 0; i < group_queue_count; ++i) {
            if (group_queues[i] != NULL) {
                xQueueSend(group_queues[i], msg, 0);
            }
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

bool msg_bus_subscribe_group(uint16_t group_id, QueueHandle_t queue)
{
    if (group_id != EVENT_GROUP_SENSOR || !queue) return false;

    if (bus_mutex == NULL) bus_mutex = xSemaphoreCreateMutex();
    xSemaphoreTake(bus_mutex, portMAX_DELAY);

    for (int i = 0; i < MAX_SUBSCRIBERS_PER_GROUP; ++i) {
        if (group_queues[i] == NULL) {
            group_queues[i] = queue;
            ++group_queue_count;
            xSemaphoreGive(bus_mutex);
            LOGI(TAG, "Subscribed to group %d", group_id);
            return true;
        }
    }

    xSemaphoreGive(bus_mutex);
    LOGW(TAG, "Group %d has too many subscribers", group_id);
    return false;
}

bool msg_bus_subscribe_any(QueueHandle_t q)
{
    return msg_bus_subscribe_group(EVENT_GROUP_SENSOR, q);
}
