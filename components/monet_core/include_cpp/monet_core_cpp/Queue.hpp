// SPDX-License-Identifier: MIT
//
// Queue.hpp — type-safe, fixed-capacity wrapper around a FreeRTOS queue.
//
// C pattern (what we replace):
//     QueueHandle_t q = xQueueCreate(8, sizeof(msg_t));   // size by hand
//     xQueueSend(q, &msg, 0);                             // void*, no check
//     msg_t out; xQueueReceive(q, &out, portMAX_DELAY);   // sizeof must match
//
// The `sizeof(T)` is written by the caller, so a queue created for one type
// and used with another compiles fine and corrupts memory at runtime.
//
// C++ pattern:
//     monet::Queue<msg_t, 8> q;     // element type + depth in the type
//     q.send(msg);                  // only a msg_t is accepted
//     msg_t out; q.receive(out, portMAX_DELAY);
//
// The element size is derived from T, so it can never disagree with the
// stored type. Storage is static (no heap) via xQueueCreateStatic.
//
#ifndef MONET_CORE_CPP_QUEUE_HPP
#define MONET_CORE_CPP_QUEUE_HPP

#include <cstddef>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

namespace monet {

template <typename T, size_t N>
class Queue {
    static_assert(N > 0, "Queue depth must be at least 1");

public:
    Queue() {
        // Static allocation: the queue control block and its element
        // storage live inside this object, so there is no malloc.
        handle_ = xQueueCreateStatic(N, sizeof(T), storage_, &queue_buf_);
    }

    ~Queue() {
        if (handle_ != nullptr) {
            vQueueDelete(handle_);
        }
    }

    // A queue owns kernel state — non-copyable, like the C handle.
    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    // Copy `item` into the queue. Returns false if full within `timeout`.
    bool send(const T& item, TickType_t timeout = 0) {
        return xQueueSend(handle_, &item, timeout) == pdTRUE;
    }

    // Send from an ISR. Sets *higher_priority_woken if a context switch
    // should be requested before returning from the ISR.
    bool send_from_isr(const T& item, BaseType_t* higher_priority_woken) {
        return xQueueSendFromISR(handle_, &item, higher_priority_woken) == pdTRUE;
    }

    // Copy one element out into `out`. Returns false on timeout.
    bool receive(T& out, TickType_t timeout = portMAX_DELAY) {
        return xQueueReceive(handle_, &out, timeout) == pdTRUE;
    }

    size_t available() const { return uxQueueMessagesWaiting(handle_); }

    static constexpr size_t capacity() { return N; }

    // Escape hatch so the typed queue can still be handed to existing C
    // APIs (e.g. msg_bus_subscribe) that expect a raw QueueHandle_t.
    QueueHandle_t handle() const { return handle_; }

private:
    QueueHandle_t handle_ = nullptr;
    StaticQueue_t queue_buf_{};
    uint8_t storage_[N * sizeof(T)]{};
};

}  // namespace monet

#endif  // MONET_CORE_CPP_QUEUE_HPP
