// components/utils/src/cache.c
#include "utils/cache.h"
#include "utils/log.h"
#include "utils/config.h"

#include <string.h>
#include <stdlib.h>

#define TAG "CACHE"
#define MAX_CACHE_ITEMS        CONFIG_CACHE_MAX_ITEMS
#define MAX_ITEM_LENGTH        CONFIG_CACHE_ITEM_SIZE
#define FLUSH_INTERVAL_MS      CONFIG_CACHE_FLUSH_INTERVAL_MS

static char cache_buf[MAX_CACHE_ITEMS][MAX_ITEM_LENGTH];
static int head = 0;
static int tail = 0;
static int count = 0;

static bool is_full(void)  { return count == MAX_CACHE_ITEMS; }
static bool is_empty(void) { return count == 0; }

bool cache_push(const char *json_str)
{
    if (is_full()) {
        LOGW(TAG, "Cache full, dropping entry");
        return false;
    }
    strncpy(cache_buf[tail], json_str, MAX_ITEM_LENGTH - 1);
    cache_buf[tail][MAX_ITEM_LENGTH - 1] = '\0';
    tail = (tail + 1) % MAX_CACHE_ITEMS;
    count++;
    LOGI(TAG, "Cached item, total = %d", count);
    return true;
}

bool cache_flush_once_with_sender(bool (*send_fn)(const char *json))
{
    if (is_empty()) return false;
    if (!send_fn) return false;

    const char *item = cache_buf[head];
    bool success = send_fn(item);

    if (success) {
        LOGI(TAG, "Flushed cached item: %s", item);
        head = (head + 1) % MAX_CACHE_ITEMS;
        count--;
    } else {
        LOGW(TAG, "Flush failed, will retry later");
    }
    return success;
}

int cache_get_count(void)
{
    return count;
}

void cache_flush_task_start(void)
{
    LOGW(TAG, "Deprecated: auto flush loop removed. Use cache_flush_once_with_sender() manually.");
}
