// SPDX-License-Identifier: MIT
//
// MessageBus.hpp — the C msg_bus model expressed as a C++ class.
//
// The existing C bus (components/monet_core/src/msg_bus.c) is a set of free
// functions over file-static arrays plus a hand-managed mutex:
//     msg_bus_publish(&msg);
//     msg_bus_subscribe(topic, queue);
//
// C pattern -> C++ pattern mapping:
//     file-static subscriber arrays   ->  private members of one object
//     msg_bus_* free functions        ->  MessageBus methods (implicit `this`)
//     manual xSemaphoreTake/Give      ->  LockGuard (RAII) in every method
//
// This class is a *parallel demonstration*. It does NOT replace the stable
// msg_bus.c used by production services; it mirrors the same semantics so the
// mapping is obvious, and exposes C API wrappers (message_bus_capi.h) so C
// modules can publish/subscribe through the C++ object when the experimental
// config is on.
//
// No heap, no exceptions, no RTTI: storage is fixed-size and lives in the
// singleton.
//
#ifndef MONET_CORE_CPP_MESSAGEBUS_HPP
#define MONET_CORE_CPP_MESSAGEBUS_HPP

#include <cstddef>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "monet_core/msg_bus.h"  // msg_t, msg_topic_t, EVENT_* (the C contract)

namespace monet {

// Reuse the existing C wire types so messages cross the C/C++ boundary
// without any conversion — the bus stays binary-compatible with C modules.
using Message = msg_t;
using Topic = msg_topic_t;

class MessageBus {
public:
    // Mirrors the C limits in msg_bus.c so behaviour matches exactly.
    static constexpr size_t kMaxSubscribersPerTopic = 8;
    static constexpr size_t kMaxGroupSubscribers = 8;

    // Process-wide instance (the C bus is also a singleton-by-file-statics).
    static MessageBus& instance();

    // Copy `msg` to every queue subscribed to its topic and to any matching
    // group subscriber. Non-blocking sends (timeout 0), same as the C bus.
    void publish(const Message& msg);

    // Subscribe `queue` to a specific topic. False if full / invalid.
    bool subscribe(Topic topic, QueueHandle_t queue);

    // Subscribe `queue` to a topic group (e.g. EVENT_GROUP_SENSOR).
    bool subscribe_group(uint16_t group_id, QueueHandle_t queue);

    MessageBus(const MessageBus&) = delete;
    MessageBus& operator=(const MessageBus&) = delete;

private:
    MessageBus();  // creates the mutex once

    static uint16_t group_of(Topic topic);

    SemaphoreHandle_t mutex_;
    QueueHandle_t topic_subs_[EVENT_SENSOR_MAX][kMaxSubscribersPerTopic];
    QueueHandle_t group_subs_[kMaxGroupSubscribers];
    size_t group_count_;
};

}  // namespace monet

#endif  // MONET_CORE_CPP_MESSAGEBUS_HPP
