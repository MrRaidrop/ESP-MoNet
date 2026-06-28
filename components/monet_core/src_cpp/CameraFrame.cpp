// SPDX-License-Identifier: MIT
//
// CameraFrame.cpp — out-of-line definitions for the RAII camera frame.
// The buffer is only ever handed back to the driver via esp_camera_fb_return,
// preserving the existing zero-copy ownership contract.
//
#include "monet_core_cpp/CameraFrame.hpp"

namespace monet {

CameraFrame::~CameraFrame() {
    reset();  // returns fb_ to the driver if we still own it
}

CameraFrame& CameraFrame::operator=(CameraFrame&& other) noexcept {
    if (this != &other) {
        reset(other.fb_);     // return whatever we held, adopt other's buffer
        other.fb_ = nullptr;  // source no longer owns it
    }
    return *this;
}

camera_fb_t* CameraFrame::release() {
    camera_fb_t* fb = fb_;
    fb_ = nullptr;  // caller now owns it; our destructor will not return it
    return fb;
}

void CameraFrame::reset(camera_fb_t* fb) {
    if (fb_ != nullptr) {
        esp_camera_fb_return(fb_);
    }
    fb_ = fb;
}

}  // namespace monet
