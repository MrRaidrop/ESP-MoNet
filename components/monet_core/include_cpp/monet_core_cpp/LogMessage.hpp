// SPDX-License-Identifier: MIT
//
// LogMessage.hpp — the unit of work carried on the logger queue, plus the
// compile-time configuration for the LoggerService.
//
// Two embedded-C++ ideas live here:
//   * No magic numbers. Queue depth, task stack/priority and buffer sizes are
//     `constexpr`, with `static_assert`s that fail the build (not the boot) if
//     someone picks an unworkable value.
//   * The message is a trivially-copyable POD with fixed-size char arrays, so a
//     copy through the FreeRTOS queue is a plain memcpy — no heap, no pointers
//     into a producer's stack that could dangle once it moves on.
//
#ifndef MONET_CORE_CPP_LOGMESSAGE_HPP
#define MONET_CORE_CPP_LOGMESSAGE_HPP

#include <cstddef>
#include <cstdint>

namespace monet {

// Severity, ordered most→least severe. uint8_t keeps LogMessage compact.
enum class LogLevel : uint8_t {
    Error = 0,
    Warn,
    Info,
    Debug,
};

// ---- Compile-time configuration (deliverable: constexpr, no magic numbers) --
//
// Plain integer types (not FreeRTOS UBaseType_t/TickType_t) so this header
// stays free of freertos/* and can be compiled on the host for unit tests.
inline constexpr size_t   kLogQueueDepth    = 16;    // pending entries before drop
inline constexpr size_t   kMaxLogTextLen    = 128;   // formatted body, incl NUL
inline constexpr size_t   kMaxLogTagLen     = 16;    // subsystem tag, incl NUL
inline constexpr uint32_t kLogTaskStackBytes = 3072; // drain task stack
inline constexpr uint32_t kLogTaskPriority   = 2;    // see priority table below

// Priority rationale (deliverable: intentional, documented priority).
//   camera/mjpeg ~10  time-critical frame pacing
//   sensors        5  periodic producers (existing service_registry default)
//   health monitor 4  must observe even when the logger is backed up
//   LOGGER TASK    2  <-- here. Non-time-critical, so it sits low; but strictly
//                       above idle(0) so it always drains, and strictly below
//                       the producers so logging can never preempt them or
//                       invert priority against a task that just enqueued.
//   idle           0
static_assert(kLogTaskPriority > 0, "logger must outrank idle so it always drains");
static_assert(kLogTaskPriority < 5, "logger must stay below sensor producers (prio 5)");
static_assert(kLogQueueDepth >= 4, "queue too shallow to absorb a normal burst");
static_assert(kMaxLogTextLen >= 32, "text buffer too small to be useful");
static_assert(kMaxLogTagLen >= 4, "tag buffer too small to be useful");

// The queued unit. POD: trivially copyable, fixed storage, no owning pointers.
struct LogMessage {
    LogLevel level;
    uint32_t ts_ms;                  // filled at enqueue time
    char     tag[kMaxLogTagLen];     // NUL-terminated, truncated if longer
    char     text[kMaxLogTextLen];   // NUL-terminated, truncated if longer
};

}  // namespace monet

#endif  // MONET_CORE_CPP_LOGMESSAGE_HPP
