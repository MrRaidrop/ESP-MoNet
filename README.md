# 🌐 ESP-MoNet — Modular ESP32-S3 IoT Framework

[![Module CI](https://github.com/MrRaidrop/ESP-MoNet/actions/workflows/ci.yml/badge.svg)](https://github.com/MrRaidrop/ESP-MoNet/actions)

A service-oriented embedded framework on **ESP-IDF 5.4 / ESP32-S3**. Every capability —
light sensor, camera, UART, HTTPS upload, BLE notify — is an independent **service** that
auto-registers with a **service registry** and talks over a **message bus** (FreeRTOS
queues, publish/subscribe). Adding a sensor is one HAL + one service file; adding a cloud
sink is one subscriber. The same bus fans data out to Wi-Fi HTTPS, BLE GATT and UART.

## Features

- **Camera** — live MJPEG stream served by the board at `http://<board-ip>/` (OV2640, QVGA);
  zero-copy JPEG capture (`camera_fb_t*` passthrough with an explicit release hook).
- **Message bus + service registry** — each service runs as its own FreeRTOS task with its
  own stack and priority; publishers fan out to subscriber sinks over a pub/sub bus.
- **Networking** — HTTPS upload of JSON telemetry and JPEG, verified against an embedded
  self-signed CA; OTA over HTTPS (SHA-256, dual-OTA partitions).
- **Sensors / IO** — ADC light sensor, DHT22, UART, BLE GATT notify.
- **Optional C++ layer** (`CONFIG_MONET_CPP_EXPERIMENTAL`, off by default) — RAII and
  templates over the FreeRTOS primitives; see [docs/cpp_refactor.md](docs/cpp_refactor.md).

## Live camera stream

With the camera enabled (OV2640 + PSRAM), open **`http://<board-ip>/`** in a browser for a
live MJPEG stream served straight off the device — one persistent connection, no per-frame
handshake. The board's IP is printed in the boot log.

> The OV2640 XCLK runs at **24 MHz, not 20** — a 20 MHz clock's harmonics fall inside every
> 2.4 GHz Wi-Fi channel and throttle the radio (~280× drop). See `monet_hal/.../camera_hal.h`.

## Quick start

```bash
. $IDF_PATH/export.sh            # ESP-IDF v5.4.x installed & sourced
./setup.sh [SERVER_IP]           # generates server cert, set-target, applies defaults
# edit Wi-Fi creds in sdkconfig (CONFIG_WIFI_SSID / CONFIG_WIFI_PASSWORD)
idf.py -p <PORT> build flash monitor
```

Full guide: [docs/getting_started.md](docs/getting_started.md).

## Architecture

The **service registry** owns task lifecycle; the **msg_bus** routes typed `msg_t` messages
from publishers to subscriber sinks. A subscriber is either its own task or a `sink_cb`
callback driven by a shared dispatch task.

[Interactive diagram](https://mrraidrop.github.io/ESP-MoNet/) ·
[C → C++ design notes](docs/cpp_refactor.md)

```
components/
├── monet_hal/    # HAL: ADC, UART, Wi-Fi, camera
├── monet_core/   # msg_bus, service_registry  (+ optional C++ layer)
├── net/          # HTTPS upload
├── OTA/          # HTTPS OTA
├── service/      # light, camera, mjpeg stream, uart, ble, http uploader, dht22
└── utils/        # cache, json, logging
main/             # application entry
server/           # companion HTTPS server (/data /image /firmware) + image viewer
```

## Data format

Telemetry is uploaded as unified JSON produced by `json_encoder`, e.g.
`{"type":"light","value":472,"ts":1713302934}`. Adding a new sensor is a 3-step,
no-other-services-touched change: [docs/how_to_add_sensor.md](docs/how_to_add_sensor.md).

## Docs

| | |
|---|---|
| [Getting started](docs/getting_started.md) | Build, configure, flash |
| [Add a sensor](docs/how_to_add_sensor.md) | The 3-step extension flow |
| [Camera deep dive](docs/camera_module.md) | Zero-copy, adaptive, offline-resilient |
| [C / C++ refactor notes](docs/cpp_refactor.md) | RAII, templates, interfaces |
| [Companion server](server/README_SERVER.md) | Data/image/OTA endpoints |
| [Changelog](CHANGELOG.md) | Release history |

## License

MIT — use freely, modify, integrate. Made by [Zhenghao Yu](https://github.com/MrRaidrop).
