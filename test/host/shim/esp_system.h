// SPDX-License-Identifier: MIT
// Host shim: esp_restart() as a recorded no-op so the Reset fail-safe path can
// be compiled (tests use Log/MarkUnhealthy and never actually restart).
#ifndef SHIM_ESP_SYSTEM_H
#define SHIM_ESP_SYSTEM_H

static inline void esp_restart(void) { /* no-op on host */ }

#endif  // SHIM_ESP_SYSTEM_H
