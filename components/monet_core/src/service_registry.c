/**
 * @file service_registry.c
 * @brief Implementation of centralized service registry for managing lifecycle of all system services.
 */

 #include <string.h>
 #include <stdlib.h>
 #include "monet_core/service_registry.h"
 #include "monet_core/msg_bus.h"
 #include "utils/log.h"
 
 #define MAX_SERVICES 16
 #define TAG "SVC_REG"
 #define MSG_TOPIC_END EVENT_SENSOR_MAX
 
 typedef struct {
     const service_desc_t *desc;
     TaskHandle_t handle;
     service_state_t state;
 } service_entry_t;
 
 static service_entry_t s_registry[MAX_SERVICES];
 static size_t s_registry_count = 0;
 
 void service_registry_register(const service_desc_t *desc)
 {
     if (!desc || !desc->name || !desc->task_fn) return;
     if (s_registry_count >= MAX_SERVICES) {
         LOGW(TAG, "Registry full, cannot register %s", desc->name);
         return;
     }
     for (size_t i = 0; i < s_registry_count; ++i) {
         if (strcmp(s_registry[i].desc->name, desc->name) == 0) {
             LOGW(TAG, "Service %s already registered", desc->name);
             return;
         }
     }
     s_registry[s_registry_count++] = (service_entry_t){
         .desc = desc,
         .handle = NULL,
         .state = SERVICE_STATE_DISABLED
     };
     LOGI(TAG, "Registered service: %s", desc->name);
 }
 
 static bool _auto_subscribe(const service_desc_t *desc, TaskHandle_t handle)
 {
     if (desc->role != SERVICE_ROLE_SUBSCRIBER || !desc->topics) return true;
 
     QueueHandle_t q = xQueueCreate(4, sizeof(msg_t));
     if (!q) return false;
 
     for (const msg_topic_t *t = desc->topics; *t != MSG_TOPIC_END; ++t) {
         if (!msg_bus_subscribe(*t, q)) {
             LOGW(TAG, "Cannot subscribe %s to topic %d", desc->name, *t);
         }
     }
 
     // Optional: pass the queue via task notification, global, or directly
     // xTaskNotify(handle, (uint32_t)q, eSetValueWithOverwrite);
 
     return true;
 }
 
 bool service_registry_start(const char *name)
 {
     for (size_t i = 0; i < s_registry_count; ++i) {
         if (strcmp(s_registry[i].desc->name, name) == 0) {
             if (s_registry[i].state == SERVICE_STATE_RUNNING) {
                 return false;
             }
             BaseType_t res = xTaskCreate(
                 s_registry[i].desc->task_fn,
                 s_registry[i].desc->task_name,
                 s_registry[i].desc->stack_size,
                 NULL, // You can pass the queue handle if needed
                 s_registry[i].desc->priority,
                 &s_registry[i].handle);
 
             if (res == pdPASS) {
                 if (!_auto_subscribe(s_registry[i].desc, s_registry[i].handle)) {
                     LOGE(TAG, "Auto-subscribe failed for %s", name);
                     vTaskDelete(s_registry[i].handle);
                     return false;
                 }
 
                 s_registry[i].state = SERVICE_STATE_RUNNING;
                 LOGI(TAG, "Started service: %s", name);
                 return true;
             } else {
                 LOGE(TAG, "Failed to start service: %s", name);
                 return false;
             }
         }
     }
     return false;
 }
 
 bool service_registry_stop(const char *name)
 {
     for (size_t i = 0; i < s_registry_count; ++i) {
         if (strcmp(s_registry[i].desc->name, name) == 0) {
             if (s_registry[i].state != SERVICE_STATE_RUNNING) return false;
             vTaskDelete(s_registry[i].handle);
             s_registry[i].handle = NULL;
             s_registry[i].state = SERVICE_STATE_DISABLED;
             LOGI(TAG, "Stopped service: %s", name);
             return true;
         }
     }
     return false;
 }
 
 bool service_registry_restart(const char *name)
 {
     return service_registry_stop(name) && service_registry_start(name);
 }
 
 void service_registry_start_all(void)
 {
     for (size_t i = 0; i < s_registry_count; ++i) {
         if (s_registry[i].state == SERVICE_STATE_DISABLED) {
             service_registry_start(s_registry[i].desc->name);
         }
     }
 }
 
 bool service_registry_status(const char *name, service_status_t *out_status)
 {
     if (!out_status) return false;
     for (size_t i = 0; i < s_registry_count; ++i) {
         if (strcmp(s_registry[i].desc->name, name) == 0) {
             out_status->state = s_registry[i].state;
             out_status->stack_remaining = s_registry[i].handle ? uxTaskGetStackHighWaterMark(s_registry[i].handle) * sizeof(StackType_t) : 0;
             return true;
         }
     }
     return false;
 }
 
 int32_t service_registry_get_stack_usage(const char *name)
 {
     for (size_t i = 0; i < s_registry_count; ++i) {
         if (strcmp(s_registry[i].desc->name, name) == 0) {
             return uxTaskGetStackHighWaterMark(s_registry[i].handle);
         }
     }
     return -1;
 }
 