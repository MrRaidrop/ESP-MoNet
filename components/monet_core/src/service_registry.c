/**
 * @file service_registry.c
 * @brief Implementation of centralized service registry for managing lifecycle of all system services.
 */

 #include "monet_core/service_registry.h"
 #include <string.h>
 #include <stdlib.h>
 #include "esp_log.h"
 
 #define MAX_SERVICES 16
 #define TAG "SVC_REG"
 
 /**
  * @brief Weakly linked service registry start and stop symbols.
  * These are used to find the start and end of the service registry in memory.
  */
extern const service_desc_t *__start_svc_registry[] __attribute__((weak));
extern const service_desc_t *__stop_svc_registry[] __attribute__((weak));
 
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
         ESP_LOGW(TAG, "Registry full, cannot register %s", desc->name);
         return;
     }
     for (size_t i = 0; i < s_registry_count; ++i) {
         if (strcmp(s_registry[i].desc->name, desc->name) == 0) {
             ESP_LOGW(TAG, "Service %s already registered", desc->name);
             return;
         }
     }
     s_registry[s_registry_count++] = (service_entry_t){
         .desc = desc,
         .handle = NULL,
         .state = SERVICE_STATE_DISABLED
     };
     ESP_LOGI(TAG, "Registered service: %s", desc->name);
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
                 NULL,
                 s_registry[i].desc->priority,
                 &s_registry[i].handle);
             if (res == pdPASS) {
                 s_registry[i].state = SERVICE_STATE_RUNNING;
                 ESP_LOGI(TAG, "Started service: %s", name);
                 return true;
             } else {
                 ESP_LOGE(TAG, "Failed to start service: %s", name);
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
             ESP_LOGI(TAG, "Stopped service: %s", name);
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
 
int32_t service_registry_get_stack_usage(const char *name){
    for (size_t i = 0; i < s_registry_count; ++i) {
        if (strcmp(s_registry[i].desc->name, name) == 0) {
            return uxTaskGetStackHighWaterMark(s_registry[i].handle);
        }
    }
    return -1;
}

static void service_registry_autoload(void)
{
    if (!__start_svc_registry || !__stop_svc_registry) return;

    for (const service_desc_t **p = __start_svc_registry; p < __stop_svc_registry; ++p) {
        service_registry_register(*p);
    }
}

// hock
__attribute__((constructor))
static void service_registry_init_ctor(void)
{
    service_registry_autoload();
}
