#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "service/uart_service.h"
#include "monet_hal/uart_hal.h"
#include "monet_core/msg_bus.h"
#include "monet_core/service_registry.h"
#include "utils/log.h"

#define TAG "UART_SERVICE"
#define UART_QUEUE_DEPTH 8

/// Define which topics this service wants to subscribe to
static const msg_topic_t UART_TOPICS[] = {
    EVENT_SENSOR_LIGHT,
    MSG_TOPIC_END
};

/// Service descriptor for automatic registration
static const service_desc_t uart_service_desc = {
    .name       = "uart_service",
    .task_fn    = uart_service_task,
    .task_name  = "uart_tx_task",
    .stack_size = 2048,
    .priority   = 5,
    .role       = SERVICE_ROLE_SUBSCRIBER,
    .topics     = UART_TOPICS
};

const service_desc_t* get_uart_service(void)
{
    return &uart_service_desc;
}

/**
 * @brief Task to forward subscribed sensor messages to UART.
 *
 * The message queue is passed from service registry via task parameter.
 */
void uart_service_task(void *arg)
{
    QueueHandle_t q = (QueueHandle_t)arg;
    if (q == NULL) {
        LOGE(TAG, "No subscription queue passed");
        vTaskDelete(NULL);
        return;
    }

    my_uart_hal_init();  // Ensure UART is ready

    msg_t msg;
    while (1) {
        if (xQueueReceive(q, &msg, portMAX_DELAY)) {
            char buf[64];
            switch (msg.topic) {
                case EVENT_SENSOR_LIGHT:
                    snprintf(buf, sizeof(buf), "Light ADC: %" PRId32, msg.data.value_int);
                    break;
                default:
                    snprintf(buf, sizeof(buf), "Unknown msg");
                    break;
            }
            my_uart_hal_write_string(buf);
        }
    }
}

/**
 * @brief Separate task to receive UART input and publish to msg_bus.
 */
static void uart_receive_task(void *arg)
{
    QueueHandle_t rx = my_uart_hal_get_rx_queue();
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

void uart_service_start(void)
{
    // Only for manual setup (e.g., receive-side or fallback use)
    my_uart_hal_init();
    xTaskCreate(uart_receive_task, "uart_receive_task", 2048, NULL, 5, NULL);
    LOGI(TAG, "UART receive-side service started");
}
