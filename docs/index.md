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
    DHT22["DHT22 Sensor(can be any sensor you add)"]
    LIGHT["light_sensor_service.c"]
    TEMP["dht22_service.c"]
    CAMERA["camera_service.c<br><small>(Zero-copy JPEG)</small>"]
    CAM_HAL["camera_hal.c"]
    BUS["Message Bus (msg_bus)"]
    UPLOADER["data_uploader_service.c"]
    UART["UART Service"]
    BLE["BLE Service"]
    CACHE["Cache System<br><small>(Binary Ring Buffer)</small>"]
    ENCODER["json_encoder.c"]
    HTTP["http_post_hal.c"]
    CLOUD["Cloud Server"]
    MOBILE["nRF Connect / App"]
    PC["PC Terminal"]

    %% Sensor flow
    SENSOR --> LIGHT
    DHT22 --> TEMP
    LIGHT  --> BUS
    TEMP   --> BUS
    CAMERA --> BUS
    CAM_HAL --> CAMERA

    %% Msg bus distribution
    BUS    --> UPLOADER
    BUS    --> UART
    BUS    --> BLE

    %% Upload path
    UPLOADER --> ENCODER
    ENCODER --> HTTP
    HTTP   --> CLOUD

    %% Fallback & edge options
    UPLOADER -->|on fail| CACHE
    CACHE -->|retry| UPLOADER
    BLE    --> MOBILE
    UART   --> PC
</div>
