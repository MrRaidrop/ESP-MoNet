// components/service/src/uart_service.c
#include "service/uart_service.h"
#include "my_hal/uart_hal.h"
#include "core/msg_bus.h"
#include "utils/log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "UART_SERVICE"
#define UART_QUEUE_DEPTH 8

/**
 * @brief Task to forward light sensor values to UART via msg_bus subscription.
 */
static void uart_send_task(void *arg)
{
    QueueHandle_t q = xQueueCreate(UART_QUEUE_DEPTH, sizeof(msg_t));
    if (!msg_bus_subscribe(EVENT_SENSOR_LIGHT, q)) {
        LOGE(TAG, "Failed to subscribe to light sensor topic");
        vTaskDelete(NULL);
        return;
    }

    msg_t msg;
    while (1) {
        if (xQueueReceive(q, &msg, portMAX_DELAY)) {
            char buf[64];
            snprintf(buf, sizeof(buf), "Light ADC: %" PRId32, msg.data.value_int);
            my_uart_hal_write_string(buf);
        }
    }
}

/**
 * @brief Task to receive UART input and publish as msg_bus EVENT_SENSOR_UART.
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
            LOGI(TAG, "[RX] %s → published", line);
            free(line);
        }
    }
}

void uart_service_start(void)
{
    my_uart_hal_init();
    xTaskCreate(uart_send_task, "uart_send_task", 2048, NULL, 5, NULL);
    xTaskCreate(uart_receive_task, "uart_receive_task", 2048, NULL, 5, NULL);
    LOGI(TAG, "UART service started");
}
