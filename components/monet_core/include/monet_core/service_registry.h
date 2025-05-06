/**
 * @file service_registry.h
 * @brief Centralized service registry for managing lifecycle of all system services.
 *
 * This module provides functions to register, start, stop, restart, and query status of FreeRTOS-based services.
 * It enables modular service design by decoupling task creation from application logic (e.g. main.c).
 *
 * Example usage:
 * 1. Each service registers via `service_registry_register()`.
 * 2. At system boot, `service_registry_start_all()` is called.
 * 3. Services can be started/stopped individually or in batch.
 * 4. Runtime status and stack usage can be queried per service.
 */

 #pragma once

 #include <stdbool.h>
 #include <stdint.h>
 #include <freertos/FreeRTOS.h>
 #include <freertos/task.h>
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 /**
  * @brief Service state enumeration.
  */
 typedef enum {
     SERVICE_STATE_DISABLED = 0, /**< Service is not running */
     SERVICE_STATE_RUNNING       /**< Service is currently active */
 } service_state_t;
 
 /**
  * @brief Service descriptor structure.
  */
 typedef struct {
     const char *name;                /**< Unique name of the service */
     TaskFunction_t task_fn;         /**< Entry function of the service */
     const char *task_name;          /**< FreeRTOS task name */
     uint32_t stack_size;            /**< Stack size in bytes */
     UBaseType_t priority;           /**< Task priority */
 } service_desc_t;
 
 /**
  * @brief Runtime status of a registered service.
  */
 typedef struct {
     service_state_t state;          /**< Whether the service is running */
     uint32_t stack_remaining;       /**< Stack watermark (bytes remaining) */
 } service_status_t;
 
 /**
  * @brief Register a new service into the registry.
  *
  * @param desc Pointer to the service descriptor.
  */
 void service_registry_register(const service_desc_t *desc);
 
 /**
  * @brief Start a specific registered service by name.
  *
  * @param name Service name.
  * @return true on success, false if not found or already running.
  */
 bool service_registry_start(const char *name);
 
 /**
  * @brief Stop a running service.
  *
  * @param name Service name.
  * @return true on success, false if not found or not running.
  */
 bool service_registry_stop(const char *name);
 
 /**
  * @brief Restart a running service.
  *
  * @param name Service name.
  * @return true on success, false if not found.
  */
 bool service_registry_restart(const char *name);
 
 /**
  * @brief Start all services that are not currently running.
  */
 void service_registry_start_all(void);
 
 /**
  * @brief Get the current runtime status of a service.
  *
  * @param name Service name.
  * @param out_status Pointer to receive status.
  * @return true if found, false otherwise.
  */
 bool service_registry_status(const char *name, service_status_t *out_status);
 
/**
* @brief Get the stack usage of a service.
*
* @param name Service name.
* @return Stack usage in bytes, or -1 if not found.
*/
int32_t service_registry_get_stack_usage(const char *name);

 /**
  * @brief Macro to register a service with a descriptor.
  *
  * @param desc_name Name of the service descriptor.
  * @param taskname Name of the FreeRTOS task.
  * @param stack Stack size in bytes.
  * @param prio Task priority.
  * @param fn Entry function of the service.
  */
 #define SERVICE_REGISTER(desc_name, taskname, stack, prio, fn)         \
 static const service_desc_t desc_name##_svc_desc = {                    \
     .name = #desc_name,                                                 \
     .task_name = #taskname,                                             \
     .stack_size = stack,                                                \
     .priority = prio,                                                   \
     .task_fn = fn                                                       \
 };                                                                       \
 __attribute__((used, section(".rodata.svc_registry")))                  \
const service_desc_t *desc_name##_svc_ptr = &desc_name##_svc_desc;
 
 #ifdef __cplusplus
 }
 #endif
 