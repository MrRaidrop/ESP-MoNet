# 🌐 ESP32 Modular IoT Framework 
[CN 中文版本](README.zh-CN.md)

A fully modular embedded system project built on ESP32-S3 using ESP-IDF 5.4. The system integrates UART, Wi-Fi, HTTPS cloud communication, ADC-based light sensor (you can add whatever sensor you want), and **BLE GATT-based communication**. Future support for MQTT is also planned.

[![BLE Module CI](https://github.com/MrRaidrop/esp32_ble_mqtt_https_sensors/actions/workflows/ci.yml/badge.svg)](https://github.com/MrRaidrop/esp32_ble_mqtt_https_sensors/actions)
how modules interact with eachother
---

##  Features

-  Auto-reconnecting Wi-Fi connection manager
-  Periodic light sensor reading via ADC (because currently I only have light sensor T_T)
-  Secure HTTPS POST to cloud with JSON data
-  Data reporter module with 5-second upload loop
-  UART echo for sensor debugging
-  BLE GATT Server: Notify mobile device with sensor data (e.g., light value)**
-  **Component‑based source layout (ESP‑IDF components/)**
-  **Over-the-air (OTA) firmware update based on wifi** (30s after boot, you can change when you want to ota)
-  Future-ready for MQTT integration

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

```
graph TD
    SENSOR[Light Sensor (ADC)]
    LIGHT[light_sensor_service.c]
    BUS[Message Bus (msg_bus)]
    UPLOADER[data_uploader_service.c]
    UART[UART Service]
    BLE[BLE Service]
    CACHE[Cache System]
    JSON[json_utils.c]
    HTTP[http_post_hal.c]
    CLOUD[Cloud Server]
    MOBILE[nRF Connect / App]

    SENSOR --> LIGHT
    LIGHT --> BUS
    BUS --> UPLOADER
    BUS --> UART
    BUS --> BLE
    UPLOADER --> JSON
    JSON --> HTTP
    HTTP --> CLOUD
    UPLOADER --> CACHE
    CACHE --> UPLOADER
    BLE --> MOBILE
    UART --> PC

```

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

##  License

MIT License — Use freely, modify, and integrate.

---

🛠️ Last Updated: May 1, 2025  
Made with ❤️ by [Greyson Yu](https://github.com/MrRaidrop)
