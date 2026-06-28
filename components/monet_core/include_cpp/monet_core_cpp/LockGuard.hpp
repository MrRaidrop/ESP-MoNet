// SPDX-License-Identifier: MIT
//
// LockGuard.hpp — RAII wrapper around a FreeRTOS mutex.
//
// C pattern (what we replace):
//     xSemaphoreTake(m, portMAX_DELAY);
//     ... // every early `return` must remember to give it back
//     xSemaphoreGive(m);
//
// C++ pattern:
//     {
//         LockGuard lock(m);   // take in constructor
//         ...                  // any return / break gives it back
//     }                        // give in destructor — cannot be forgotten
//
#ifndef MONET_CORE_CPP_LOCKGUARD_HPP
#define MONET_CORE_CPP_LOCKGUARD_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace monet {

// Scope-bound ownership of a FreeRTOS mutex. No heap, no exceptions.
class LockGuard {
public:
    // Take the mutex on construction. timeout defaults to "wait forever",
    // matching the behaviour of the existing C msg_bus code.
    explicit LockGuard(SemaphoreHandle_t mutex,
                       TickType_t timeout = portMAX_DELAY)
        : mutex_(mutex),
          held_(mutex != nullptr &&
                xSemaphoreTake(mutex, timeout) == pdTRUE) {}

    // Give the mutex back on scope exit — the whole point of RAII.
    ~LockGuard() {
        if (held_) {
            xSemaphoreGive(mutex_);
        }
    }

    // Did we actually acquire the lock? (false only on timeout / null mutex)
    bool locked() const { return held_; }

    // A lock guard owns a unique resource — copying makes no sense.
    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;

private:
    SemaphoreHandle_t mutex_;
    bool held_;
};

}  // namespace monet

#endif  // MONET_CORE_CPP_LOCKGUARD_HPP
