#ifndef HTTP_POST_HAL_H_
#define HTTP_POST_HAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/**
 * @brief Post a JSON string to configured URL.
 *
 * @param json_str Null-terminated JSON string.
 * @return true if post was successful, false otherwise.
 */
bool http_post_send(const char* json_str);

/**
 * @brief Post a binary image to configured URL.
 *
 * @param data Pointer to the image data.
 * @param len Length of the image data in bytes.
 * @return true if post was successful, false otherwise.
 */
bool http_post_image(const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif // HTTP_POST_HAL_H_
