#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

#include "esp_crc.h" // For CRC32 calculation
#include "monet_core/msg_bus.h"
#include "monet_core/service_registry.h"
#include "monet_hal/camera_hal.h"
#include "monet_hal/uart_hal.h"
#include "service/uart_service.h"
#include "utils/log.h"

#define TAG "UART_SERVICE"


// now the sink handler logic is here
static bool uart_sink_handler(const msg_t *m) {
  switch (m->topic) {

  case EVENT_SENSOR_JPEG: {
    const camera_fb_t *fb = m->data.jpeg.fb;
    uint32_t crc = esp_crc32_le(0, fb->buf, fb->len);
    struct __attribute__((packed)) {
      uint32_t magic;
      uint16_t w, h;
      uint32_t len, crc32;
    } hdr = {0xA5A5A5A5, fb->width, fb->height, fb->len, crc};

    monet_uart_hal_write_bytes((uint8_t *)&hdr, sizeof(hdr));
    monet_uart_hal_write_bytes(fb->buf, fb->len);
    return true;
  }

/** This is the entry of user want to add his own output logic */
/** -------------------------------------------------------------------------- */

  default: {
    const char *json = m->data.json_str.json;

    // if user don't provide a json output then fallback to normal output
    if (json[0] != '\0') {
      monet_uart_hal_write_string(json);
    } else {
      char buf[128];
      switch (m->topic) {
      case EVENT_SENSOR_LIGHT:
        snprintf(buf, sizeof(buf), "[LIGHT] ADC: %" PRId32, m->data.value_int);
        break;
      case EVENT_SENSOR_TEMP:
        snprintf(buf, sizeof(buf), "[TEMP] %.2f°C, %.2f%%",
                 m->data.temp_hum.temperature, m->data.temp_hum.humidity);
        break;
      case EVENT_SENSOR_UART:
        snprintf(buf, sizeof(buf), "[UART] %s", m->data.uart_text.str);
        break;
      default:
        snprintf(buf, sizeof(buf), "[UNKNOWN] topic=%d", m->topic);
        break;
      }
      monet_uart_hal_write_string(buf);
    }

    monet_uart_hal_write_string("\r\n");
    return true;
  }
  }
}

static const msg_topic_t uart_topics[] = {
#if CONFIG_ROUTE_SENSOR_GROUP_UART
    EVENT_GROUP_SENSOR,
#endif
    MSG_TOPIC_END
};

/// Service descriptor for automatic registration
static const service_desc_t uart_service_desc = {
    .name       = "uart_service",
#if CONFIG_UART_SERVICE_ENABLE
    .task_fn    = uart_service_task,
#else
    .task_fn    = NULL,                 // disabled at build-time
#endif
    .role       = SERVICE_ROLE_SUBSCRIBER,
    .topics     = uart_topics,
    .sink_cb    = uart_sink_handler,
    .stack_size = 4096,
    .priority   = 6,
};
/**
 * @brief Task to forward sensor messages to UART.
 *
 * This task receives messages like ADC, Temp, JPEG etc. and prints them over
 * UART. All formatting logic is now handled by `uart_sink_handler()`.
 * If the message is a JPEG frame, it sends a binary header with metadata
 * and CRC.
 */
void uart_service_task(void *arg) {
  QueueHandle_t q = (QueueHandle_t)arg;
  if (q == NULL) {
    LOGE(TAG, "No subscription queue passed to UART task");
    vTaskDelete(NULL);
    return;
  }

  monet_uart_hal_init();

  msg_t msg;
  while (xQueueReceive(q, &msg, portMAX_DELAY)) {
    if (uart_sink_handler(&msg)) {
      if (msg.release)
        msg.release(&msg);
    }
  }
}

/**
 * @brief Task to receive UART input and publish to msg_bus.
 */
static void uart_receive_task(void *arg) {
  QueueHandle_t rx = monet_uart_hal_get_rx_queue();
  char *line;
  while (1) {
    if (xQueueReceive(rx, &line, portMAX_DELAY) == pdTRUE) {
      msg_t msg = {.topic = EVENT_SENSOR_UART, .ts_ms = esp_log_timestamp()};
      strncpy(msg.data.uart_text.str, line, sizeof(msg.data.uart_text.str) - 1);
      msg.data.uart_text.str[sizeof(msg.data.uart_text.str) - 1] = '\0';

      msg_bus_publish(&msg);
      LOGI(TAG, "[RX] %s published", line);
      free(line);
    }
  }
}

// Internal uses only now
void uart_service_start(void) {
  monet_uart_hal_init();
  xTaskCreate(uart_receive_task, "uart_receive_task", 2048, NULL, 5, NULL);
  LOGI(TAG, "UART receive-side task started");
}

const service_desc_t *get_uart_service(void) { return &uart_service_desc; }
