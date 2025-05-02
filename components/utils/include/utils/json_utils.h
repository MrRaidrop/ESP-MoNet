// json_utils.h
#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Build a JSON string for light sensor data.
 *
 * This function formats a JSON object containing the light sensor value
 * and a timestamp into the provided output buffer.
 *
 * Example output:
 * { "type": "light", "value": 1234, "ts": 16240012 }
 *
 * @param[out] out_buf     Target buffer to write the JSON string
 * @param[in]  buf_size    Size of the output buffer
 * @param[in]  light_val   Integer light value to include
 * @param[in]  timestamp   Timestamp in milliseconds
 */
void json_utils_build_light_sensor_json(char *out_buf, size_t buf_size, int light_val, uint32_t timestamp);

/**
 * @brief Build a JSON string containing UART payload and sequence counter.
 *
 * This function generates a JSON payload embedding a timestamp, an input
 * UART string, and an incremented counter. It returns a pointer to a static
 * buffer containing the JSON result.
 *
 * Note: This function is not thread-safe due to internal static buffer use.
 *
 * Example output:
 * { "type": "uart", "data": "<your data>", "count": 42, "ts": 16240123 }
 *
 * @param uart_data Null-terminated string received from UART
 * @param counter Pointer to an integer counter (auto-increments on call)
 * @return Pointer to static internal buffer containing JSON string
 */
char *build_json_payload(const char *uart_data, int *counter);

#ifdef __cplusplus
}
#endif