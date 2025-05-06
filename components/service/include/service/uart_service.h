/**
 * @file uart_service.h
 * @brief UART Service module that subscribes to sensor messages and transmits them over UART.
 *
 * This service is designed to act as a message subscriber via the centralized service registry.
 * It automatically subscribes to configured sensor topics using msg_bus and transmits their
 * content over UART using the HAL interface. It also includes a receive task that reads from UART
 * and republishes the input to the internal msg_bus (e.g., EVENT_SENSOR_UART).
 */

 #pragma once

 #include "monet_core/service_registry.h"
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 /**
  * @brief Get the service descriptor for the UART service.
  *
  * This function returns a pointer to the statically defined `service_desc_t` structure,
  * which includes task metadata and a list of subscribed topics.
  * It is used by the main application to register this service with the registry.
  *
  * @return Pointer to a service descriptor structure.
  */
 const service_desc_t* get_uart_service(void);
 
 /**
  * @brief Optional manual initialization entry point.
  *
  * This function initializes the UART HAL and starts the UART receive task.
  * It can be called from `main()` or within a test framework if the registry is not used.
  *
  * The main data transmission task (`uart_service_task`) is normally launched automatically
  * by the service registry and should not be manually created.
  */
 void uart_service_start(void);
 
 /**
  * @brief Internal UART service task function.
  *
  * This task is registered with the service registry and is responsible for
  * receiving sensor messages via the shared queue (provided as a parameter)
  * and forwarding them over UART.
  *
  * @param queue_handle Pointer to the FreeRTOS queue passed in by the registry.
  */
 void uart_service_task(void *queue_handle);
 
 #ifdef __cplusplus
 }
 #endif
 