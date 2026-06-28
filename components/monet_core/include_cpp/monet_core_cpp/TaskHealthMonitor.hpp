// SPDX-License-Identifier: MIT
//
// TaskHealthMonitor.hpp — a software watchdog with an ESP-IDF TWDT backstop.
//
// The safety-product angle: a task that silently stops making progress (stuck
// in a loop, starved, deadlocked on a peripheral) is worse than one that
// crashes, because nothing notices. This monitor makes "no progress" an
// observable, actionable event.
//
// Two layers, on purpose:
//   * Soft layer (this class): each registered task calls kick(id) once per
//     healthy loop. A supervisor periodically calls check(); any task whose
//     last kick is older than its deadline is reported by name and a
//     configurable fail-safe runs (log / mark-unhealthy / reset). This layer
//     knows *which* task missed and can react gently.
//   * Hard layer (ESP-IDF Task Watchdog): kick(id) also feeds the TWDT for the
//     calling task. If a task hangs so hard it never reaches check() either,
//     the hardware-backed TWDT still resets the chip. Belt and suspenders.
//
// Testability: time comes from an injected LogClockFn-style `now_fn`, so a unit
// test can march the clock past a deadline deterministically without sleeping,
// and the esp_task_wdt_* calls are isolated behind a single guard so the pure
// deadline logic compiles and runs on the host.
//
#ifndef MONET_CORE_CPP_TASKHEALTHMONITOR_HPP
#define MONET_CORE_CPP_TASKHEALTHMONITOR_HPP

#include <cstddef>
#include <cstdint>

namespace monet {

// Millisecond time source seam (matches LoggerService's clock seam).
using HealthClockFn = uint32_t (*)();

// What to do when a registered task misses its deadline.
enum class FailSafe : uint8_t {
    Log,            // report the offender and keep running (default)
    MarkUnhealthy,  // report + flag the entry unhealthy for callers to inspect
    Reset,          // report + esp_restart() the chip
};

inline constexpr size_t kMaxMonitoredTasks = 8;

class TaskHealthMonitor {
public:
    // Inject the clock (defaults to esp_timer-based ms on target). The default
    // fail-safe is Log — the safe choice for a demo that must not reboot boards.
    explicit TaskHealthMonitor(HealthClockFn now = nullptr,
                               FailSafe on_miss = FailSafe::Log);

    // Register a task with a progress deadline (ms). `use_wdt` subscribes the
    // *calling* task to the ESP-IDF TWDT (call from that task's context).
    // Returns a non-negative id, or -1 if the table is full.
    int register_task(const char* name, uint32_t deadline_ms, bool use_wdt = true);

    // Record progress for task `id`: refresh its timestamp and, if it opted in,
    // reset the TWDT for the calling task. Call once per healthy loop iteration.
    void kick(int id);

    // Supervisor pass: report+fail-safe any task past its deadline. Call
    // periodically from a supervisor task (or directly in a test).
    void check();

    // Inspection seams (used by tests and callers).
    bool healthy(int id) const;            // false once a deadline was missed
    uint32_t missed_count() const { return missed_count_; }
    void set_failsafe(FailSafe fs) { on_miss_ = fs; }

private:
    struct Entry {
        const char* name = nullptr;
        uint32_t    deadline_ms = 0;
        uint32_t    last_kick_ms = 0;
        bool        in_use = false;
        bool        use_wdt = false;
        bool        healthy = true;
    };

    void on_miss(Entry& e);  // report + run the configured fail-safe

    // Concurrency model (why there is no mutex):
    //   * After registration, each slot has exactly ONE writer — the task that
    //     owns that id calls kick() on it. The supervisor only READS slots in
    //     check(). last_kick_ms/healthy are 32-bit aligned scalars, whose
    //     load/store is atomic on Xtensa, so check() can never see a torn value;
    //     at worst it reads a kick that is one cycle stale, which is harmless.
    //   * register_task() itself is NOT synchronized (its scan-then-claim of a
    //     free slot is a TOCTOU). So register from a single context BEFORE the
    //     monitored tasks run concurrently — which is exactly how the demo wires
    //     it. A multi-writer registration path would need a short critical section.
    HealthClockFn now_;
    FailSafe      on_miss_;
    Entry         entries_[kMaxMonitoredTasks];
    uint32_t      missed_count_ = 0;
};

}  // namespace monet

#endif  // MONET_CORE_CPP_TASKHEALTHMONITOR_HPP
