/**
 * @file uart_hal.c
 * @brief Thin HAL wrapper around ESP-IDF UART driver
 */
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "utils/log.h"
#include "monet_hal/uart_hal.h"

#define TAG "UART_HAL"
#define UART_RX_TASK_PRIO   10
#define UART_RX_TASK_STACK  4096
#define UART_BUF_SIZE       128

static QueueHandle_t uart_queue = NULL;

/* ────────────────────────────────
 *  Helpers – validate user config
 * ──────────────────────────────── */
static bool gpio_valid(int gpio)
{
    return (gpio >= 0 && gpio <= 48);               /* ESP32-S3 max 48 */
}

static uart_port_t resolve_uart_port(int port_cfg)
{
    switch (port_cfg) {
        case 0: return UART_NUM_0;
        case 1: return UART_NUM_1;
        case 2: return UART_NUM_2;
        default:
            LOGE(TAG, "Invalid UART port %d (expect 0-2)", port_cfg);
            return UART_NUM_MAX;
    }
}


static gpio_num_t resolve_gpio_num(int gpio_val)
{
    if (gpio_val < 0 || gpio_val > 48) {
        LOGE(TAG, "Invalid GPIO number: %d", gpio_val);
        return GPIO_NUM_NC;  // Not Connected
    }

    return (gpio_num_t)gpio_val;  // enum GPIO_NUM_0 == 0 ... GPIO_NUM_48 == 48
}

/* ────────────────────────────────
 *  RX Task
 * ──────────────────────────────── */
static void uart_rx_task(void *pvParameters)
{
    uint8_t data[UART_BUF_SIZE];

    while (true) {
        int len = uart_read_bytes(resolve_uart_port(CONFIG_UART_PORT_NUM),
                                  data, UART_BUF_SIZE - 1,
                                  pdMS_TO_TICKS(1000));
        if (len > 0) {
            data[len] = '\0';
            LOGI(TAG, "RX: %s", data);

            char *copy = strdup((char *)data);
            if (copy) {
                xQueueSend(uart_queue, &copy, portMAX_DELAY);
            }
        }
    }
}

/* ────────────────────────────────
 *  Public API
 * ──────────────────────────────── */
void monet_uart_hal_init(void)
{
    uart_port_t port = resolve_uart_port(CONFIG_UART_PORT_NUM);
    if (port == UART_NUM_MAX) { return; }

    if (!gpio_valid(CONFIG_UART_TX_PIN) || !gpio_valid(CONFIG_UART_RX_PIN)) {
        LOGE(TAG, "Invalid GPIO pin(s): TX=%d RX=%d",
             CONFIG_UART_TX_PIN, CONFIG_UART_RX_PIN);
        return;
    }

    uart_config_t cfg = {
        .baud_rate  = CONFIG_UART_BAUD_RATE,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
    };

    uart_driver_install(port, 1024 * 2, 0, 0, NULL, 0);
    uart_param_config(port, &cfg);
    gpio_num_t tx = resolve_gpio_num(CONFIG_UART_TX_PIN);
    gpio_num_t rx = resolve_gpio_num(CONFIG_UART_RX_PIN);
    if (tx == GPIO_NUM_NC || rx == GPIO_NUM_NC) {
        LOGE(TAG, "Aborting UART init due to invalid GPIOs");
        return;
    }
    uart_set_pin(port, tx, rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_queue = xQueueCreate(5, sizeof(char *));
    xTaskCreate(uart_rx_task, "uart_rx_task",
                UART_RX_TASK_STACK, NULL,
                UART_RX_TASK_PRIO, NULL);

    LOGI(TAG, "UART%d @ %d bps, TX=%d RX=%d",
         port, CONFIG_UART_BAUD_RATE,
         CONFIG_UART_TX_PIN, CONFIG_UART_RX_PIN);
}

QueueHandle_t monet_uart_hal_get_rx_queue(void)
{
    return uart_queue;
}

void monet_uart_hal_write_string(const char *str)
{
    if (str) {
        uart_write_bytes(resolve_uart_port(CONFIG_UART_PORT_NUM), str, strlen(str));
        uart_write_bytes(resolve_uart_port(CONFIG_UART_PORT_NUM), "\r\n", 2);
    }
}

void monet_uart_hal_write_bytes(const uint8_t *data, size_t len)
{
    if (data && len) {
        uart_write_bytes(resolve_uart_port(CONFIG_UART_PORT_NUM),
                         (const char *)data, len);
        LOGI(TAG, "TX %zu bytes", len);
    }
}
