// SPDX-License-Identifier: MIT
//
// WebLoggerBackend.cpp — scrollback-buffer log sink read by the browser.
//
#include "monet_core_cpp/WebLoggerBackend.hpp"

#include <cstdio>
#include <cstring>

#include "monet_core_cpp/LockGuard.hpp"

namespace monet {

WebLoggerBackend::WebLoggerBackend() : mutex_(xSemaphoreCreateMutex()) {}

void WebLoggerBackend::write(const char* data, size_t len) {
    if (data == nullptr || len == 0) {
        return;
    }
    // Echo to the console so serial logging keeps working alongside the web view.
    std::fwrite(data, 1, len, stdout);
    std::fflush(stdout);

    LockGuard lock(mutex_);
    if (len >= kCap) {
        // A single write bigger than the buffer: keep only its tail.
        std::memcpy(buf_, data + (len - kCap), kCap);
        len_ = kCap;
        return;
    }
    if (len_ + len > kCap) {
        // Evict just enough oldest bytes to make room (logs are low-rate, so
        // the memmove cost is negligible and the code stays obvious).
        size_t drop = len_ + len - kCap;
        std::memmove(buf_, buf_ + drop, len_ - drop);
        len_ -= drop;
    }
    std::memcpy(buf_ + len_, data, len);
    len_ += len;
}

size_t WebLoggerBackend::snapshot(char* out, size_t cap) {
    if (out == nullptr || cap == 0) {
        return 0;
    }
    LockGuard lock(mutex_);
    size_t n = len_ < (cap - 1) ? len_ : (cap - 1);
    // If clamped by cap, return the most recent n bytes.
    std::memcpy(out, buf_ + (len_ - n), n);
    out[n] = '\0';
    return n;
}

}  // namespace monet
