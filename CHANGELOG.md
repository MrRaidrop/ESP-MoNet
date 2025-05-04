## [Unreleased]

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
