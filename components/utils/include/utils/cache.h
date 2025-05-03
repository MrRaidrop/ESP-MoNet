// components/utils/include/utils/cache.h
#ifndef CACHE_H_
#define CACHE_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cache.h
 * @brief In-memory upload cache for retrying failed data transmissions.
 *
 * This module provides a RAM-based circular buffer to store outgoing
 * JSON payloads when network connections (Wi-Fi/BLE) are unavailable.
 * A background task periodically attempts to flush cached entries.
 */

/**
 * @brief Push a JSON string into the upload cache.
 *
 * The string will be copied into internal buffer (max 128 bytes).
 *
 * @param json_str Null-terminated string
 * @return true if pushed successfully, false if cache is full
 */
bool cache_push(const char *json_str);

/**
 * @brief Start the periodic flush task to retry cached uploads.
 */
void cache_flush_task_start(void);

/**
 * @brief Flush one cached item manually (e.g. from test).
 */
void cache_flush_once(void);

/**
 * @brief Get the current number of cached items pending upload.
 *
 * @return Number of valid cached entries (0 ~ MAX)
 */
int cache_get_count(void);

/**
 * @brief Flushes one cached item using a JSON string sender function.
 *
 * This function retrieves the oldest cached string entry and attempts to send it
 * using the provided `send_fn` function pointer. If sending succeeds, the item
 * is removed from the cache. Otherwise, it will remain in the buffer for future retries.
 *
 * This is typically used for string-formatted data, such as JSON payloads.
 *
 * @param send_fn A function pointer that takes a JSON string (const char *) and returns true on success.
 * @return true if the item was sent and removed from cache; false otherwise.
 */
bool cache_flush_once_with_sender(bool (*send_fn)(const char *json));

/**
 * @brief Flushes one cached item using a binary sender function.
 *
 * This extended version supports binary-safe transmission. It retrieves the oldest
 * cached item (stored as a string), and passes its pointer and length to the user-provided
 * binary `send_fn`. This is useful when the data being sent is not null-terminated or
 * is expected to be treated as raw binary (e.g., JPEG image).
 *
 * @param send_fn A function pointer that accepts a binary buffer and its length,
 *                and returns true on successful transmission.
 * @return true if the item was sent and removed from cache; false otherwise.
 */
bool cache_flush_once_with_sender_ex(bool (*send_fn)(const uint8_t *data, size_t len));

/**
 * @brief Push a binary blob (e.g. JPEG image) into the cache ring buffer.
 *
 * This function is used to store arbitrary binary data for later retry,
 * typically when uploading to the server fails (e.g. Wi-Fi disconnected).
 * 
 * The blob will be stored in PSRAM (or internal heap) as a deep copy,
 * and the function ensures the size does not exceed CONFIG_CACHE_ITEM_SIZE.
 * 
 * @param buf Pointer to the binary data to be cached
 * @param len Length of the binary data in bytes
 * @return true on success, false if cache is full or data too large
 */
bool cache_push_blob(const void *buf, size_t len);

/**
 * @brief Log cache cur statues, performance monitoring
 */
void cache_log_stats(void);


#ifdef __cplusplus
}
#endif

#endif // CACHE_H_
