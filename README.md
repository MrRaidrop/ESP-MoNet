# 🌐 ESP32 Modular IoT Framework

A fully modular embedded system project built on ESP32-S3 using ESP-IDF 5.4. The system integrates UART, Wi-Fi, HTTPS cloud communication, ADC-based light sensor, and future support for MQTT and BLE mobile interaction.

---

##  Features

-  Auto-reconnecting Wi-Fi connection manager
-  Periodic light sensor reading via ADC
-  Secure HTTPS POST to cloud with JSON data
-  Data reporter module with 5-second upload loop
-  UART echo for sensor debugging
-  Modular source structure for scalability
-  Future-ready for BLE & MQTT integration

---

##  Project Structure

```
main/
├── bsp/                    # Hardware drivers (e.g., ADC light sensor)
│   ├── light_sensor_driver.c/h
│
├── https_client/          # HTTPS POST module
│   ├── https_post.c/h
│
├── service/               # Runtime service modules
│   ├── data_reporter.c/h          # Collect + upload data
│   ├── light_sensor_service.c/h   # Manage light sensor logic + caching
│   ├── uart_handler.c/h           # UART handling
│
├── utils/                 # Utility modules
│   ├── json_utils.c/h             # Build JSON payload
│
├── wifi_manager.c/h       # Wi-Fi connection logic
├── main.c                 # Top-level startup entry
├── log_wrapper.h          # Optional logging macro
├── README.md              # You're reading it!
```

---

##  Current Architecture

```mermaid
graph TD
    SENSOR[Light Sensor (ADC)]
    SERVICE[light_sensor_service.c]
    REPORT[data_reporter.c]
    JSON[json_utils.c]
    POST[https_post.c]
    CLOUD[Cloud Server]
    UART[UART Handler]
    MONITOR[Serial Output]

    SENSOR --> SERVICE
    SERVICE --> REPORT
    REPORT --> JSON
    JSON --> POST
    POST --> CLOUD
    SERVICE --> UART
    UART --> MONITOR
```

---

## 🔧 Getting Started

### 1. Prerequisites

- ESP-IDF 5.4+ installed and configured
- Supported board: ESP32-S3 devkit
- Internet access for cloud upload

### 2. Build and Flash

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

### 3. Wi-Fi Configuration

Update your SSID and password in `data_reporter.c`:

```c
#define WIFI_SSID "your-ssid"
#define WIFI_PASS "your-password"
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

## 🚀 Roadmap

| Feature                         | Status        | Notes                                  |
|----------------------------------|---------------|----------------------------------------|
| Light sensor ADC driver         | ✅ Done        | Caches latest value every second       |
| JSON packaging utility          | ✅ Done        | Can be adapted for MQTT/BLE            |
| HTTPS POST to cloud             | ✅ Done        | JSON content, no CA cert required      |
| Modular task architecture       | ✅ Done        | Using FreeRTOS tasks                   |
| GitHub repo + documentation     | ✅ Done        | Modular code + diagram                 |
| DMA + Ring Buffer integration   | 🔜 Planned     | For ultrasonic / high-rate sensor      |
| MQTT secure upload              | 🔜 Planned     | Add TLS MQTT broker support            |
| BLE GATT + mobile app           | 🔜 Planned     | Using Flutter + ESP32 BLE              |
| OTA update integration          | ⏳ In Progress | Optional for remote firmware updates   |

---

## 🧪 Example Use Cases

- Low-power sensor node with cloud logging
- BLE + UART + MQTT hybrid IoT edge device
- Sensor/actuator hub with REST and mobile access
- ESP32 data pipeline demo for job portfolio

---

## 📜 License

MIT License — Use freely, modify, and integrate.

---

🛠️ Last Updated: April 11, 2025  
Made with ❤️ by [Greyson Yu](https://github.com/MrRaidrop)

