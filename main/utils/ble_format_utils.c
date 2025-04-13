#include "ble_format_utils.h"

void ble_format_notify_data(int value, uint8_t *out) {
    out[0] = (value >> 0) & 0xFF;
    out[1] = (value >> 8) & 0xFF;
    out[2] = (value >> 16) & 0xFF;
    out[3] = (value >> 24) & 0xFF;
}
