// SPDX-License-Identifier: MIT
//
// MessageBus.cpp — C++ MessageBus implementation plus C API wrappers.
//
// Mirrors the semantics of src/msg_bus.c (same fan-out, same non-blocking
// sends, same group rule) but expressed as one object whose mutex is taken
// through LockGuard, so no method can leak the lock on an early return.
//
#include "monet_core_cpp/MessageBus.hpp"

#include "monet_core_cpp/LockGuard.hpp"
#include "monet_core_cpp/message_bus_capi.h"

#include <cstring>

namespace monet {

MessageBus::MessageBus() : mutex_(xSemaphoreCreateMutex()), group_count_(0) {
    std::memset(topic_subs_, 0, sizeof(topic_subs_));
    std::memset(group_subs_, 0, sizeof(group_subs_));
}

MessageBus& MessageBus::instance() {
    static MessageBus bus;  // constructed once, lives for the program
    return bus;
}

uint16_t MessageBus::group_of(Topic topic) {
    // Same classification as get_group_id() in msg_bus.c.
    if (topic < EVENT_SENSOR_MAX) {
        return EVENT_GROUP_SENSOR;
    }
    return 0xFFFF;
}

void MessageBus::publish(const Message& msg) {
    LockGuard lock(mutex_);  // released no matter which branch returns

    if (msg.topic < EVENT_SENSOR_MAX) {
        for (size_t i = 0; i < kMaxSubscribersPerTopic; ++i) {
            if (topic_subs_[msg.topic][i] != nullptr) {
                xQueueSend(topic_subs_[msg.topic][i], &msg, 0);
            }
        }
    }

    if (group_of(msg.topic) == EVENT_GROUP_SENSOR) {
        for (size_t i = 0; i < group_count_; ++i) {
            if (group_subs_[i] != nullptr) {
                xQueueSend(group_subs_[i], &msg, 0);
            }
        }
    }
}

bool MessageBus::subscribe(Topic topic, QueueHandle_t queue) {
    if (queue == nullptr || topic >= EVENT_SENSOR_MAX) {
        return false;
    }

    LockGuard lock(mutex_);
    for (size_t i = 0; i < kMaxSubscribersPerTopic; ++i) {
        if (topic_subs_[topic][i] == nullptr) {
            topic_subs_[topic][i] = queue;
            return true;
        }
    }
    return false;  // topic full
}

bool MessageBus::subscribe_group(uint16_t group_id, QueueHandle_t queue) {
    if (group_id != EVENT_GROUP_SENSOR || queue == nullptr) {
        return false;
    }

    LockGuard lock(mutex_);
    if (group_count_ >= kMaxGroupSubscribers) {
        return false;
    }
    group_subs_[group_count_++] = queue;
    return true;
}

}  // namespace monet

// ---- C API wrappers: let plain C modules use the C++ bus ----------------

extern "C" void monet_cpp_bus_publish(const msg_t* msg) {
    if (msg != nullptr) {
        monet::MessageBus::instance().publish(*msg);
    }
}

extern "C" bool monet_cpp_bus_subscribe(msg_topic_t topic, QueueHandle_t queue) {
    return monet::MessageBus::instance().subscribe(topic, queue);
}

extern "C" bool monet_cpp_bus_subscribe_group(uint16_t group_id,
                                              QueueHandle_t queue) {
    return monet::MessageBus::instance().subscribe_group(group_id, queue);
}
