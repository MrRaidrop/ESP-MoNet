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
    BUS["Message Bus (msg_bus)"]
    UPLOADER["data_uploader_service.c"]
    UART["UART Service"]
    BLE["BLE Service"]
    CACHE["Cache System"]
    JSON["json_utils.c"]
    HTTP["http_post_hal.c"]
    CLOUD["Cloud Server"]
    MOBILE["nRF Connect / App"]
    PC["PC Terminal"]

    SENSOR --> LIGHT
    LIGHT  --> BUS
    BUS    --> DATA_UPLOADER
    BUS    --> UART
    BUS    --> BLE
    UPLOADER --> JSON
    JSON   --> HTTP
    HTTP   --> CLOUD
    UPLOADER <--> CACHE
    BLE    --> MOBILE
    UART   --> PC
</div>
