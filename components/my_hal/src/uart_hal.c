// components/hal/src/uart_hal.c
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "my_hal/uart_hal.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#include "utils/log.h"

#define TAG "UART_HAL"
#define UART_PORT_NUM      UART_NUM_1
#define TXD_PIN            (GPIO_NUM_17)
#define RXD_PIN            (GPIO_NUM_16)
#define UART_BUF_SIZE      128

static QueueHandle_t uart_queue = NULL;

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

void my_uart_hal_init(void)
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
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

QueueHandle_t my_uart_hal_get_rx_queue(void)
{
    return uart_queue;
}

void my_uart_hal_write_string(const char *str)
{
    if (str) {
        uart_write_bytes(UART_PORT_NUM, str, strlen(str));
        uart_write_bytes(UART_PORT_NUM, "\r\n", 2);
    }
}
