// uart_handler.c
#include <string.h>
#include <stdlib.h>
#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "utils/log_wrapper.h"
#include "bsp/uart_handler.h"
#include "service/uart_service.h"
#include "service/light_sensor_service.h"

static void uart_light_send_task(void *pvParameters)
{
    ESP_LOGI(TAG, "UART light send task started");
    while (1) {
        int val = light_sensor_get_cached_value();
        char msg[64];
        snprintf(msg, sizeof(msg), "Light ADC: %d", val);
        uart_write_string(msg);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void uart_service_start(void)
{
    uart_init();
    xTaskCreate(uart_light_send_task, "uart_light_send_task", 2048, NULL, 5, NULL);
    ESP_LOGI("uart", "UART service started");
}