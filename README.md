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

## Features

- Auto-reconnecting Wi-Fi connection manager
- Periodic light sensor readings via ADC  
  *(currently only light sensor available—T_T)*
- Secure HTTPS POST to cloud with JSON payloads
- Data reporter with 5-second upload loop
- UART echo for sensor debugging
- BLE GATT Server: notifies mobile device with sensor data (e.g. light)
- Component-based structure under `components/`
- Over-the-Air (OTA) firmware update (starts 30s after boot by default)
- **Camera (OV2640) JPEG capture + HTTP upload**
- **Message Bus `EVENT_SENSOR_JPEG` for binary frames**
- [Camera Module Deep Dive](#camera-module-deep-dive)
- Future-ready: designed for MQTT integration

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

##  Getting Started

### 1. Prerequisites

- ESP-IDF 5.4+ installed and configured (Vscode ESP-IDF extension available)
- Supported board: ESP32-S3 devkit
- Internet access for cloud upload
- BLE mobile app (e.g., **nRF Connect** by Nordic)

### 2. Build and Flash

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

### 3. Wi-Fi Configuration

Update your SSID and password in `utils/config.h`

```c
#define WIFI_SSID "your-ssid"
#define WIFI_PASS "your-password"
```
It will be pass to `service/src/wifi_service.c`

### 4. BLE Verification

- Install **nRF Connect** mobile app (or other)
- Scan and connect to `ESP_GATTS_DEMO`
- Locate the characteristic under service UUID `0x00FF`
- Enable **Notify**
- You will receive 4-byte little-endian integer (e.g., light = `0x0802 = 520`)

### 4. OTA Update Test

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

## 📤 JSON Upload Format

All sensor data is uploaded in a unified JSON format via HTTP POST or BLE Notify.

Example output (light sensor reading):

```json
{
  "type": "light",
  "value": 472,
  "ts": 1713302934
}

```

You can modify `json_utils.c` to use field-style format instead:

```json
{
  "timestamp": "2025-04-11T14:23:52",
  "light": 472,
  "count": 23
}
```

---
##  Roadmap

## 📦 Feature Overview

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
| MQTT secure upload              | ⏳ In Progress | Add TLS MQTT broker support                          |
| OTA update (BLE)                | 🔜 Planned     | Plan to implement BLE-based OTA update               |
| DMA + Ring Buffer integration   | 🔜 Planned     | For ultrasonic / high-rate sensor support            |
| Flutter mobile app (sensor UI)  | 🔜 Planned     | BLE dashboard for real-time sensor data              |
| Flutter mobile app (BLE OTA)    | 🔜 Planned     | Integrated BLE OTA functionality                     |

---
## 🚧 Known Limitations & Improvement Plan

| Category       | Issue Description                                 | Improvement Direction                                | Status        |
|----------------|----------------------------------------------------|------------------------------------------------------|---------------|
| Architecture   | No centralized service lifecycle manager           | Add `service_registry` + `app_init()` startup logic  | ⏳ In Progress |
| Config System  | Configs hardcoded in `.c` files                    | Use `Kconfig` + NVS runtime override                 | 🔜 Planned     |
| Logging        | LOGI/W macros used, but no module-level control    | Introduce `LOG_MODULE_REGISTER` + per-module level   | ⏳ In Progress |
| Unit Testing   | Only BLE utils tested in CI                        | Add test cases for `json_utils`, cache, uploader     | ⏳ In Progress |
| HTTPS Security | TLS certs not validated                           | Add CA config toggle + cert verification             | 🔜 Planned     |
| OTA Mechanism  | No image validation or rollback                    | Add SHA256 + dual partition fallback                 | 🔜 Planned     |
| BLE Extension  | Only 1 notify char, no write command support       | Extend GATT profile to support control commands      | ⏳ In Progress |


> Want to contribute or suggest improvements? Feel free to [open an issue](https://github.com/MrRaidrop/esp32_ble_mqtt_https_sensors/issues) or fork this repo!


---
##  Example Use Cases

- Low-power sensor node with cloud logging
- BLE + UART + MQTT hybrid IoT edge device
- Sensor/actuator hub with REST and mobile access

---

## How to Add a Sensor


# How to Add a New Sensor

This guide walks you through adding a new sensor (e.g., DHT22 for temperature/humidity) into the system. All sensors follow a simple 3-step pattern:

---

## 1. Create a HAL Driver

Location: `components/hal/dht22_hal.[ch]`

```c
/// dht22_hal.h

/**
 * @brief Initialize DHT22 GPIO and timing
 */
void dht22_hal_init(void);

/**
 * @brief Read temperature and humidity from DHT22 sensor
 * 
 * @param out_temp_deg_c Pointer to float storing temperature in °C
 * @param out_humidity_pct Pointer to float storing relative humidity %
 * @return true if read is successful, false otherwise
 */
bool dht22_hal_read(float *out_temp_deg_c, float *out_humidity_pct);
```

---

## 2. Add a Sensor Service

Location: `components/service/dht22_service.c`

```c
void dht22_service_start(void)
{
    dht22_hal_init();

    xTaskCreate([](void *) {
        while (1) {
            float temp = 0, hum = 0;
            if (dht22_hal_read(&temp, &hum)) {
                msg_t msg = {
                    .topic = EVENT_SENSOR_TEMP,
                    .ts_ms = esp_log_timestamp(),
                };
                msg.data.temp.value = temp;
                msg_bus_publish(&msg);
            }
            vTaskDelay(pdMS_TO_TICKS(10000)); // every 10 sec
        }
    }, "dht22_task", 4096, NULL, 5, NULL);
}
```

---

## 3. Extend JSON Upload Logic

Location: `json_utils.c`

```c
bool json_build_from_msg(const msg_t *msg, char *out_buf, size_t buf_size)
{
    if (msg->topic == EVENT_SENSOR_TEMP) {
        snprintf(out_buf, buf_size,
            "{"type":"temp","value":%.2f,"ts":%lu}",
            msg->data.temp.value, msg->ts_ms);
        return true;
    }
    ...
}
```

---

## Architecture Overview

👉 [System Architecture Diagram - GitHub Pages](https://mrraidrop.github.io/ESP-MoNet/)


---

## Example Output

```json
{
  "type": "temp",
  "value": 24.65,
  "ts": 3432943
}
```

---

## Add it to the Project

- Add `dht22_hal.c` to `CMakeLists.txt` in `components/hal/`
- Add `dht22_service.c` to `components/service/`
- Call `dht22_service_start()` from your `app_main()` or main service start

---

## OK, Done!

You’ve added a new sensor in just 3 simple files.  
To add more sensors (e.g. CO₂, PIR, Light), repeat with:

- 1x HAL driver
- 1x Service
- 1x JSON encoder


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

## Next Milestones

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

🛠️ Last Updated: May 1, 2025  
Made with ❤️ by [Greyson Yu](https://github.com/MrRaidrop)
