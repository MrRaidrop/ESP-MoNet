# 🌐 ESP32 Modular IoT Framework

[🇨🇳 中文版本 README.zh-CN.md](README.zh-CN.md) | [🇺🇸 English Version README.md](README.md)

A fully modular embedded system project built on ESP32-S3 using ESP-IDF 5.4. The system integrates UART, Wi-Fi, HTTPS cloud communication, an ADC-based light sensor (you can easily add your own), and **BLE GATT-based communication**. Future support for MQTT is also planned.

[How to Add a Sensor](docs/how_to_add_sensor.md)

[![Module CI](https://github.com/MrRaidrop/esp32_ble_mqtt_https_sensors/actions/workflows/ci.yml/badge.svg)](https://github.com/MrRaidrop/esp32_ble_mqtt_https_sensors/actions)

---

## Releases
| Version | Date       | Highlights                                                                                                                                                                                                                  |
| ------- | ---------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| v0.5    | 2025‑05‑03 | Zero‑copy camera, binary cache, adaptive FPS, new docs                                                                                                                                                                      |
| v0.6    | 2025‑05‑07 | Introduced **`service_registry`**:<br>• All services now registered via `SERVICE_REGISTER()` macro<br>• `app_main()` is simplified to a single `service_registry_init()` call<br>• Modular sensor integration fully enabled |

---

## Milestone Roadmap

| Version                           | ETA     | Key Changes                                                                                                                                                                                                                                        | Delivery Criteria                                                                                                |
| --------------------------------- | ------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------- |
| **v0.7 — Uploader Refactor**      | 2025‑05 | • Split `data_uploader_service` into multiple sinks: `http_uploader`, `ble_uploader`, `cache_sink`<br>• **Only JPEG handler owns and returns `camera_fb_t`**<br>• Add `.release()` callback to `msg_t` for clean resource lifecycle management     | \* JPEG frame leak fixed<br>\* UART/HTTP/Cache all decouple from each other<br>\* Memory-safe under stress test  |
| **v0.8 — Kconfig Migration**      | 2025‑05 | • Refactor `utils/config.h` into **component-level Kconfig** files<br>• Provide `sdkconfig.defaults` example<br>• Update README “Quick Start” to use `idf.py menuconfig`<br>• CI validates default SDK config                                      | \* All settings configurable via menuconfig<br>\* `config.h` becomes a wrapper of `sdkconfig.h` • Use Router table to control sub-pub logic, implement K-config for this                 |
| **v0.9 — BLE Service Refinement** | 2025‑05 | • Refactor **BLE GATT** layer: separate *profile* and *service* logic<br>• Introduce `ble_register_characteristic()` API<br>• Demo: add a custom Notify in 5 lines<br>• Add *How to Add BLE Characteristic* doc                                    | \* BLE unit tests cover new API<br>\* Existing Light Notify functionality remains compatible                     |
| **v1.0 — Quality Release**        | 2025‑06 | • **≥ 80 % unit test coverage** (cache, encoder, msg\_bus, registry, BLE API)<br>• GitHub Actions: build + `ctest` + `clang-format` all pass<br>• Public firmware binary + 2‑min demo video<br>• Complete bilingual docs and architecture diagrams | \* CI passes all checks<br>\* CHANGELOG & release notes finalized<br>\* README features embedded demo video link |

---

## Features

- Auto-reconnecting Wi-Fi connection manager
- Periodic sensor readings via ADC (Analog-to-Digital Converter) or Digital Pin
- Secure HTTPS POST to cloud with JSON payloads
- Data uploader: 5-second loop by default (adaptive via RSSI)
- UART echo for sensor debugging
- BLE GATT Server: notifies mobile device with sensor data (e.g. light / JPEG hash)
- Component-based structure under `components/`
- Over-the-Air (OTA) firmware update (starts 30s after boot by default)
- **Camera (OV2640) JPEG capture + HTTP upload**
- **Message Bus `EVENT_SENSOR_JPEG` for binary frame pipeline**
- **Zero-copy JPEG transmission (`camera_fb_t*` passthrough, no memcpy)**
- **Offline binary JPEG ring buffer with auto-retry**
- **Centralized Service Registry**: one-line macro registration via `SERVICE_REGISTER()`, auto-started on boot
- Future-ready: designed for MQTT and custom sensors
- [Camera Module Deep Dive](docs/camera_module.md)
- Bilingual Docs + Mermaid Diagrams

---

##  Project Structure

```
project-root
├── components/
│   ├── monet_hal/                 # Hardware abstraction layer (ADC, UART, Wi-Fi)
│   │   ├── include/monet_hal/*.h
│   │   └── src/*.c
│   ├── monet_core/                # Core messaging infrastructure (e.g., msg_bus)
│   │   ├── include/monet_core/*.h
│   │   └── src/*.c
│   ├── net/                 # Networking (HTTPS POST, future MQTT)
│   │   ├── include/net/*.h
│   │   └── src/*.c
│   ├── OTA/                 # OTA update support via HTTPS
│   │   ├── include/OTA/*.h
│   │   └── src/*.c
│   ├── service/             # Business logic (FreeRTOS tasks, BLE, Uploader)
│   │   ├── include/service/*.h
│   │   └── src/*.c
│   └── utils/               # Common tools and infrastructure
│       ├── include/utils/*.h
│       └── src/*.c          # json_utils, cache, log, format helpers
│
├── main/                    # Application entry point
│   ├── main.c
│   └── CMakeLists.txt
│
├── .github/workflows/       # GitHub Actions CI
│   └── ci.yml
├── server/                  # OTA test server (optional)
└── README.md

```

---

##  Current Architecture

##  System Architecture

> Click the link below to view the interactive system diagram:  
👉 [System Architecture Diagram - GitHub Pages](https://mrraidrop.github.io/ESP-MoNet/)

---
# Getting Started

This guide will help you build, configure, and run the ESP-MoNet project on your **ESP32-S3** board.

## Prerequisites

- ESP-IDF v5.0 or higher installed and configured
- [How to get ESP-IDF](https://docs.espressif.com/projects/vscode-esp-idf-extension/en/latest/)
- Your ESP32-S3 DevKit board (e.g. Freenova ESP32-S3 with PSRAM)
- A USB cable and access to serial terminal (e.g. `screen`, `minicom`, or `idf.py monitor`)

## Quick Setup

1. **Clone the repository**
   ```bash
   git clone https://github.com/MrRaidrop/ESP-MoNet.git
   cd ESP-MoNet
   ```

2. **Set the target chip**
   ```bash
   idf.py set-target esp32s3
   ```

3. **Use provided configuration**
   ```bash
   cp sdkconfig.default sdkconfig
   ```

4. **Configure and verify settings (optional)**
   ```bash
   idf.py menuconfig
   ```

   Key settings (already set in `sdkconfig.default`, but double-check if needed):

   - **Wi-Fi**: enabled by default (`CONFIG_WIFI_SERVICE_ENABLE`)  
   - **BLE**: enabled (`CONFIG_BLE_SERVICE_ENABLE`)  
     BLE needs **Bluetooth 4.2**, not 5.0 (required on ESP32-S3)
   - **Camera**: enabled (`CONFIG_CAMERA_SERVICE_ENABLE`)  
     Requires **PSRAM support** on ESP32-S3 and camera module (OV2640)
   - **ADC / Light Sensor**: enabled (`CONFIG_LIGHT_SENSOR_ENABLE`)
   - **msg_bus / cache / JSON / uploader** modules are enabled

## Build & Flash

```bash
idf.py build flash monitor
```

> Tip: Use `idf.py menuconfig` anytime to enable/disable features in the **Modules** section.

## Optional: Verify Functionality

- **BLE**: Connect via **nRF Connect**, observe sensor notify (e.g. `light_value`, `jpeg_frame_hash`)
- **UART**: Run `screen /dev/ttyUSB0 115200` or use serial terminal to see logs
- **Wi-Fi**: ESP32 will upload JPEG + sensor data to your configured HTTPS server

## What’s pre-configured in sdkconfig.default

| Module         | Setting                                     |
|----------------|---------------------------------------------|
| Wi-Fi          | Enabled, STA mode                           |
| BLE            | Enabled, **4.2** only (not 5.0!)             |
| Camera         | Enabled, **PSRAM required**                 |
| Light Sensor   | Enabled                                     |
| Logging        | Info level, tag filtering enabled           |
| Services       | All core modules enabled (msg_bus, cache)   |

---

Ready to go? Plug in your board, flash it, and watch the data flow.

---

### OTA Update Test

Workflow:
Firmware boots and connects to Wi-Fi

ota_test_task starts counting to 30

After 30s, it performs OTA via HTTPS:

```c
https://<your_server_ip>:8443/firmware.bin 
```

To test OTA, build a new firmware version, host it on your server, and let the device auto-update.

For complete OTA server setup instructions, refer to:

```c
/server/README_SERVER.md
```

---

## JSON Upload Format

All sensor data is uploaded in a unified JSON format,
automatically generated by the centralized encoder in json_encoder.c.

Each msg_t from the message bus is converted using json_encoder_encode()
and sent via HTTP POST or BLE Notify, depending on connection status.

Example Outputs
Light Sensor
```json
{
  "type": "light",
  "value": 472,
  "ts": 1713302934
}

```

DHT22 Temperature & Humidity
```json
{
  "type": "temp",
  "temperature": 24.65,
  "humidity": 63.10,
  "ts": 1713302971
}

```

Unknown topic fallback
```json
{
  "type": "unknown",
  "topic": 99,
  "ts": 1713303001
}

```

**Add Your Own Format**

To define the format for your new sensor:

Extend the msg_t data union in msg_bus.h

Add a case in json_encoder_encode()
to format the desired JSON fields for your topic.

The upload system requires no other changes — just define the message and format once.

---

# Project Roadmap

## Feature Overview

| Feature                         | Status        | Notes                                                |
|----------------------------------|---------------|------------------------------------------------------|
| Light sensor ADC driver         | ✅ Done        | Reads every 1s, publishes via msg_bus                |
| JSON packaging utility          | ✅ Done        | Structured `type + value + timestamp` format         |
| HTTPS POST to cloud             | ✅ Done        | Modular `http_post_hal()` + retry support            |
| BLE GATT notification           | ✅ Done        | Subscribe msg_bus, notify phone via `notify_raw()`   |
| UART forwarding (light sensor)  | ✅ Done        | UART sends formatted values via msg_bus subscription |
| Upload retry (cache)            | ✅ Done        | RAM ring buffer with manual `flush_with_sender()`    |
| Modular task architecture       | ✅ Done        | Each service runs as isolated FreeRTOS task          |
| OTA update (Wi-Fi)              | ✅ Done        | HTTPS OTA using `esp_https_ota()`                    |
| GitHub repo + documentation     | ✅ Done        | Clean README, architecture diagram, GitHub Actions   |
| Camera JPEG capture             | ✅ Done        | OV2640 integration + camera_hal abstraction          |
| Zero-copy JPEG pipeline         | ✅ Done        | `camera_fb_t*` passed directly via msg_bus           |
| Offline binary JPEG cache       | ✅ Done        | flash-resident ring buffer with auto flush           |
| Dynamic FPS based on RSSI       | ✅ Done        | Adapts to Wi-Fi quality automatically                |
| MQTT secure upload              | 🔜 Planned     | Add TLS MQTT broker support                          |
| OTA update (BLE)                | 🔜 Planned     | Plan to implement BLE-based OTA update               |
| DMA + Ring Buffer integration   | ✅ Done        | For ultrasonic / high-rate sensor support            |
| Flutter mobile app (sensor UI)  | 🔜 Planned     | BLE dashboard for real-time sensor data              |
| Flutter mobile app (BLE OTA)    | 🔜 Planned     | Integrated BLE OTA functionality                     |

---

## Known Limitations & Improvement Plan

| Category       | Issue Description                                 | Improvement Direction                                | Status        |
|----------------|----------------------------------------------------|------------------------------------------------------|---------------|
| Architecture   | No centralized service lifecycle manager           | Add `service_registry` + `app_init()` startup logic  | ✅ Done |
| Config System  | Configs hardcoded in `.c` files                    | Use `Kconfig` + NVS runtime override                 | 🔜 Planned     |
| Logging        | LOGI/W macros used, but no module-level control    | Introduce `LOG_MODULE_REGISTER` + per-module level   | ✅ Done     |
| Unit Testing   | Only BLE utils tested in CI                        | Add test cases for `json_utils`, cache, uploader     | ⏳ In Progress |
| HTTPS Security | TLS certs not validated                           | Add CA config toggle + cert verification             | 🔜 Planned     |
| OTA Mechanism  | No image validation or rollback                    | Add SHA256 + dual partition fallback                 | 🔜 Planned     |
| BLE Extension  | Only 1 notify char, no write command support       | Extend GATT profile to support control commands      | ⏳ In Progress |

> Want to contribute or suggest improvements? Feel free to [open an issue](https://github.com/MrRaidrop/esp32_ble_mqtt_https_sensors/issues) or fork this repo!


---
## Example Use Cases

- Wi-Fi JPEG camera node with fail-safe cache
- BLE-notifying environmental monitor
- Hybrid image + sensor uploader
- Custom IoT prototype platform, easily extendable to  sensors, or servo output
- Edge computing node with dynamic FPS control
- Developer-friendly OTA testbed

---

# How to Add a New Sensor

This guide walks you through integrating a new sensor (e.g., DHT22 for temperature/humidity) into the system.  
Because this is a modular architecture, you can do this in **3 simple steps** — without modifying any other services (UART, BLE, HTTP, etc.).

> Click the link below to view the guide.
👉 [docs/how_to_add_sensor](docs/how_to_add_sensor.md)

---


## Camera Module Deep Dive

> Click the link below to view the guide.
👉 [docs/camera_module.md](docs/camera_module.md)

Zero‑Copy, Adaptive & Offline‑Resilient

*(Drop‑in for any OV2640‑based ESP32‑S3 board — tested on Freenove DevKit)*


##  License

MIT License — Use freely, modify, and integrate.

---

🛠️ Last Updated: May 3, 2025  
Made with ❤️ by [Greyson Yu](https://github.com/MrRaidrop)
