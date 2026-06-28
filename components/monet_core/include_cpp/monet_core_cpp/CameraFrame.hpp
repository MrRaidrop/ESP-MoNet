// SPDX-License-Identifier: MIT
//
// CameraFrame.hpp — RAII ownership of a zero-copy camera frame buffer.
//
// The existing camera pipeline passes `camera_fb_t*` around without copying
// the pixels, and whoever finishes with the frame must call
// `esp_camera_fb_return()` (often via a msg_t release hook). Forgetting that
// call leaks a DMA frame buffer and eventually starves the camera.
//
// C pattern (what we replace):
//     camera_fb_t *fb = esp_camera_fb_get();
//     ...                          // every path below must remember:
//     esp_camera_fb_return(fb);    // ...this, including error returns
//
// C++ pattern:
//     {
//         monet::CameraFrame frame(esp_camera_fb_get());
//         use(frame.get());        // zero-copy: same pointer, no duplication
//     }                            // destructor returns the buffer for us
//
// Ownership is unique: the frame is non-copyable but movable, so it can be
// handed off (e.g. into a queue slot or another scope) without ever
// double-returning the underlying buffer.
//
#ifndef MONET_CORE_CPP_CAMERAFRAME_HPP
#define MONET_CORE_CPP_CAMERAFRAME_HPP

#include "esp_camera.h"

namespace monet {

class CameraFrame {
public:
    CameraFrame() = default;

    // Take ownership of `fb` (may be nullptr, e.g. a failed capture).
    explicit CameraFrame(camera_fb_t* fb) : fb_(fb) {}

    // Return the buffer to the driver if we still own one.
    ~CameraFrame();

    // Zero-copy hand-off: move transfers ownership, leaving the source empty
    // so its destructor will not return the same buffer twice.
    CameraFrame(CameraFrame&& other) noexcept : fb_(other.fb_) {
        other.fb_ = nullptr;
    }
    CameraFrame& operator=(CameraFrame&& other) noexcept;

    // Copying a frame would alias the buffer and double-return it — forbidden.
    CameraFrame(const CameraFrame&) = delete;
    CameraFrame& operator=(const CameraFrame&) = delete;

    // Borrow the underlying buffer without giving up ownership.
    camera_fb_t* get() const { return fb_; }

    // True when this object currently owns a frame buffer.
    bool valid() const { return fb_ != nullptr; }

    // Give up ownership without returning the buffer — for handing the raw
    // pointer to a C API (e.g. an existing msg_t release-hook flow) that will
    // call esp_camera_fb_return() itself.
    camera_fb_t* release();

    // Return the current buffer now and optionally adopt a new one.
    void reset(camera_fb_t* fb = nullptr);

private:
    camera_fb_t* fb_ = nullptr;
};

}  // namespace monet

#endif  // MONET_CORE_CPP_CAMERAFRAME_HPP
