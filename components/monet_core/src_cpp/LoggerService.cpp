// SPDX-License-Identifier: MIT
//
// LoggerService.cpp — async drain task, formatting, and the default façade.
//
#include "monet_core_cpp/LoggerService.hpp"

#include <cstdio>
#include <cstring>

#include "esp_log.h"

#include "monet_core_cpp/LockGuard.hpp"

namespace monet {

namespace {

// Default clock: the same millisecond source the existing C services stamp
// messages with, so logger timestamps line up with msg_bus traffic.
uint32_t default_clock() { return esp_log_timestamp(); }

// One char prefix per level so a flat console still shows severity.
char level_char(LogLevel level) {
    switch (level) {
        case LogLevel::Error: return 'E';
        case LogLevel::Warn:  return 'W';
        case LogLevel::Info:  return 'I';
        case LogLevel::Debug: return 'D';
    }
    return '?';
}

// Copy `src` into `dst[cap]` (fixed array), always NUL-terminating.
void copy_truncated(char* dst, size_t cap, const char* src) {
    if (cap == 0) {
        return;
    }
    if (src == nullptr) {
        dst[0] = '\0';
        return;
    }
    std::strncpy(dst, src, cap - 1);
    dst[cap - 1] = '\0';
}

}  // namespace

LoggerService::LoggerService(LoggerBackend& backend, LogClockFn clock)
    : backend_(&backend),
      clock_(clock != nullptr ? clock : &default_clock),
      mutex_(xSemaphoreCreateMutex()) {}

LoggerService::~LoggerService() {
    if (task_ != nullptr) {
        vTaskDelete(task_);
    }
    if (mutex_ != nullptr) {
        vSemaphoreDelete(mutex_);
    }
}

bool LoggerService::start() {
    if (task_ != nullptr) {
        return true;  // already running — idempotent
    }
    BaseType_t ok = xTaskCreate(&LoggerService::task_trampoline, "logger",
                                kLogTaskStackBytes, this,
                                static_cast<UBaseType_t>(kLogTaskPriority),
                                &task_);
    return ok == pdPASS;
}

bool LoggerService::vlog(LogLevel level, const char* tag, const char* fmt,
                         va_list ap) {
    LogMessage msg;
    msg.level = level;
    msg.ts_ms = clock_();
    copy_truncated(msg.tag, kMaxLogTagLen, tag);
    // vsnprintf always NUL-terminates and never overflows the fixed buffer.
    std::vsnprintf(msg.text, kMaxLogTextLen, fmt != nullptr ? fmt : "", ap);

    // Try-enqueue (timeout 0). On full: drop and count, never block a producer.
    if (queue_.send(msg, 0)) {
        return true;
    }
    LockGuard lock(mutex_);
    ++dropped_;
    return false;
}

bool LoggerService::log(LogLevel level, const char* tag, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    bool ok = vlog(level, tag, fmt, ap);
    va_end(ap);
    return ok;
}

void LoggerService::set_backend(LoggerBackend& backend) {
    LockGuard lock(mutex_);
    backend_ = &backend;
}

uint32_t LoggerService::dropped() const {
    LockGuard lock(mutex_);
    return dropped_;
}

void LoggerService::write_through_backend(const LogMessage& msg) {
    // Snapshot the backend pointer under the lock, then release before the
    // (potentially slow) write — we guard the pointer, not the I/O.
    LoggerBackend* backend;
    {
        LockGuard lock(mutex_);
        backend = backend_;
    }
    if (backend == nullptr) {
        return;
    }
    char line[kMaxLogTagLen + kMaxLogTextLen + 32];
    int n = std::snprintf(line, sizeof(line), "%c (%lu) %s: %s\n",
                          level_char(msg.level),
                          static_cast<unsigned long>(msg.ts_ms), msg.tag,
                          msg.text);
    if (n > 0) {
        backend->write(line, static_cast<size_t>(n) < sizeof(line)
                                 ? static_cast<size_t>(n)
                                 : sizeof(line) - 1);
    }
}

bool LoggerService::drain_one(TickType_t timeout) {
    LogMessage msg;
    if (!queue_.receive(msg, timeout)) {
        return false;
    }
    write_through_backend(msg);
    return true;
}

void LoggerService::run() {
    for (;;) {
        drain_one(portMAX_DELAY);
    }
}

void LoggerService::task_trampoline(void* self) {
    static_cast<LoggerService*>(self)->run();
}

LoggerService& default_logger() {
    // Constructed on first use over the build-time-selected backend; the
    // backing FreeRTOS objects live for the program lifetime.
    static LoggerService instance(logger_default_backend());
    return instance;
}

}  // namespace monet
