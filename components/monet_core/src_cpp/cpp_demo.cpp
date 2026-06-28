// SPDX-License-Identifier: MIT
//
// cpp_demo.cpp — one self-contained run exercising every C++ abstraction.
//
// Called from app_main() (plain C) via monet_cpp_demo_run() when
// CONFIG_MONET_CPP_EXPERIMENTAL is enabled. It allocates nothing on the heap
// and touches no real hardware, so it is safe to run on any board.
//
#include "monet_core_cpp/cpp_demo.h"

#include <utility>  // std::move

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "monet_core_cpp/CameraFrame.hpp"
#include "monet_core_cpp/LockGuard.hpp"
#include "monet_core_cpp/LoggerBackend.hpp"
#include "monet_core_cpp/MessageBus.hpp"
#include "monet_core_cpp/Queue.hpp"
#include "monet_core_cpp/message_bus_capi.h"

#include "monet_core/msg_bus.h"

namespace {

constexpr const char* kTag = "CPP_DEMO";

// 1) RAII LockGuard ------------------------------------------------------
void demo_lock_guard() {
    static SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    {
        monet::LockGuard lock(mutex);  // taken here
        ESP_LOGI(kTag, "LockGuard: acquired=%d (released on scope exit)",
                 lock.locked());
    }  // released here, even if we had returned early above
    ESP_LOGI(kTag, "LockGuard: mutex free again");
}

// 2) Type-safe Queue<T, N> ----------------------------------------------
void demo_queue() {
    monet::Queue<int, 4> q;  // element type + depth baked into the type
    for (int i = 0; i < 3; ++i) {
        q.send(i * 10);  // only an int is accepted; size can't be wrong
    }
    ESP_LOGI(kTag, "Queue<int,4>: cap=%u waiting=%u",
             static_cast<unsigned>(q.capacity()),
             static_cast<unsigned>(q.available()));
    int out = 0;
    while (q.receive(out, 0)) {
        ESP_LOGI(kTag, "Queue<int,4>: received %d", out);
    }
}

// 3) RAII CameraFrame ownership -----------------------------------------
void demo_camera_frame() {
    // A real pipeline would write `CameraFrame frame(esp_camera_fb_get());`
    // and the destructor would call esp_camera_fb_return() automatically.
    // Here we use a stack stand-in and release() it before scope exit so the
    // driver's return is never invoked on a non-driver buffer.
    static camera_fb_t fake_fb = {};

    monet::CameraFrame f1(&fake_fb);
    ESP_LOGI(kTag, "CameraFrame: f1.valid=%d", f1.valid());

    monet::CameraFrame f2(std::move(f1));  // zero-copy ownership hand-off
    ESP_LOGI(kTag, "CameraFrame: after move f1.valid=%d f2.valid=%d",
             f1.valid(), f2.valid());

    camera_fb_t* taken = f2.release();  // detach so dtor won't return it
    ESP_LOGI(kTag, "CameraFrame: released ptr non-null=%d, f2.valid=%d",
             taken != nullptr, f2.valid());
}

// 4) Logger backend interface -------------------------------------------
void demo_logger_backend() {
    monet::LoggerBackend& backend = monet::logger_default_backend();
    backend.write("[CPP_DEMO] LoggerBackend: hello via virtual write()\n");
    ESP_LOGI(kTag, "LoggerBackend: dispatched through the interface");
}

// 5) MessageBus class + C API bridge ------------------------------------
void demo_message_bus() {
    monet::Queue<msg_t, 4> sub_q;
    monet::MessageBus::instance().subscribe(EVENT_SENSOR_LIGHT, sub_q.handle());

    msg_t m = {};
    m.topic = EVENT_SENSOR_LIGHT;
    m.ts_ms = 0;
    m.data.value_int = 1234;
    m.release = nullptr;

    // Publish through the C API wrapper to prove the C<->C++ bridge works.
    monet_cpp_bus_publish(&m);

    msg_t out = {};
    if (sub_q.receive(out, 0)) {
        ESP_LOGI(kTag, "MessageBus: delivered value_int=%ld via C API bridge",
                 static_cast<long>(out.data.value_int));
    } else {
        ESP_LOGW(kTag, "MessageBus: no message delivered");
    }
}

}  // namespace

extern "C" void monet_cpp_demo_run(void) {
    ESP_LOGI(kTag, "==== C++ experimental layer demo: START ====");
    demo_lock_guard();
    demo_queue();
    demo_camera_frame();
    demo_logger_backend();
    demo_message_bus();
    ESP_LOGI(kTag, "==== C++ experimental layer demo: END ====");
}
