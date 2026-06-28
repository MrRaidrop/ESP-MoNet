// SPDX-License-Identifier: MIT
//
// test_cpp_layer.cpp — host-side unit tests for the experimental C++ layer.
//
// Compiled by test/host/run.sh against the real src_cpp sources plus the thin
// FreeRTOS/ESP shims in test/host/shim. Covers the brief's test matrix: log
// routing, backend selection/fallback, drop-on-full counting, LockGuard
// acquire/release, CameraFrame move/release, Queue send/receive/full, and the
// TaskHealthMonitor deadline logic (with an injected fake clock).
//
#include <cstring>
#include <utility>

#include "unity.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "monet_core_cpp/CameraFrame.hpp"
#include "monet_core_cpp/LockGuard.hpp"
#include "monet_core_cpp/LogMessage.hpp"
#include "monet_core_cpp/LoggerBackend.hpp"
#include "monet_core_cpp/LoggerService.hpp"
#include "monet_core_cpp/Queue.hpp"
#include "monet_core_cpp/TaskHealthMonitor.hpp"

// ---- esp_camera shim backing store (declared in shim/esp_camera.h) --------
int g_fb_return_count = 0;
void esp_camera_fb_return(camera_fb_t* fb) {
    (void)fb;
    ++g_fb_return_count;
}

// ---- fake clock for TaskHealthMonitor -------------------------------------
static uint32_t g_now = 0;
static uint32_t fake_now() { return g_now; }

void setUp(void) {}
void tearDown(void) {}

// ---- 1. log routing: enqueue -> drain -> FakeBackend received it ----------
static void test_log_routing(void) {
    monet::FakeLoggerBackend fake;
    monet::LoggerService logger(fake);

    TEST_ASSERT_TRUE(logger.log(monet::LogLevel::Info, "ROUTE", "hello %d", 42));
    TEST_ASSERT_EQUAL_UINT(0, fake.writes());  // async: nothing written yet
    TEST_ASSERT_TRUE(logger.drain_one());       // drain the one entry
    TEST_ASSERT_EQUAL_UINT(1, fake.writes());
    TEST_ASSERT_NOT_NULL(strstr(fake.contents(), "hello 42"));
    TEST_ASSERT_NOT_NULL(strstr(fake.contents(), "ROUTE"));
    TEST_ASSERT_EQUAL_CHAR('I', fake.contents()[0]);  // level prefix

    TEST_ASSERT_FALSE(logger.drain_one());  // queue now empty
}

// ---- 2. backend selection / fallback (runtime swap) -----------------------
static void test_backend_swap(void) {
    monet::FakeLoggerBackend a;
    monet::FakeLoggerBackend b;
    monet::LoggerService logger(a);

    logger.log(monet::LogLevel::Warn, "T", "to-a");
    logger.drain_one();
    TEST_ASSERT_NOT_NULL(strstr(a.contents(), "to-a"));
    TEST_ASSERT_EQUAL_UINT(0, b.writes());

    logger.set_backend(b);
    logger.log(monet::LogLevel::Warn, "T", "to-b");
    logger.drain_one();
    TEST_ASSERT_NOT_NULL(strstr(b.contents(), "to-b"));
    TEST_ASSERT_NULL(strstr(a.contents(), "to-b"));  // old backend untouched
}

// ---- 3. drop-and-count on full --------------------------------------------
static void test_drop_on_full(void) {
    monet::FakeLoggerBackend fake;
    monet::LoggerService logger(fake);

    // Fill the queue to capacity without draining.
    for (size_t i = 0; i < monet::kLogQueueDepth; ++i) {
        TEST_ASSERT_TRUE(logger.log(monet::LogLevel::Info, "FILL", "n=%zu", i));
    }
    TEST_ASSERT_EQUAL_UINT32(0, logger.dropped());

    // The next two must be dropped and counted — producers never block.
    TEST_ASSERT_FALSE(logger.log(monet::LogLevel::Info, "FILL", "overflow1"));
    TEST_ASSERT_FALSE(logger.log(monet::LogLevel::Info, "FILL", "overflow2"));
    TEST_ASSERT_EQUAL_UINT32(2, logger.dropped());
}

