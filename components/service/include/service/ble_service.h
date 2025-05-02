#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

void ble_service_start(void);

//void ble_format_notify_data(int value, uint8_t *out);

/**
 * @brief Notify raw binary data (MTU-sized) to the central device.
 *
 * @param data Pointer to data buffer
 * @param len  Length in bytes (must be <= negotiated MTU)
 * @return true on success
 */
bool ble_service_notify_raw(const uint8_t *data, size_t len);

/**
 * @brief Helper: notify a 32-bit integer (little-endian) to the central.
 */
static inline bool ble_service_notify_int32(int32_t value)
{
    uint8_t buf[4];
    buf[0] = (value >> 0)  & 0xFF;
    buf[1] = (value >> 8)  & 0xFF;
    buf[2] = (value >> 16) & 0xFF;
    buf[3] = (value >> 24) & 0xFF;
    return ble_service_notify_raw(buf, sizeof buf);
}

/**
 * @brief Check whether BLE service is currently connected and notifiable.
 *
 * @return true if connection and notify characteristic are ready
 */
bool ble_service_is_connected(void);



#ifdef __cplusplus
}
#endif

#endif // BLE_SERVICE_H
