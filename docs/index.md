---
layout: default
title: System Architecture
---

<script src="https://cdn.jsdelivr.net/npm/mermaid/dist/mermaid.min.js"></script>
<script>
  mermaid.initialize({ startOnLoad: true });
</script>

<h2>System Architecture</h2>

<div class="mermaid">
graph TD
    SENSOR["Light Sensor (ADC)"]
    LIGHT["light_sensor_service.c"]
    CAMERA["camera_service.c<br><small>(Zero-copy JPEG)</small>"]
    CAM_HAL["camera_hal.c"]
    BUS["Message Bus (msg_bus)"]
    UPLOADER["data_uploader_service.c"]
    UART["UART Service"]
    BLE["BLE Service"]
    CACHE["Cache System<br><small>(Binary Ring Buffer)</small>"]
    JSON["json_utils.c"]
    HTTP["http_post_hal.c"]
    CLOUD["Cloud Server"]
    MOBILE["nRF Connect / App"]
    PC["PC Terminal"]

    SENSOR --> LIGHT
    LIGHT  --> BUS
    CAMERA --> BUS
    CAM_HAL --> CAMERA
    BUS    --> UPLOADER
    BUS    --> UART
    BUS    --> BLE
    UPLOADER --> JSON
    JSON   --> HTTP
    HTTP   --> CLOUD
    BLE    --> MOBILE
    UART   --> PC

    %% Highlight JPEG path
    CAMERA -.->|camera_fb_t*| UPLOADER
    UPLOADER -->|on fail| CACHE
    CACHE -->|retry when Wi-Fi OK| UPLOADER
</div>
