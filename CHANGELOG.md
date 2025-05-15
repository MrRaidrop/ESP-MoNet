## [0.8.0 PREV] – 2025‑05‑12
**TL;DR** — TODO LIST for 0.8

server code update: 
    • jpeg frame handle
    • more clean structure
    • throughout test
    • README_SERVER update

KCONFIG:
    • config.h -> menuconfig |
    • all configuration adjustable by UI
    • publisher - subscriber relationship configurable through router table
    • sdkconfig.default 

How to add sensor document modification due to the change of msg_bus

WIFI start sequence modify, currently all publisher need to wait wifi    connection to start, not the best practice




## [0.7.0] – 2025‑05‑10
**TL;DR** — What You Get After v0.7
Memory‑safe zero‑copy camera pipeline (publisher‑side release hook).

Plug‑and‑play sinks: add UART / HTTP / BLE / MQTT in one descriptor, no task code.

Add a new sensor → write HAL + service that publishes either json_str or jpeg.fb; no changes to any uploader.

Cleaner code‑base: UART task ‑50 % LOC, uploader ‑80 % LOC, registry handles all queues.

The project is now genuinely “drop‑in sensor / drop‑in transport” ready.

The data pineline now look like that:

msg_bus               service_registry                 HTTP sink_cb
  │ publish(msg)            │                                 │
  └──────────────► queue ───┴─► subscriber_dispatch_task ────► http_sink_handler
                            │                                 │
                            └───(msg.release)───────────────► esp_camera_fb_return()



| Area                                 | What Changed                                                                                                                                                                                                                                                                                                                                            | **Why**                                                                                                                                                                                   | **What It Enables Now**                                                                                                                                                                                                                                  |
| ------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **1. Message Bus API**               | • `msg_t` gains a new field<br>`void (*release)(struct msg *);`                                                                                                                                                                                                                                                                                         | • Camera frames are “zero‑copy”; someone must eventually give the frame‑buffer back to the driver.<br>• Before, each subscriber had to remember to call `esp_camera_fb_return()`.         | • Publisher (e.g. Camera service) attaches a cleanup lambda once; every subscriber can stay memory‑safe just by calling `if (msg.release) msg.release(&msg);`.<br>• Future complex payloads (malloc’ed buffers, files, etc.) can use the same mechanism. |
| **2. `service_registry`**            | • Introduced **sink‑callback architecture**.<br>  `service_desc_t` → `sink_cb` pointer<br>• Unified `subscriber_dispatch_task` internally.                                                                                                                                                                                                              | • We want *all* formatting / transport logic outside of the core registry loop.<br>• Makes adding new “sinks” (UART, HTTP, BLE, MQTT…) a drop‑in, zero boiler‑plate task.                 | • Any subscriber can specify **topics + sink\_cb** and *never* write a FreeRTOS task again – the registry spins the queue/dispatcher for you.<br>• Centralised memory‑release call – done once, not in every task.                                       |
| **3. UART Service**                  | • Removed giant `switch…case` from task.<br>• Added `uart_sink_handler(const msg_t*)` and registered it via `sink_cb`.<br>• Default branch prints `msg.data.json_str.json`, falling back to a minimal human‑readable line if JSON is empty.<br>• Only JPEG still needs special handling (binary header + CRC).                                          | • Goal: adding a *new* sensor must **not** require editing UART code.<br>• Separate “how to format” (sink) from “how to schedule/receive” (registry).                                     | • Devs just publish a JSON string – it appears on UART automatically.<br>• UART task only \~20 lines; easy to maintain.<br>• Performance: one queue, zero extra copies.                                                                                  |
| **4. Data Uploader → HTTP Uploader** | • Deleted `data_uploader_service` (two dedicated tasks).<br>• Added **single** `http_uploader_service` descriptor:<br>  – Subscribes to `EVENT_GROUP_SENSOR`<br>  – `sink_cb = http_sink_handler`<br>  – JPEG → `http_post_image()`<br>  – Everything else → `http_post_send(json)`<br>  – Failure ➜ `cache_push*()`<br>  – Success path flushes cache. | • Old design required a new FreeRTOS task **per** sensor type – unscalable.<br>• HTTP logic duplicated in two loops (light vs JPEG).<br>• Needed separate *data* and *uploader* coupling. | • **Any** present / future sensor automatically uploads once it carries a JSON string.<br>• Only 1 queue, 1 task, 1 code path to maintain.<br>• Retry / cache logic unified.<br>• Easy to add BLE or MQTT by duplicating a tiny sink file.               |

---

## [0.6.0] – 2025‑05‑7


### Why it matters
| Before v0.6                                                                            | After v0.6                                                                   |
| -------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------- |
| Each new service required a bespoke `xTaskCreate()` + queue plumbing in `main.c`.      | Drop a `service_desc_t` in its own file, call `service_registry_register()`. |
| Adding a subscriber meant writing queue‑creation & `msg_bus_subscribe()` boiler‑plate. | Auto‑subscription handled once in the registry helper.                       |
| Hard to query whether a task was alive / how much stack it still had.                  | One API call returns state and stack watermark.                              |


### Added

service_registry core module

Central table that tracks every service (service_desc_t) and its runtime state.

One‑line API: service_registry_register(&svc_desc); – no more manual xTaskCreate() calls scattered through the code‑base.

service_registry_start_all() boots every registered service in the order they were added.

Auto‑subscription helper

If a service is tagged SERVICE_ROLE_SUBSCRIBER, the registry automatically

Creates an internal queue.

Subscribes the queue to the topic list declared in the descriptor.

Passes that queue as the task‑argument.

Topic‑group subscription (EVENT_GROUP_SENSOR)

Wild‑card style: one entry subscribes to all present / future sensor topics without editing code.

Run‑time status helpers

service_registry_status() → running / disabled + stack high‑water‑mark.

service_registry_get_stack_usage() for quick CLI diagnostics.

### Changed

All existing services (camera_service, light_sensor_service, uart_service, data_uploader_service, wifi_service, …) are now registered in main.c via the registry API instead of calling *_start() directly.

app_main() trimmed to <15 lines – purely registers services then calls service_registry_start_all().

### Removed

Legacy components/service/**/_start() helpers (kept only for backward‑compat unit tests).


---


## [0.5.0] – 2025‑05‑03

Initial public checkpoint

### Added
- Zero-copy JPEG camera pipeline (OV2640 + `camera_service`)
- Offline binary ring buffer (`cache.c`) for retry support
- BLE GATT Server for live sensor data push (notify)
- UART echo and forwarding mechanism
- OTA firmware update via HTTPS
- Component-based project structure
- GitHub Actions CI + Documentation pages (Mermaid, sensor guides)


### Added
- `DHT22` sensor HAL (`dht22_hal.c/h`) and service (`dht22_service.c`)
- Centralized `json_encoder` module for structured sensor data encoding
- `how_to_add_sensor.md` step-by-step sensor integration guide
- JSON upload path now supports multiple sensor types via `msg_bus`
- Updated Mermaid architecture diagram with encoder + DHT22 paths

### Changed
- Restructured `README.md` (split into Camera Module, Getting Started, etc.)
- Replaced legacy `json_utils.c` with `json_encoder.c` (moved to `codec/`)
- `data_uploader_service.c` now uses `json_encoder_encode()` for uploads
- Improved `How to Add a Sensor` doc: explains `msg_t` and `Kconfig` workflow

### Deprecated
- `json_utils.c` (use `json_encoder.c` instead)

