# 🌐 ESP32 Modular IoT Framework 

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
│   ├── bsp/                 # Hardware drivers (Wi‑Fi, UART, ADC light sensor)
│   │   ├── include/bsp/*.h
│   │   └── src/*.c
│   ├── net/                 # Networking sub‑system (HTTPS POST, future MQTT)
│   │   ├── include/net/*.h
│   │   └── src/https_post.c
│   ├── OTA/                 # Wi‑Fi OTA service
│   │   ├── include/OTA/*.h
│   │   └── src/https_ota_service.c
│   ├── service/             # Runtime logic (FreeRTOS tasks)
│   │   ├── include/service/*.h
│   │   └── src/*.c          # ble_service.c, data_reporter.c, ...
│   └── utils/               # Helper utilities
│       ├── include/utils/*.h
│       └── src/*.c          # json_utils.c, ble_format_utils.c
│
├── main/                    # Application entry
│   ├── main.c
│   └── CMakeLists.txt
│
├── README.md  (you are reading~)
└── .github/workflows/ci.yml # GitHub Actions: build & unit‑test BLE module

```

---

##  Current Architecture

```
graph TD
    SENSOR[Light Sensor (ADC)]
    SERVICE[light_sensor_service.c]
    REPORT[data_reporter.c]
    JSON[json_utils.c]
    POST[https_post.c]
    CLOUD[Cloud Server]
    UART[UART Handler]
    MONITOR[Serial Output]
    BLE[BLE GATT Server]
    PHONE[Mobile App (e.g., nRF Connect)]

    SENSOR --> SERVICE
    SERVICE --> REPORT
    REPORT --> JSON
    JSON --> POST
    POST --> CLOUD
    SERVICE --> UART
    UART --> MONITOR
    SERVICE --> BLE
    BLE --> PHONE
```

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

Update your SSID and password in `main/main.c`:

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

##  JSON Upload Format

Data is uploaded every 5 seconds with structure like:

```json
{
  "esp32": "2025-04-11 14:23:52",
  "uart_data": "light: 472",
  "hello": 23
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

| Feature                         | Status        | Notes                                                |
|----------------------------------|---------------|------------------------------------------------------|
| Light sensor ADC driver         | ✅ Done        | Caches latest value every second                     |
| JSON packaging utility          | ✅ Done        | Can be adapted for MQTT/BLE                          |
| HTTPS POST to cloud             | ✅ Done        | JSON content, no CA cert required                    |
| Modular task architecture       | ✅ Done        | Using FreeRTOS tasks                                 |
| GitHub repo + documentation     | ✅ Done        | Modular code + diagram                               |
| **BLE GATT notification**       | ✅ Done        | Sends int light value every 3 seconds                |
| DMA + Ring Buffer integration   | 🔜 Planned     | For ultrasonic / high-rate sensor                    |
| MQTT secure upload              | ⏳ In Progress | Add TLS MQTT broker support                          |
| OTA update (Wi-Fi)              | ✅ Done        | HTTPS OTA support using esp_https_ota                |
| OTA update (BLE)                | 🔜 Planned     | Plan to implement BLE-based OTA update               |
| Flutter mobile app (sensor UI)  | 🔜 Planned     | Flutter app to visualize sensor data via BLE         |
| Flutter mobile app (BLE OTA)    | 🔜 Planned     | BLE OTA logic integrated into the Flutter application|

---

##  Example Use Cases

- Low-power sensor node with cloud logging
- BLE + UART + MQTT hybrid IoT edge device
- Sensor/actuator hub with REST and mobile access

---

##  License

MIT License — Use freely, modify, and integrate.

---

🛠️ Last Updated: April 19, 2025  
Made with ❤️ by [Greyson Yu](https://github.com/MrRaidrop)
