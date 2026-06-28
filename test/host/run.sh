#!/usr/bin/env bash
# Host-side unit tests for the experimental C++ layer (no ESP-IDF, no board).
# Compiles the real components/monet_core/src_cpp sources against the thin
# FreeRTOS/ESP shims in test/host/shim and runs them under Unity.
set -euo pipefail

# repo root = two levels up from this script (test/host/run.sh)
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"

OUT="$(mktemp -d)"
trap 'rm -rf "$OUT"' EXIT

# Unity is C; build it as C to avoid C++ strictness on the vendored source.
gcc -c -Itest/unity/src test/unity/src/unity.c -o "$OUT/unity.o"

g++ -std=c++17 -Wall -Wextra -O0 -g \
    -Itest/host/shim \
    -Icomponents/monet_core/include_cpp \
    -Itest/unity/src \
    components/monet_core/src_cpp/Logger.cpp \
    components/monet_core/src_cpp/LoggerService.cpp \
    components/monet_core/src_cpp/TaskHealthMonitor.cpp \
    components/monet_core/src_cpp/CameraFrame.cpp \
    test/host/test_cpp_layer.cpp \
    "$OUT/unity.o" \
    -o "$OUT/test_cpp_layer"

"$OUT/test_cpp_layer"
