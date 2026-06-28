// SPDX-License-Identifier: MIT
// Host shim: a fixed microsecond clock. Tests that care about time inject a
// fake clock via the constructor seam instead of relying on this.
#ifndef SHIM_ESP_TIMER_H
#define SHIM_ESP_TIMER_H

#include <stdint.h>

static inline int64_t esp_timer_get_time(void) { return 0; }

#endif  // SHIM_ESP_TIMER_H
