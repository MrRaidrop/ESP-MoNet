// SPDX-License-Identifier: MIT
//
// LoggerService.hpp — an asynchronous, backend-pluggable logging service.
//
// What it demonstrates (and why it is shaped this way):
//   * Async via task+queue. Producers call log()/MLOG* which formats a
//     LogMessage and *try-enqueues* it onto a dedicated queue, then returns
//     immediately. A single drain task pops messages and writes them through
//     the active backend, so no producer ever blocks on a slow UART.
//   * Dependency injection. The service is constructed with a LoggerBackend&
//     (and an optional time source) — never a global. Tests inject a
//     FakeLoggerBackend and a deterministic clock; production injects the
//     Kconfig-selected backend.
//   * Back-pressure policy: try-enqueue, drop-and-count on full. A wedged
//     console can cost you log lines but can never stall the system.
//   * Lock only what must be locked. The FreeRTOS queue is already
//     thread-safe, so it is NOT guarded. The active-backend pointer and the
//     dropped counter are genuinely shared mutable state, so they are — via
//     LockGuard. (Deliberately narrow locking; see set_backend()/dropped().)
//
// Why a dedicated queue rather than an EVENT_LOG topic on msg_bus: logs must
// not ride the sensor bus and be discarded by its timeout-0 fan-out policy,
// and the stable C msg_bus.* stays untouched.
//
#ifndef MONET_CORE_CPP_LOGGERSERVICE_HPP
#define MONET_CORE_CPP_LOGGERSERVICE_HPP

#include <cstdarg>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "monet_core_cpp/LogMessage.hpp"
#include "monet_core_cpp/LoggerBackend.hpp"
#include "monet_core_cpp/Queue.hpp"

namespace monet {

// Time source seam: returns a millisecond tick. Defaults to the same source
// the existing C services use (esp_log_timestamp); tests inject a fake clock.
using LogClockFn = uint32_t (*)();

class LoggerService {
public:
    // Inject the backend (and optionally the clock). Stores references/values,
    // creates the guarding mutex; does NOT start the task (call start()).
    explicit LoggerService(LoggerBackend& backend, LogClockFn clock = nullptr);
    ~LoggerService();

    LoggerService(const LoggerService&) = delete;
    LoggerService& operator=(const LoggerService&) = delete;

    // Spawn the drain task. Idempotent: a second call is a no-op.
    bool start();

    // Format and enqueue one entry, non-blocking. Returns false if the queue
    // was full (the entry is dropped and the drop counter incremented).
    bool log(LogLevel level, const char* tag, const char* fmt, ...)
        __attribute__((format(printf, 4, 5)));
    bool vlog(LogLevel level, const char* tag, const char* fmt, va_list ap);

    // Swap the active backend at runtime (e.g. UART -> USB fallback). Guarded.
    void set_backend(LoggerBackend& backend);

    // Number of entries dropped because the queue was full. Guarded.
    uint32_t dropped() const;

    // Test seam: pop and write at most one entry without the drain task.
    // Returns true if an entry was processed within `timeout` ticks.
    bool drain_one(TickType_t timeout = 0);

private:
    static void task_trampoline(void* self);
    void run();                              // drain loop (never returns)
    void write_through_backend(const LogMessage& msg);

    Queue<LogMessage, kLogQueueDepth> queue_;
    LoggerBackend*    backend_;              // guarded by mutex_
    LogClockFn        clock_;
    SemaphoreHandle_t mutex_ = nullptr;      // guards backend_ and dropped_
    uint32_t          dropped_ = 0;          // guarded by mutex_
    TaskHandle_t      task_ = nullptr;
};

// ---- Process-wide façade -------------------------------------------------
// One default service over the Kconfig-selected backend, plus MLOG* macros so
// call sites read like the existing LOGI/LOGW/LOGE but enqueue asynchronously.
LoggerService& default_logger();

}  // namespace monet

#define MLOG(level, tag, ...) ::monet::default_logger().log((level), (tag), __VA_ARGS__)
#define MLOGE(tag, ...) MLOG(::monet::LogLevel::Error, (tag), __VA_ARGS__)
#define MLOGW(tag, ...) MLOG(::monet::LogLevel::Warn, (tag), __VA_ARGS__)
#define MLOGI(tag, ...) MLOG(::monet::LogLevel::Info, (tag), __VA_ARGS__)
#define MLOGD(tag, ...) MLOG(::monet::LogLevel::Debug, (tag), __VA_ARGS__)

#endif  // MONET_CORE_CPP_LOGGERSERVICE_HPP
