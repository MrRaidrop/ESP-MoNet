 #ifndef MONET_CORE_SERVICE_REGISTRY_H
 #define MONET_CORE_SERVICE_REGISTRY_H

 #include <stdbool.h>
 #include <stdint.h>
 #include <freertos/FreeRTOS.h>
 #include <freertos/task.h>
 #include "monet_core/msg_bus.h"
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 #define MSG_TOPIC_END EVENT_SENSOR_MAX

 /**
  * @brief Enumeration for the runtime state of a service.
  */
 typedef enum {
     SERVICE_STATE_DISABLED = 0,     /**< Service is not running */
     SERVICE_STATE_RUNNING           /**< Service is actively running */
 } service_state_t;

 /**
  * @brief Enumeration for the characteristics of a service.
  * e.g : light sensor/temp_hum sensor is publisher, ble/http is subscriber
  */
 typedef enum {
    SERVICE_ROLE_NONE = 0,           /**< Not pub not sub, e.g. wifi_service */
    SERVICE_ROLE_PUBLISHER,          /**< Sensors or data sources (e.g., light, camera) that publish messages */
    SERVICE_ROLE_SUBSCRIBER          /**< Services (e.g., UART, BLE, HTTP) that consume messages via msg_bus */
} service_role_t;

/**
 * @brief Callback function for subscriber services to handle incoming messages.
 *
 * This function is called by the internal dispatch task of `service_registry` 
 * whenever a subscribed message is received. If provided, this callback replaces 
 * the need to define a dedicated FreeRTOS task.
 *
 * @param m Pointer to the incoming message (`msg_t`)
 * @return true if message handled successfully, false otherwise
 */
typedef bool (*sink_cb_t)(const msg_t *m);


 /**
  * @brief Structure that describes a service to be managed by the registry.
  *
  * This structure is used to define the metadata of a service (task),
  * including its function pointer, stack size, and priority.
  */
 typedef struct {
     const char *name;               /**< Unique identifier of the service */
     TaskFunction_t task_fn;         /**< Task entry function */
     const char *task_name;          /**< Name used for the FreeRTOS task */
     uint32_t stack_size;            /**< Stack size in bytes */
     UBaseType_t priority;           /**< Task execution priority */
     /* NEW ↓↓↓ */
     service_role_t    role;         /**< publisher or subscriber or none */
     const msg_topic_t *topics;           
      /**< NULL-terminated list of topics to subscribe to, used only if role is SUBSCRIBER */
      bool (*sink_cb)(const msg_t *);
      /**< Optional callback to handle incoming messages */
 } service_desc_t;
 
 /**
  * @brief Structure representing the runtime status of a service.
  */
 typedef struct {
     service_state_t state;          /**< Current state: running or disabled */
     uint32_t stack_remaining;       /**< Stack watermark in bytes (lowest free amount) */
 } service_status_t;
 
 /**
  * @brief Register a new service to the registry.
  *
  * This must be called before attempting to start a service.
  *
  * @param desc Pointer to the service descriptor structure.
  */
 void service_registry_register(const service_desc_t *desc);
 
 /**
  * @brief Start a registered service by its name.
  *
  * @param name The name of the service to start.
  * @return true if started successfully, false if not found or already running.
  */
 bool service_registry_start(const char *name);
 
 /**
  * @brief Stop a currently running service by its name.
  *
  * @param name The name of the service to stop.
  * @return true if stopped successfully, false if not found or not running.
  */
 bool service_registry_stop(const char *name);
 
 /**
  * @brief Restart a service by stopping and then starting it again.
  *
  * @param name The name of the service to restart.
  * @return true if successfully restarted, false otherwise.
  */
 bool service_registry_restart(const char *name);
 
 /**
  * @brief Start all registered services that are currently disabled.
  */
 void service_registry_start_all(void);
 
 /**
  * @brief Query the runtime status and stack usage of a service.
  *
  * @param name The name of the service to query.
  * @param out_status Pointer to a structure that will receive the status info.
  * @return true if the service was found and status is returned, false otherwise.
  */
 bool service_registry_status(const char *name, service_status_t *out_status);
 
 /**
  * @brief Get the current stack watermark (high water mark) of a service.
  *
  * @param name The name of the service.
  * @return Remaining stack in bytes, or -1 if the service was not found.
  */
 int32_t service_registry_get_stack_usage(const char *name);
 
 #ifdef __cplusplus
 }
 #endif

 #endif // MONET_CORE_SERVICE_REGISTRY_H
 