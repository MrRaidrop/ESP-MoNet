# ESP32 Wi-Fi UART HTTPS Project Template

This project is a modular embedded system template based on the ESP32 platform, featuring the following functionalities:

- UART data reception with automatic echo
- Periodic HTTPS POST (every 5 seconds) to a cloud server with JSON payload
- JSON includes UART data, timestamp, and a counter
- Automatic Wi-Fi reconnection mechanism
- Fully modular codebase, easy to extend and maintain

---

## 🔧 Functional Modules

| Module        | File(s)                  | Description                                      |
|---------------|--------------------------|--------------------------------------------------|
| UART Driver   | `uart_handler.c/h`       | Initializes UART1, receives serial data, provides write interface |
| Wi-Fi Manager | `wifi_manager.c/h`       | Handles Wi-Fi connection with event group support |
| JSON Builder  | `json_utils.c/h`         | Packages UART data with timestamp into JSON      |
| HTTPS Upload  | `https_client/https_post.c/h` | Sends JSON payload to server via HTTPS POST     |
| Main Logic    | `main.c`                 | Application entry point; coordinates task startup |

---

## 📁 Project Structure

```
main/
├── main.c
├── uart_handler.c
├── uart_handler.h
├── wifi_manager.c
├── wifi_manager.h
├── json_utils.c
├── json_utils.h
├── https_client/
│   ├── https_post.c
│   └── https_post.h
├── log_wrapper.h      // Optional: macro-based logging control
CMakeLists.txt
README.md
```

---

## 🚀 Build & Run

### 1. Environment Setup

Use [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/index.html) version 5.4 or compatible. It’s recommended to use `esp-idf-tools` for environment configuration.

```bash
cd hello_world
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

### 2. Wi-Fi Configuration

Edit the `main.c` file:

```c
#define WIFI_SSID      "your-ssid"
#define WIFI_PASS      "your-password"
```

---

## 📡 Server Upload Configuration

The HTTPS POST destination is defined in `https_client/https_post.c`:

```c
.url = "https://40.233.83.32:8443/data"
```

> ✅ HTTPS supported. For testing purposes, certificate verification is currently disabled.

---

## 🔄 JSON Payload Example

A JSON packet is uploaded every 5 seconds with the following structure:

```json
{
  "esp32": "2025-04-11 14:23:52",
  "uart_data": "Hello from UART",
  "hello": 23
}
```

---

## 📦 Planned Feature Roadmap (in progress)

The project will be extended in future stages with additional modules. Features will be developed and tested incrementally:

### ✅ Sensor Data Acquisition & Caching
- [ ] Use **I2C** to connect two sensors: ultrasonic distance + temperature/humidity (e.g., SHT31 or DHT12)
- [ ] Use **DMA + ring buffer** for non-blocking data collection
- [ ] Implement **half-full DMA interrupt** for mid-buffer processing and full-buffer trigger for upload
- [ ] All data packets include **timestamp** and unified JSON format

### ✅ BLE Module (Mobile Communication)
- [ ] Implement ESP32 BLE GATT to broadcast and allow reading of recent data
- [ ] Develop a **Flutter mobile app**, including emulator support to:
  - Display connection status
  - Show real-time sensor data and timestamps
- [ ] Evaluate BLE performance (throughput, frame size, stability)

### ✅ MQTTs Cloud Integration
- [ ] Add MQTTs client (TLS enabled) on ESP32
- [ ] Buffer and upload data to an MQTT broker (e.g., `mqtts://example.com:8883`)
- [ ] Subscribe and process incoming data using Linux tools (`mosquitto_sub`, etc.)

### ✅ System Integration Phase
- [ ] Complete pipeline: DMA acquisition → JSON packaging → MQTTs upload
- [ ] Synchronize BLE broadcasting with the latest sensor data
- [ ] Add fault recovery (Wi-Fi reconnection, buffer overflow protection, etc.)

---

🛠 Current Status: *Modular design and initial framework completed*  
📅 Last Updated: 2025-04-11

---

## 📜 License

MIT License – Free to use, study, and modify. Feel free to fork this project as a base for your own ESP32 development!
