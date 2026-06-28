// SPDX-License-Identifier: MIT
//
// WebLoggerBackend.hpp — a LoggerBackend that buffers recent log lines for a
// browser to read.
//
// This is the payoff of the LoggerBackend interface: a brand-new log transport
// (HTTP) drops in by implementing one virtual method, with zero changes to any
// MLOG* call site. The service keeps writing through `LoggerBackend&`; only the
// concrete type changes.
//
// write() appends the formatted line into a fixed scrollback buffer (oldest
// bytes evicted) and also echoes to stdout, so the serial console still works.
// A small HTTP handler (see logger_demo.cpp) serves snapshot() to the browser.
//
#ifndef MONET_CORE_CPP_WEBLOGGERBACKEND_HPP
#define MONET_CORE_CPP_WEBLOGGERBACKEND_HPP

#include <cstddef>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "monet_core_cpp/LoggerBackend.hpp"

namespace monet {

class WebLoggerBackend final : public LoggerBackend {
public:
    WebLoggerBackend();

    // Append `len` bytes to the scrollback (evicting oldest) and echo to stdout.
    void write(const char* data, size_t len) override;
    using LoggerBackend::write;  // keep the const char* overload visible

    // Copy the buffered tail into out[cap], NUL-terminated. Returns bytes
    // written (excluding the NUL). Safe to call from another task (the HTTP
    // handler); guarded by the same mutex as write().
    size_t snapshot(char* out, size_t cap);

    // Scrollback capacity in bytes. Public so a reader (e.g. the HTTP handler)
    // can size its buffer as kCap + 1 and never drift from this value.
    static constexpr size_t kCap = 6144;

private:
    SemaphoreHandle_t mutex_;
    size_t            len_ = 0;     // valid bytes in buf_ (front = oldest)
    char              buf_[kCap];
};

}  // namespace monet

#endif  // MONET_CORE_CPP_WEBLOGGERBACKEND_HPP