// ---- 4. LockGuard acquire / release ---------------------------------------
static void test_lock_guard(void) {
    SemaphoreHandle_t m = xSemaphoreCreateMutex();

    {
        monet::LockGuard g(m);
        TEST_ASSERT_TRUE(g.locked());
        // Re-taking with timeout 0 while held must fail.
        monet::LockGuard g2(m, 0);
        TEST_ASSERT_FALSE(g2.locked());
    }  // g releases here

    // Released: a fresh guard can take it again.
    monet::LockGuard g3(m, 0);
    TEST_ASSERT_TRUE(g3.locked());
}

// ---- 5. CameraFrame move / release ----------------------------------------
static void test_camera_frame_move(void) {
    camera_fb_t fb = {};
    g_fb_return_count = 0;
    {
        monet::CameraFrame f1(&fb);
        TEST_ASSERT_TRUE(f1.valid());
        monet::CameraFrame f2(std::move(f1));      // ownership hand-off
        TEST_ASSERT_FALSE(f1.valid());             // source emptied
        TEST_ASSERT_TRUE(f2.valid());
    }  // exactly one destructor returns the buffer
    TEST_ASSERT_EQUAL_INT(1, g_fb_return_count);
}

static void test_camera_frame_release(void) {
    camera_fb_t fb = {};
    g_fb_return_count = 0;
    {
        monet::CameraFrame f(&fb);
        camera_fb_t* raw = f.release();            // detach: no auto-return
        TEST_ASSERT_EQUAL_PTR(&fb, raw);
        TEST_ASSERT_FALSE(f.valid());
    }
    TEST_ASSERT_EQUAL_INT(0, g_fb_return_count);   // release suppressed return
}

// ---- 6. Queue send / receive / full ---------------------------------------
static void test_queue(void) {
    monet::Queue<int, 4> q;
    TEST_ASSERT_EQUAL_UINT(4, q.capacity());

    for (int i = 0; i < 4; ++i) {
        TEST_ASSERT_TRUE(q.send(i * 10, 0));
    }
    TEST_ASSERT_EQUAL_UINT(4, q.available());
    TEST_ASSERT_FALSE(q.send(999, 0));  // full

    int out = -1;
    TEST_ASSERT_TRUE(q.receive(out, 0));
    TEST_ASSERT_EQUAL_INT(0, out);      // FIFO
    TEST_ASSERT_TRUE(q.receive(out, 0));
    TEST_ASSERT_EQUAL_INT(10, out);
    TEST_ASSERT_EQUAL_UINT(2, q.available());
}

// ---- 7. TaskHealthMonitor deadline logic (fake clock) ---------------------
static void test_health_monitor(void) {
    g_now = 0;
    monet::TaskHealthMonitor health(&fake_now, monet::FailSafe::MarkUnhealthy);
    int id = health.register_task("worker", 100 /*ms*/, /*use_wdt=*/false);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, id);

    health.kick(id);            // checked in at t=0
    g_now = 50;
    health.check();             // within deadline
    TEST_ASSERT_TRUE(health.healthy(id));
    TEST_ASSERT_EQUAL_UINT32(0, health.missed_count());

    g_now = 200;                // 200ms since last kick > 100ms deadline
    health.check();
    TEST_ASSERT_FALSE(health.healthy(id));
    TEST_ASSERT_EQUAL_UINT32(1, health.missed_count());

    health.kick(id);            // recovery
    TEST_ASSERT_TRUE(health.healthy(id));
}

static void test_health_monitor_table_full(void) {
    g_now = 0;
    monet::TaskHealthMonitor health(&fake_now);
    for (size_t i = 0; i < monet::kMaxMonitoredTasks; ++i) {
        TEST_ASSERT_GREATER_OR_EQUAL_INT(0, health.register_task("t", 10, false));
    }
    TEST_ASSERT_EQUAL_INT(-1, health.register_task("overflow", 10, false));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_log_routing);
    RUN_TEST(test_backend_swap);
    RUN_TEST(test_drop_on_full);
    RUN_TEST(test_lock_guard);
    RUN_TEST(test_camera_frame_move);
    RUN_TEST(test_camera_frame_release);
    RUN_TEST(test_queue);
    RUN_TEST(test_health_monitor);
    RUN_TEST(test_health_monitor_table_full);
    return UNITY_END();
}
