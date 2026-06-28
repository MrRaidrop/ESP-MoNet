// SPDX-License-Identifier: MIT
//
// message_bus_capi.h — C API wrappers around the C++ MessageBus.
//
// These let existing C modules talk to the C++ MessageBus object without
// knowing any C++. They are only available when CONFIG_MONET_CPP_EXPERIMENTAL
// is enabled (the .cpp that defines them is only compiled then). The function
// names are deliberately distinct from the production msg_bus_* API so the
// stable C path is never shadowed or replaced.
//
// This header is C-safe: it can be included from both .c and .cpp.
//
#ifndef MONET_CORE_CPP_MESSAGE_BUS_CAPI_H
#define MONET_CORE_CPP_MESSAGE_BUS_CAPI_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <stdbool.h>

#include "monet_core/msg_bus.h"  // msg_t, msg_topic_t

#ifdef __cplusplus
extern "C" {
#endif

// Forwards to monet::MessageBus::instance().publish(*msg).
void monet_cpp_bus_publish(const msg_t* msg);

// Forwards to monet::MessageBus::instance().subscribe(topic, queue).
bool monet_cpp_bus_subscribe(msg_topic_t topic, QueueHandle_t queue);

// Forwards to monet::MessageBus::instance().subscribe_group(group_id, queue).
bool monet_cpp_bus_subscribe_group(uint16_t group_id, QueueHandle_t queue);

#ifdef __cplusplus
}
#endif

#endif  // MONET_CORE_CPP_MESSAGE_BUS_CAPI_H
