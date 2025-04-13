#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

void ble_service_start(void);
void ble_format_notify_data(int value, uint8_t *out);//for unity test

#ifdef __cplusplus
}
#endif

#endif // BLE_SERVICE_H
