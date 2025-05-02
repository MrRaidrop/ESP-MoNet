// components/utils/include/utils/cache.h
#ifndef CACHE_H_
#define CACHE_H_

#include <stdbool.h>
#include <stddef.h>

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

#ifdef __cplusplus
}
#endif

#endif // CACHE_H_
