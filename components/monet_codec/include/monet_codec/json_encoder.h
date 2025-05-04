// components/monet_codec/include/monet_codec/json_encoder.h
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "monet_core/msg_bus.h"  // for msg_t

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Encode a message (`msg_t`) to JSON string.
 *
 * @param msg Pointer to the msg_t struct
 * @param out_buf Output buffer to write the JSON string
 * @param buf_size Size of the output buffer
 * @return true if successful, false otherwise
 */
bool json_encode_msg(const msg_t *msg, char *out_buf, size_t buf_size);

#ifdef __cplusplus
}
#endif
