/**
 * @file light_sensor_service.c
 * @brief Periodically samples an ADC‑based photo‑resistor and publishes the
 *        value on the message‑bus – *now with an inline JSON payload* so that
 *        every downstream “sink” (HTTP, BLE, UART …) can forward the data
 *        without re‑encoding.
 *
 *  ▌Key points
 *  ──────────────────────────────────────────────────────────────────────────
 *  • Samples every 500 ms on `CONFIG_LIGHT_SENSOR_CHANNEL`
 *  • Fills **two** fields in `msg_t`
 *      ① `data.value_int`      → raw integer value (fast to inspect on device)
 *      ② `data.json_str.json`  → pre‑formatted JSON, via `json_encode_msg()`
 *  • No dynamic allocation, no heap use – the JSON lives inside `msg_t`
 *  • Service is auto‑registered through `get_light_sensor_service()`
 */

 #include "freertos/FreeRTOS.h"
 #include "freertos/task.h"
 #include "esp_log.h"
 
 #include "monet_core/msg_bus.h"
 #include "monet_core/service_registry.h"
 
 #include "monet_hal/adc_hal.h"
 
 #include "utils/config.h"
 #include "utils/log.h"
 #include "monet_codec/json_encoder.h"   /* <-- for json_encode_msg()        */
 #include "service/light_sensor_service.h"
 
 #define TAG "LIGHT_SENSOR"
 
 #define STACK_SIZE   4096               /* bytes – plenty for printf        */
 #define PRIORITY     5                  /* same as other low‑rate sensors   */
 #define SAMPLE_MS    500
 #define DARK_THRESH  2000               /* arbitrary demo threshold         */
 
 /* ──────────────────────────────────────────
  * Module‑private cached value so other tasks
  * can query the most‑recent light level.
  * ────────────────────────────────────────── */
 static int s_latest_adc = 0;
 
 int light_sensor_get_cached_value(void) { return s_latest_adc; }
 
 /* ──────────────────────────────────────────
  * Main sampling loop
  * ────────────────────────────────────────── */
 static void light_sensor_task(void *arg)
 {
     /* 1. ADC driver init */
     adc_hal_init(CONFIG_LIGHT_SENSOR_CHANNEL);
     LOGI(TAG, "ADC channel %d initialised", CONFIG_LIGHT_SENSOR_CHANNEL);
 
     while (true) {
         /* 2. Read ADC */
         s_latest_adc = adc_hal_read(CONFIG_LIGHT_SENSOR_CHANNEL);
         LOGI(TAG, "ADC raw: %d", s_latest_adc);
 
         /* 3. Prepare message */
         msg_t msg = {
             .topic = EVENT_SENSOR_LIGHT,
             .ts_ms = esp_log_timestamp(),
             .data.value_int = s_latest_adc,   /* numeric field */
             .release = NULL                   /* nothing to free */
         };
 
         /* 3‑A. Encode JSON directly into the msg buffer
          *      The helper fills msg.data.json_str.json and NUL‑terminates.   */
         json_encode_msg(&msg, msg.data.json_str.json,
                         sizeof(msg.data.json_str.json));   /* never fails for <128B */
 
         /* 4. Publish (zero‑copy) */
         msg_bus_publish(&msg);
 
         /* 5. Local threshold demo */
         if (s_latest_adc > DARK_THRESH)
             LOGW(TAG, "Dark / covered");
         else
             LOGI(TAG, "Bright environment");
 
         vTaskDelay(pdMS_TO_TICKS(SAMPLE_MS));
     }
 }
 
 /* ──────────────────────────────────────────
  * Service descriptor for the registry
  * ────────────────────────────────────────── */
 static const service_desc_t light_sensor_desc = {
     .name       = "light_sensor_service",
     .task_fn    = light_sensor_task,
     .task_name  = "light_sensor_task",
     .stack_size = STACK_SIZE,
     .priority   = PRIORITY,
     .role       = SERVICE_ROLE_PUBLISHER,   /* (<‑ optional, for clarity) */
     .topics     = NULL,                     /* publishers don't subscribe */
     .sink_cb    = NULL
 };
 
 /* ──────────────────────────────────────────
  * Accessor – called from app_main() or auto‑registration
  * ────────────────────────────────────────── */
 const service_desc_t* get_light_sensor_service(void)
 {
     return &light_sensor_desc;
 }
 