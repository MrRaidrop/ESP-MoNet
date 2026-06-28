// SPDX-License-Identifier: MIT
//
// LoggerBackend.hpp — interface / polymorphism, the embedded-friendly way.
//
// C pattern (what we replace): an ops-table of function pointers.
//     struct logger_ops { void (*write)(void *ctx, const char *, size_t); };
//     const struct logger_ops *backend;
//     backend->write(backend_ctx, buf, len);
//
// C++ pattern: a pure-virtual interface. The vtable IS the ops-table, the
// `this` pointer IS the ctx, and the compiler wires the dispatch for us.
//     LoggerBackend *backend = logger_default_backend();
//     backend->write(buf, len);
//
// Why runtime polymorphism here? Only because a logger genuinely benefits
// from runtime backend selection / fallback (e.g. switch UART -> USB when a
// host attaches). Where the backend is fixed at build time we still let
// Kconfig pick the concrete type (see logger_default_backend) so we do not
// drag unused transports into flash. Polymorphism is a tool, not a default.
//
// No RTTI, no exceptions, no heap: backends are static singletons.
//
#ifndef MONET_CORE_CPP_LOGGERBACKEND_HPP
#define MONET_CORE_CPP_LOGGERBACKEND_HPP

#include <cstddef>

namespace monet {

// Abstract log sink. One virtual call; cheap and embedded-safe.
class LoggerBackend {
public:
    virtual ~LoggerBackend() = default;

    // Emit `len` bytes starting at `data`. Implementations must not block
    // indefinitely and must tolerate len == 0.
    virtual void write(const char* data, size_t len) = 0;

    // Convenience: write a NUL-terminated C string.
    void write(const char* str);
};

// Writes to the primary UART console (the default ESP-IDF stdout path).
class UartLoggerBackend final : public LoggerBackend {
public:
    void write(const char* data, size_t len) override;
    using LoggerBackend::write;  // keep the const char* overload visible
};

// Writes to the USB-CDC console. In this demo it shares the stdout path but
// is a distinct type to show runtime backend selection / fallback.
class UsbLoggerBackend final : public LoggerBackend {
public:
    void write(const char* data, size_t len) override;
    using LoggerBackend::write;
};

// Records every write into a fixed in-memory buffer instead of a peripheral.
// Exists for tests: inject one in place of a real backend (LoggerService takes
// a LoggerBackend&) and assert on what the service actually emitted. No heap —
// the capture buffer is a member array; writes past capacity are truncated but
// still counted, so "did it try to write too much" stays observable.
class FakeLoggerBackend final : public LoggerBackend {
public:
    void write(const char* data, size_t len) override;
    using LoggerBackend::write;

    const char* contents() const { return buf_; }   // NUL-terminated capture
    size_t      writes() const { return writes_; }   // number of write() calls

private:
    static constexpr size_t kCapacity = 512;
    char   buf_[kCapacity + 1] = {};  // +1 keeps a trailing NUL for contents()
    size_t len_ = 0;
    size_t writes_ = 0;
};

// Accessors for the static backend singletons (no allocation).
UartLoggerBackend& uart_logger_backend();
UsbLoggerBackend& usb_logger_backend();

// Returns the backend selected at build time via Kconfig
// (CONFIG_MONET_CPP_LOGGER_BACKEND_*). Demonstrates "config picks the
// concrete type, interface keeps the call site backend-agnostic".
LoggerBackend& logger_default_backend();

}  // namespace monet

#endif  // MONET_CORE_CPP_LOGGERBACKEND_HPP
