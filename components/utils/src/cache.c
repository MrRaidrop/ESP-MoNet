#include "utils/cache.h"
#include "utils/log.h"
#include "utils/config.h"

#include <string.h>
#include <stdlib.h>

#define TAG "CACHE"
#define MAX_CACHE_ITEMS        CONFIG_CACHE_MAX_ITEMS
#define MAX_ITEM_LENGTH        CONFIG_CACHE_ITEM_SIZE
#define FLUSH_INTERVAL_MS      CONFIG_CACHE_FLUSH_INTERVAL_MS

typedef struct {
    uint8_t data[MAX_ITEM_LENGTH];  ///< Payload (can be JSON or binary blob)
    size_t len;                     ///< Actual length of data
} cache_entry_t;

static cache_entry_t cache_buf[MAX_CACHE_ITEMS];
static int head = 0;
static int tail = 0;
static int count = 0;

static bool is_full(void)  { return count == MAX_CACHE_ITEMS; }
static bool is_empty(void) { return count == 0; }

/**
 * @brief Push a JSON string into the cache.
 *
 * Used for sensor JSON payloads. Data is copied into ring buffer with length recorded.
 */
bool cache_push(const char *json_str)
{
    if (is_full()) {
        LOGW(TAG, "Cache full, dropping JSON entry");
        return false;
    }

    size_t len = strnlen(json_str, MAX_ITEM_LENGTH - 1);
    memcpy(cache_buf[tail].data, json_str, len);
    cache_buf[tail].data[len] = '\0';  // Ensure null-terminated for JSON safety
    cache_buf[tail].len = len + 1;

    tail = (tail + 1) % MAX_CACHE_ITEMS;
    count++;

    LOGI(TAG, "Cached JSON item (%u bytes), total = %d/%d", (unsigned)(len + 1), count, MAX_CACHE_ITEMS);
    return true;
}

/**
 * @brief Flush one cached JSON entry via string-based sender callback.
 */
bool cache_flush_once_with_sender(bool (*send_fn)(const char *json))
{
    if (is_empty()) return false;
    if (!send_fn) return false;

    const char *item = (const char *)cache_buf[head].data;
    bool success = send_fn(item);

    if (success) {
        LOGI(TAG, "Flushed cached JSON: %s", item);
        head = (head + 1) % MAX_CACHE_ITEMS;
        count--;
    } else {
        LOGW(TAG, "Flush (JSON) failed, will retry later");
    }
    return success;
}

/**
 * @brief Flush one cached binary blob via binary sender callback.
 */
bool cache_flush_once_with_sender_ex(bool (*send_fn)(const uint8_t *data, size_t len))
{
    if (is_empty()) return false;
    if (!send_fn) return false;

    const uint8_t *item = cache_buf[head].data;
    size_t len = cache_buf[head].len;

    bool success = send_fn(item, len);

    if (success) {
        LOGI(TAG, "Flushed cached binary (%u bytes)", (unsigned)len);
        head = (head + 1) % MAX_CACHE_ITEMS;
        count--;
    } else {
        LOGW(TAG, "Flush (binary) failed, will retry later");
    }
    return success;
}

/**
 * @brief Push a binary blob (e.g. JPEG) into the cache.
 */
bool cache_push_blob(const void *buf, size_t len)
{
    if (is_full()) {
        LOGW(TAG, "Cache full, dropping binary");
        return false;
    }
    if (len > MAX_ITEM_LENGTH) {
        LOGW(TAG, "Binary too large to cache (%u > %u)", (unsigned)len, MAX_ITEM_LENGTH);
        return false;
    }

    memcpy(cache_buf[tail].data, buf, len);
    cache_buf[tail].len = len;

    tail = (tail + 1) % MAX_CACHE_ITEMS;
    count++;

    LOGI(TAG, "Cached binary item (%u bytes), total = %d/%d", (unsigned)len, count, MAX_CACHE_ITEMS);
    return true;
}

/**
 * @brief Print current cache statistics to log.
 */
void cache_log_stats(void)
{
    LOGI(TAG, "[Cache] Entries: %d/%d | Head=%d | Tail=%d", count, MAX_CACHE_ITEMS, head, tail);
}

/**
 * @brief Return current number of items in cache.
 */
int cache_get_count(void)
{
    return count;
}

/**
 * @brief Deprecated auto-flush task (manually call flush instead).
 */
void cache_flush_task_start(void)
{
    LOGW(TAG, "Deprecated: auto flush loop removed. Use cache_flush_once_with_sender[_ex]() manually.");
}
