// SPDX-License-Identifier: MIT
//
// TaskHealthMonitor.cpp — deadline tracking + ESP-IDF Task Watchdog backstop.
//
#include "monet_core_cpp/TaskHealthMonitor.hpp"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "esp_timer.h"

namespace monet {

namespace {

constexpr const char* kTag = "HEALTH";

// Default clock: microsecond timer reduced to milliseconds. Kept in one place
// so the host test can swap it out wholesale via the constructor seam.
uint32_t default_now() {
    return static_cast<uint32_t>(esp_timer_get_time() / 1000);
}

}  // namespace

TaskHealthMonitor::TaskHealthMonitor(HealthClockFn now, FailSafe on_miss)
    : now_(now != nullptr ? now : &default_now), on_miss_(on_miss) {}

int TaskHealthMonitor::register_task(const char* name, uint32_t deadline_ms,
                                     bool use_wdt) {
    for (size_t i = 0; i < kMaxMonitoredTasks; ++i) {
        Entry& e = entries_[i];
        if (e.in_use) {
            continue;
        }
        e.name = name;
        e.deadline_ms = deadline_ms;
        e.last_kick_ms = now_();
        e.in_use = true;
        e.use_wdt = use_wdt;
        e.healthy = true;
        if (use_wdt) {
            // Subscribe the calling task to the TWDT. This assumes the TWDT is
            // already initialized (CONFIG_ESP_TASK_WDT_INIT, on by default). The
            // error is intentionally ignored: an already-added task is harmless,
            // and if the TWDT is disabled entirely the soft deadline layer still
            // works — we just lose the hardware backstop.
            esp_task_wdt_add(nullptr);
        }
        return static_cast<int>(i);
    }
    ESP_LOGE(kTag, "register_task: table full (%u)",
             static_cast<unsigned>(kMaxMonitoredTasks));
    return -1;
}

void TaskHealthMonitor::kick(int id) {
    if (id < 0 || static_cast<size_t>(id) >= kMaxMonitoredTasks) {
        return;
    }
    Entry& e = entries_[id];
    if (!e.in_use) {
        return;
    }
    e.last_kick_ms = now_();
    e.healthy = true;
    if (e.use_wdt) {
        esp_task_wdt_reset();  // feed the hardware-backed backstop
    }
}

void TaskHealthMonitor::check() {
    uint32_t now = now_();
    for (size_t i = 0; i < kMaxMonitoredTasks; ++i) {
        Entry& e = entries_[i];
        if (!e.in_use) {
            continue;
        }
        // Unsigned subtraction is intentional: it wraps correctly across the
        // 32-bit ms rollover, so a stale entry is detected even at wrap time.
        uint32_t since = now - e.last_kick_ms;
        if (since > e.deadline_ms) {
            on_miss(e);
        }
    }
}

void TaskHealthMonitor::on_miss(Entry& e) {
    ++missed_count_;
    e.healthy = false;
    ESP_LOGE(kTag, "task '%s' missed its %lums deadline",
             e.name != nullptr ? e.name : "?",
             static_cast<unsigned long>(e.deadline_ms));
    switch (on_miss_) {
        case FailSafe::Log:
            break;
        case FailSafe::MarkUnhealthy:
            break;  // healthy flag already cleared; callers can inspect it
        case FailSafe::Reset:
            ESP_LOGE(kTag, "fail-safe: restarting");
            esp_restart();
            break;
    }
}

bool TaskHealthMonitor::healthy(int id) const {
    if (id < 0 || static_cast<size_t>(id) >= kMaxMonitoredTasks) {
        return false;
    }
    const Entry& e = entries_[id];
    return e.in_use && e.healthy;
}

}  // namespace monet
