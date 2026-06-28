// SPDX-License-Identifier: MIT
//
// cpp_demo.h — C-callable entry point for the experimental C++ demo.
//
// Lets app_main() (plain C) exercise the C++ layer with a single call,
// guarded by CONFIG_MONET_CPP_EXPERIMENTAL. Safe to include from C and C++.
//
#ifndef MONET_CORE_CPP_CPP_DEMO_H
#define MONET_CORE_CPP_CPP_DEMO_H

#ifdef __cplusplus
extern "C" {
#endif

// Runs a short, self-contained demonstration of the C++ abstractions:
// RAII LockGuard, RAII CameraFrame ownership/move, type-safe Queue<T,N>,
// the LoggerBackend interface, and the MessageBus class + C API bridge.
// It allocates nothing on the heap and returns when finished.
void monet_cpp_demo_run(void);

#ifdef __cplusplus
}
#endif

#endif  // MONET_CORE_CPP_CPP_DEMO_H
