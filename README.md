# 🌐 ESP32 Modular IoT Framework

[🇨🇳 中文版本 README.zh-CN.md](README.zh-CN.md) | [🇺🇸 English Version README.md](README.md)

A fully modular embedded system project built on ESP32-S3 using ESP-IDF 5.4. The system integrates UART, Wi-Fi, HTTPS cloud communication, an ADC-based light sensor (you can easily add your own), and **BLE GATT-based communication**. Future support for MQTT is also planned.

[How to Add a Sensor](#how-to-add-a-sensor)

[![BLE Module CI](https://github.com/MrRaidrop/esp32_ble_mqtt_https_sensors/actions/workflows/ci.yml/badge.svg)](https://github.com/MrRaidrop/esp32_ble_mqtt_https_sensors/actions)

---

## Why This Repo

- Modular Architecture 
Clean separation into **HAL / Core / Service / Net** layers. Every module is plug-and-play—easy to remove, replace, or extend. Comes with a complete [sensor integration guide](https://mrraidrop.github.io/ESP-MoNet/#/how_to_add_sensor).

- Event-Driven Message Bus
Fully decoupled, many-to-many publish/subscribe architecture—more flexible than direct queues and ideal for scalable embedded design.

- Secure OTA + HTTPS  
Out-of-the-box secure OTA update system, with future support for AWS IoT Jobs.

- CI-Ready Testing 
Includes a BLE unit test example, and is designed for integration with `idf.py build` in CI pipelines.

- Dual-Channel Upload  
Automatic failover between Wi‑Fi and BLE. RAM cache ensures no data loss during disconnections—frames are cached and retried seamlessly.

- Bilingual Docs + Mermaid Diagrams  
English and Chinese documentation with rich Mermaid diagrams. Friendly for open-source contributors worldwide.

---

## Releases
| Version | Date | Highlights |
|---------|------|------------|
| v0.5    | 2025‑05‑03 | Zero‑copy camera, binary cache, adaptive FPS, new docs |

---

## Milestone Roadmap

| Version                             | ETA       | Key Changes                                                                                                                                                                     | Delivery Criteria                                                                                       |
|-------------------------------------|-----------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------------|
| **v0.7 — Service Registry**         | 2025‑05   | • Introduce **`service_registry`** mechanism<br>• All `*_service_start()` functions registered via `SERVICE_REGISTER()` macro<br>• `app_main()` reduced to `service_registry_init()` call<br>• README and *How to Add Sensor* updated | \* Firmware builds and runs correctly<br>\* All core services (`light`, `camera`, `DHT22`, `uploader`, `UART`, `BLE`) launched via registry<br>\* Architecture diagram updated |
| **v0.8 — Kconfig Migration**        | 2025‑05   | • Refactor `utils/config.h` into **component-level Kconfig** files<br>• Provide `sdkconfig.defaults` example<br>• Update README “Quick Start” to use `idf.py menuconfig`<br>• CI validates default SDK config | \* All settings configurable via menuconfig<br>\* `config.h` becomes a wrapper of `sdkconfig.h`                           |
| **v0.9 — BLE Service Refinement**   | 2025‑05   | • Refactor **BLE GATT** layer: separate *profile* and *service* logic<br>• Introduce `ble_register_characteristic()` API<br>• Demo: add a custom Notify in 5 lines<br>• Add *How to Add BLE Characteristic* doc | \* BLE unit tests cover new API<br>\* Existing Light Notify functionality remains compatible                             |
| **v1.0 — Quality Release**          | 2025‑06   | • **≥ 80 % unit test coverage** (cache, encoder, msg_bus, registry, BLE API)<br>• GitHub Actions: build + `ctest` + `clang-format` all pass<br>• Public firmware binary + 2‑min demo video<br>• Complete bilingual docs and architecture diagrams | \* CI passes all checks<br>\* CHANGELOG & release notes finalized<br>\* README features embedded demo video link          |


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
- **Dynamic FPS: auto-adjusts based on Wi-Fi signal (RSSI)**
- Future-ready: designed for MQTT and custom sensors
- [Camera Module Deep Dive](docs/camera_module.md)

---

##  Project Structure

```
project-root
├── components/
│   ├── hal/                 # Hardware abstraction layer (ADC, UART, Wi-Fi)
│   │   ├── include/hal/*.h
│   │   └── src/*.c
│   ├── core/                # Core messaging infrastructure (e.g., msg_bus)
│   │   ├── include/core/*.h
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

> Click the link below to view the interactive system diagram with Mermaid rendering support:  
👉 [System Architecture Diagram - GitHub Pages](https://mrraidrop.github.io/ESP-MoNet/)

> This modular architecture enables flexible service composition, better testing, and future support for more transports (e.g., MQTT).

---
# Getting Started

This guide will help you build, configure, and run the ESP-MoNet project on your **ESP32-S3** board.

## Prerequisites

- ESP-IDF v5.0 or higher installed and configured
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
| Architecture   | No centralized service lifecycle manager           | Add `service_registry` + `app_init()` startup logic  | ⏳ In Progress |
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
The project uses a clean, modular structure — all sensors follow the **3-step rule**.

---

## 1. Create a HAL Driver

**Location:** `components/my_hal/dht22_hal.[ch]`

```c
/// dht22_hal.h

/**
 * @brief Initialize DHT22 GPIO and timing
 */
esp_err_t dht22_hal_init(void);

/**
 * @brief Read temperature and humidity from DHT22 sensor
 * 
 * @param out_temp_deg_c Pointer to float storing temperature in °C
 * @param out_humidity_pct Pointer to float storing relative humidity in %
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t dht22_hal_read(float *out_temp_deg_c, float *out_humidity_pct);
```

The HAL should only handle **raw hardware access** (GPIO, timing, etc). Return stub values if testing without sensor.

---

## 2. Add a Sensor Service

**Location:** `components/service/dht22_service.[ch]`

```c
void dht22_service_start(void)
{
    if (dht22_hal_init() != ESP_OK) {
        LOGE("DHT22_SERVICE", "Failed to init HAL");
        return;
    }

    xTaskCreate([](void *) {
        while (1) {
            float temp = 0, hum = 0;
            if (dht22_hal_read(&temp, &hum) == ESP_OK) {
                msg_t msg = {
                    .topic = EVENT_SENSOR_TEMP,
                    .ts_ms = esp_log_timestamp(),
                };
                snprintf(msg.data.json_str, sizeof(msg.data.json_str),
                         "{\"type\":\"temp\",\"t\":%.2f,\"h\":%.2f}", temp, hum);
                msg_bus_publish(&msg);
            }
            vTaskDelay(pdMS_TO_TICKS(10000));
        }
    }, "dht22_task", 4096, NULL, 5, NULL);
}
```

You can publish either `.json_str` or raw `.value` data depending on your message model.

---

## 3. Add to CMake

Ensure you edit `CMakeLists.txt`:

- In `components/my_hal/CMakeLists.txt`, add:
  ```cmake
  srcs += src/dht22_hal.c
  ```

- In `components/service/CMakeLists.txt`, add:
  ```cmake
  srcs += src/dht22_service.c
  ```

---

## 4. Extend JSON Encoding

To support cloud upload for your new sensor,
you no longer need to snprintf() JSON strings manually.

Instead, the system uses a centralized encoder:

```c
#include "codec/json_encoder.h"

char json_buf[256];
json_encoder_encode(&msg, json_buf, sizeof(json_buf));
```

To support your sensor:

Add a new topic (e.g. EVENT_SENSOR_TEMP) in msg_bus.h

Extend the msg_t union with a matching data struct (e.g. temp_hum)

Add a new case block in json_encoder_encode() in json_encoder.c:

```c
case EVENT_SENSOR_TEMP:
    snprintf(out_buf, buf_size,
        "{ \"type\": \"temp\", \"temperature\": %.2f, \"humidity\": %.2f, \"ts\": %" PRIu32 " }",
        msg->data.temp_hum.temperature,
        msg->data.temp_hum.humidity,
        msg->ts_ms);
    return true;
```

No further change is needed in uploader or cache,
your new sensor will be automatically handled by both Wi-Fi and BLE.

---

## Example JSON Output

```json
{
  "type": "temp",
  "t": 24.65,
  "h": 52.1,
  "ts": 3432943
}
```

---

## Optional BLE Notification

To notify mobile devices via BLE, just:

1. Subscribe to `EVENT_SENSOR_TEMP` in `ble_service.c`
2. Format & call `notify_raw()` with the same JSON string

---


---

## 5.Modify `msg_t` Structure

To support new sensor data types (like temperature and humidity), you should update the `msg_t` definition in `core/msg_bus.h`:

```c
typedef struct {
    uint32_t ts_ms;
    union {
        struct {
            float t;  ///< temperature in °C
            float h;  ///< relative humidity %
        } temp;

        struct {
            int adc_value;
        } light;

        struct {
            camera_fb_t *fb;
        } jpeg;

        // Add more types here as needed
    } data;
    event_topic_t topic;
} msg_t;
```


## OK, Done!

You’ve added a new sensor in just **3 files + 2 CMake lines**.  
To add more (e.g. CO₂, PIR, gas, tilt), just repeat the pattern:

- HAL driver (`my_hal/`)
- Service module (`service/`)
- JSON/Notify integration (optional)



---

## Camera Module Deep Dive

Zero‑Copy, Adaptive & Offline‑Resilient

*(Drop‑in for any OV2640‑based ESP32‑S3 board — tested on Freenove DevKit)*

| Key Capability                  | How it Works                                                                                                                                         | Why it Matters                                               |
|--------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------|--------------------------------------------------------------|
| **Zero‑copy JPEG pipeline**    | `camera_fb_t*` is published on `msg_bus` → uploader task uses it directly → **one** `esp_camera_fb_return()`. No `memcpy()`.                        | • ≈ 50 % PSRAM saved<br>• Higher FPS<br>• Ready for streaming |
| **Dynamic FPS / bandwidth adapt** | Each frame calls `wifi_get_rssi()` and maps RSSI → interval:<br>`>-60 dBm ➜ 3 FPS` ・ `-70…-60 dBm ➜ 1 FPS` ・ `<-70 dBm ➜ 0.2 FPS`                | • Auto‑throttles in weak Wi‑Fi<br>• Keeps link stable         |
| **Offline binary cache**       | On `http_post_image()` failure, `cache_push_blob()` stores JPEG into PSRAM ring buffer. On reconnect, `cache_flush_once_with_sender_ex()` retries. | • No data loss during outages<br>• Seamless store‑and‑forward |
| **Message‑bus decoupled**      | Any task can subscribe to `EVENT_SENSOR_JPEG`. Swap HTTP uploader with MQTT or SD‑card logger without touching camera code.                        | • Loose coupling<br>• Unit‑testable<br>• Easy to extend       |

---

## Design Walk‑through

### 1. Init  
```c
camera_hal_init();  // init OV2640, PSRAM, pins, quality, etc.
```

### 2. Capture Loop
```c
camera_fb_t *fb = camera_hal_capture();
publish(EVENT_SENSOR_JPEG, fb);          // zero copy
update_capture_interval();               // RSSI-based
vTaskDelay(capture_interval_ms);
```

### 3. Upload Task
```c
success = http_post_image(fb->buf, fb->len);
if (!success) cache_push_blob(fb->buf, fb->len);
esp_camera_fb_return(fb);  // release here
```

### 4. Cache Flush
```c
cache_flush_once_with_sender_ex(http_post_image);
```

---

## Config Snippet (`utils/config.h`)

```c
#define CONFIG_CAPTURE_INTERVAL_MS   1000           // default (overridden dynamically)
#define CONFIG_CACHE_ITEM_SIZE       (40*1024)      // max JPEG size
#define CONFIG_CACHE_MAX_ITEMS       10             // number of cached frames
```

---

## Typical Resource Usage

| Frame Size      | PSRAM Peak (before/after) | FPS (‑70 dBm RSSI) |
|------------------|----------------------------|---------------------|
| XGA 1024×768     | 680 KB → **340 KB**        | ~0.9 FPS            |
| SVGA 800×600     | 450 KB → **230 KB**        | ~1.1 FPS            |

> Measured with `heap_caps_get_info(MALLOC_CAP_SPIRAM)` on ESP-IDF v5.4

---

## Next Milestones for Camera

- MJPEG streaming via `multipart/x-mixed-replace`
- SD‑Card fallback cache when PSRAM full
- Per‑frame adaptive JPEG quality based on RSSI

---

> **TL;DR** – The camera module captures once, copies zero, adapts to Wi‑Fi in real time, and never loses a frame even if the router goes down.  
Plug in, subscribe, profit

---

##  License

MIT License — Use freely, modify, and integrate.

---

🛠️ Last Updated: May 3, 2025  
Made with ❤️ by [Greyson Yu](https://github.com/MrRaidrop)
