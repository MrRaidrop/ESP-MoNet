// components/hal/src/uart_hal.c
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "monet_hal/uart_hal.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#include "utils/log.h"

#define TAG "UART_HAL"

#define TXD_PIN               (GPIO_NUM_47) // freenove ESP32-S3 WROOM DevKit setup
#define RXD_PIN               (GPIO_NUM_21) // not too much pin out available this board

#define UART_PORT_NUM         UART_NUM_1
// #define TXD_PIN            (GPIO_NUM_17) // last version
// #define RXD_PIN            (GPIO_NUM_16)
#define UART_BUF_SIZE         128           // for RX buffer size

static QueueHandle_t uart_queue = NULL;


// internal function to handle UART RX task
static void uart_rx_task(void *pvParameters)
{
    uint8_t data[UART_BUF_SIZE];
    while (1) {
        int len = uart_read_bytes(UART_PORT_NUM, data, UART_BUF_SIZE - 1, pdMS_TO_TICKS(1000));
        if (len > 0) {
            data[len] = '\0';
            LOGI(TAG, "Received: %s", data);

            char *copy = strdup((char *)data);
            if (copy) {
                xQueueSend(uart_queue, &copy, portMAX_DELAY);
            }
        }
    }
}

// UART initialization function, rx task creation for test uses
void monet_uart_hal_init(void)
{
    uart_config_t uart_config = {
        .baud_rate = 921600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_driver_install(UART_PORT_NUM, 1024 * 2, 0, 0, NULL, 0);
    uart_param_config(UART_PORT_NUM, &uart_config);
    uart_set_pin(UART_PORT_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_queue = xQueueCreate(5, sizeof(char *));
    xTaskCreate(uart_rx_task, "uart_rx_task", 4096, NULL, 10, NULL);
}

QueueHandle_t monet_uart_hal_get_rx_queue(void)
{
    return uart_queue;
}

void monet_uart_hal_write_string(const char *str)
{
    if (str) {
        uart_write_bytes(UART_PORT_NUM, str, strlen(str));
        uart_write_bytes(UART_PORT_NUM, "\r\n", 2);
    }
}

void monet_uart_hal_write_bytes(const uint8_t *data, size_t length)
{
    if (data && length > 0) {
        uart_write_bytes(UART_PORT_NUM, (const char *)data, length);
        LOGI(TAG, "Sent %zu bytes", length);
    }
}