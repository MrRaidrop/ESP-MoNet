/**
 * @file dht22_service.h
 * @brief Periodic DHT22 polling and publishing via msg_bus.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Start the DHT22 service task.
 */
void dht22_service_start(void);

#ifdef __cplusplus
}
#endif