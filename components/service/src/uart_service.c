#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "service/uart_service.h"
#include "monet_hal/uart_hal.h"
#include "monet_hal/camera_hal.h"
#include "monet_core/msg_bus.h"
#include "monet_core/service_registry.h"
#include "utils/log.h"
#include "esp_crc.h"  // For CRC32 calculation

#define TAG "UART_SERVICE"

/// Service descriptor for automatic registration
static const service_desc_t uart_service_desc = {
    .name       = "uart_service",
    .task_fn    = uart_service_task,
    .task_name  = "uart_tx_task",
    .stack_size = 4096,
    //.priority   = 5,
    .priority   = 6,  // Higher priority for test purposes, can be adjusted to 5
    .role       = SERVICE_ROLE_SUBSCRIBER,
    .topics     = (const msg_topic_t[]){ EVENT_GROUP_SENSOR, MSG_TOPIC_END }
};

const service_desc_t* get_uart_service(void)
{
    return &uart_service_desc;
}

/**
 * @brief Task to forward sensor messages to UART.
 *
 * This task receives messages like ADC, Temp, JPEG etc. and prints them over UART.
 * If the message is a JPEG frame, it sends a binary header with metadata and CRC.
 */
void uart_service_task(void *arg)
{
    QueueHandle_t q = (QueueHandle_t)arg;
    if (q == NULL) {
        LOGE(TAG, "No subscription queue passed to UART task");
        vTaskDelete(NULL);
        return;
    }

    monet_uart_hal_init();

    msg_t msg;
    while (1) {
        if (xQueueReceive(q, &msg, portMAX_DELAY)) {
            char buf[64];
            switch (msg.topic) {
                case EVENT_SENSOR_LIGHT:
                    snprintf(buf, sizeof(buf), "Light ADC: %" PRId32 "\r\n", msg.data.value_int);
                    monet_uart_hal_write_string(buf);
                    break;
                case EVENT_SENSOR_TEMP:
                // You can implement yourself here, or your own sensor
                    snprintf(buf, sizeof(buf), "Temp: %.2f C, Hum: %.2f%%\r\n",
                             msg.data.temp_hum.temperature,
                             msg.data.temp_hum.humidity);
                    monet_uart_hal_write_string(buf);
                    break;
                case EVENT_SENSOR_JPEG: {
                    const camera_fb_t *fb = msg.data.jpeg.fb;

                    // Compute CRC32 over the image data
                    uint32_t crc = esp_crc32_le(0, fb->buf, fb->len);

                    // Define packed JPEG frame header for transmission
                    struct __attribute__((packed)) FrameHeader {
                        uint32_t magic;     // 0xA5A5A5A5 = magic start
                        uint16_t width;     // Image width (if available)
                        uint16_t height;    // Image height (if available)
                        uint32_t length;    // JPEG byte size
                        uint32_t crc32;     // CRC32 of payload
                    };

                    struct FrameHeader header = {
                        .magic  = 0xA5A5A5A5,
                        .width  = fb->width,
                        .height = fb->height,
                        .length = fb->len,
                        .crc32  = crc
                    };

                    // Send the header first
                    monet_uart_hal_write_bytes((uint8_t*)&header, sizeof(header));

                    // Then send the actual JPEG data
                    monet_uart_hal_write_bytes(fb->buf, fb->len);

                    LOGI(TAG, "JPEG frame sent: %d bytes, CRC=0x%08X", fb->len, (unsigned int)crc);
                    break;
                }
                default:
                    snprintf(buf, sizeof(buf), "[UART] Unknown topic: %d\r\n", msg.topic);
                    monet_uart_hal_write_string(buf);
                    break;
            }
        }
    }
}

/**
 * @brief Task to receive UART input and publish to msg_bus.
 */
static void uart_receive_task(void *arg)
{
    QueueHandle_t rx = monet_uart_hal_get_rx_queue();
    char *line;
    while (1) {
        if (xQueueReceive(rx, &line, portMAX_DELAY) == pdTRUE) {
            msg_t msg = {
                .topic = EVENT_SENSOR_UART,
                .ts_ms = esp_log_timestamp()
            };
            strncpy(msg.data.uart_text.str, line, sizeof(msg.data.uart_text.str) - 1);
            msg.data.uart_text.str[sizeof(msg.data.uart_text.str) - 1] = '\0';

            msg_bus_publish(&msg);
            LOGI(TAG, "[RX] %s published", line);
            free(line);
        }
    }
}

// Internal uses only now
void uart_service_start(void)
{
    monet_uart_hal_init();
    xTaskCreate(uart_receive_task, "uart_receive_task", 2048, NULL, 5, NULL);
    LOGI(TAG, "UART receive-side task started");
}