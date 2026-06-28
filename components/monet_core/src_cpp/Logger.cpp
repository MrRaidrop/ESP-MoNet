// SPDX-License-Identifier: MIT
//
// Logger.cpp — concrete LoggerBackend implementations and the build-time
// default selection driven by Kconfig.
//
#include "monet_core_cpp/LoggerBackend.hpp"

#include <cstdio>
#include <cstring>

#include "sdkconfig.h"

namespace monet {

void LoggerBackend::write(const char* str) {
    if (str != nullptr) {
        write(str, std::strlen(str));
    }
}

// Both backends route through stdout, which on ESP-IDF is the selected
// console peripheral. They stay distinct types so the call site can switch
// or fall back between them at runtime through the LoggerBackend interface;
// in a real port each would target its own peripheral driver.
void UartLoggerBackend::write(const char* data, size_t len) {
    if (data != nullptr && len > 0) {
        std::fwrite(data, 1, len, stdout);
        std::fflush(stdout);
    }
}

void UsbLoggerBackend::write(const char* data, size_t len) {
    if (data != nullptr && len > 0) {
        std::fwrite(data, 1, len, stdout);
        std::fflush(stdout);
    }
}

// Function-local statics: constructed once, no heap, thread-safe init.
UartLoggerBackend& uart_logger_backend() {
    static UartLoggerBackend backend;
    return backend;
}

UsbLoggerBackend& usb_logger_backend() {
    static UsbLoggerBackend backend;
    return backend;
}

LoggerBackend& logger_default_backend() {
#if defined(CONFIG_MONET_CPP_LOGGER_BACKEND_USB)
    return usb_logger_backend();
#else
    // Default / CONFIG_MONET_CPP_LOGGER_BACKEND_UART
    return uart_logger_backend();
#endif
}

}  // namespace monet
