// SPDX-License-Identifier: MIT
//
// button_isr_demo.cpp — the canonical embedded interrupt pattern:
//
//     hardware IRQ  ->  tiny ISR  ->  queue  ->  worker task  ->  real work
//
// Why this shape (interview talking points):
//   * The ISR must be SHORT and ISR-SAFE: no logging, no malloc, no blocking
//     calls. It only timestamps the event and hands it to a queue, then asks
//     the scheduler to wake the waiting task (portYIELD_FROM_ISR).
//   * The TASK does the heavy/loggable work, at a normal priority, where
//     blocking and ESP_LOGx are fine. This is "deferred interrupt processing".
//   * Queue (not a semaphore) because we carry DATA (the timestamp). A binary
//     semaphore would only signal "something happened" with no payload.
//   * Debounce lives in the task, keeping the ISR minimal.
//
// It reuses monet::Queue<T,N>::send_from_isr(), so the same type-safe C++
// wrapper works in both thread and interrupt context.
//
#include "monet_core_cpp/button_isr_demo.h"

#include "monet_core_cpp/Queue.hpp"

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace {

constexpr const char* kTag        = "BTN_ISR";
constexpr gpio_num_t  kButtonGpio = GPIO_NUM_0;     // BOOT button on this board
constexpr int64_t     kDebounceUs = 200 * 1000;     // 200 ms

// Payload carried from ISR to task. Small and trivially-copyable so it fits a
// queue slot by value.
struct ButtonEvent {
    int64_t ts_us;
};

// File-static so the queue's lifetime spans setup + ISR + task, and so the ISR
// (a free function) can reach it without a context pointer.
monet::Queue<ButtonEvent, 8> g_queue;

// --- INTERRUPT CONTEXT ---------------------------------------------------
// Must be in IRAM (may run while flash cache is disabled) and must do the
// absolute minimum. No ESP_LOG here — it would crash or block.
void IRAM_ATTR button_isr(void* /*arg*/) {
    BaseType_t higher_priority_woken = pdFALSE;
    ButtonEvent ev{esp_timer_get_time()};
    g_queue.send_from_isr(ev, &higher_priority_woken);
    // If sending unblocked a higher-priority task, switch to it on ISR exit
    // instead of waiting for the next tick — lower latency.
    if (higher_priority_woken) {
        portYIELD_FROM_ISR();
    }
}

// --- TASK CONTEXT --------------------------------------------------------
// Blocks on the queue (0% CPU while idle) and does the real work per press.
void button_task(void* /*arg*/) {
    ESP_LOGI(kTag, "ready — press the BOOT button (GPIO%d)", kButtonGpio);
    ButtonEvent ev{};
    int64_t  last_us = 0;
    uint32_t presses = 0;
    while (true) {
        if (!g_queue.receive(ev, portMAX_DELAY)) {
            continue;
        }
        if (ev.ts_us - last_us < kDebounceUs) {
            continue;  // bounce / repeat within debounce window — ignore
        }
        last_us = ev.ts_us;
        ESP_LOGI(kTag, "BOOT press #%lu  (deferred from ISR, t=%lld ms)",
                 static_cast<unsigned long>(++presses),
                 static_cast<long long>(ev.ts_us / 1000));
    }
}

}  // namespace

extern "C" void monet_button_isr_demo_start(void) {
    gpio_config_t io = {};
    io.pin_bit_mask = 1ULL << kButtonGpio;
    io.mode         = GPIO_MODE_INPUT;
    io.pull_up_en   = GPIO_PULLUP_ENABLE;   // idles high; a press pulls it low
    io.intr_type    = GPIO_INTR_NEGEDGE;    // fire on the press edge
    gpio_config(&io);

    // Worker task first, so it is ready before interrupts can arrive.
    xTaskCreate(button_task, "btn_isr_task", 3072, nullptr, 10, nullptr);

    // Per-pin ISR dispatch: install the shared service, then add our handler.
    gpio_install_isr_service(0);
    gpio_isr_handler_add(kButtonGpio, button_isr, nullptr);
    ESP_LOGI(kTag, "ISR installed on GPIO%d (BOOT button)", kButtonGpio);
}
