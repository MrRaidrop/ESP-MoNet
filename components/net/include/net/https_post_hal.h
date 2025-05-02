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

#ifdef __cplusplus
}
#endif

#endif // HTTP_POST_HAL_H_
