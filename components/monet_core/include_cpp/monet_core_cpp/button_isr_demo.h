// SPDX-License-Identifier: MIT
//
// button_isr_demo.h — C-callable entry point for the GPIO-ISR demo.
//
// Sets up a falling-edge interrupt on the BOOT button (GPIO0) and a worker
// task. The ISR hands the press to the task through a FreeRTOS queue
// (xQueueSendFromISR via monet::Queue) — the canonical "ISR wakes a task /
// defer the work out of interrupt context" pattern. Returns immediately; the
// task runs for the life of the program.
//
#ifndef MONET_CORE_CPP_BUTTON_ISR_DEMO_H
#define MONET_CORE_CPP_BUTTON_ISR_DEMO_H

#ifdef __cplusplus
extern "C" {
#endif

void monet_button_isr_demo_start(void);

#ifdef __cplusplus
}
#endif

#endif  // MONET_CORE_CPP_BUTTON_ISR_DEMO_H
