---
layout: default
title: Camera Module Architecture
---

<script src="https://cdn.jsdelivr.net/npm/mermaid/dist/mermaid.min.js"></script>
<script>
  mermaid.initialize({ startOnLoad: true });
</script>

<h2>Camera Module Architecture</h2>

<div class="mermaid">
graph TD
    CAMERA["camera_service.c<br><small>📷 Zero-copy JPEG</small>"]
    CAM_HAL["camera_hal.c"]
    BUS["Message Bus (msg_bus)"]
    UPLOADER["data_uploader_service.c<br><small>HTTP uploader</small>"]
    CACHE["Cache System<br><small>Binary Ring Buffer</small>"]
    HTTP["http_post_hal.c"]
    CLOUD["Cloud Server"]

    CAMERA --> BUS
    CAM_HAL --> CAMERA
    BUS --> UPLOADER
    UPLOADER --> HTTP
    HTTP --> CLOUD

    %% Highlight JPEG path
    CAMERA -.->|camera_fb_t*| UPLOADER
    UPLOADER -->|on fail| CACHE
    CACHE -->|retry when Wi-Fi OK| UPLOADER
</div>

<p><strong>Key Features:</strong></p>
<ul>
  <li><strong>Zero-copy</strong> JPEG streaming with <code>camera_fb_t*</code> passing</li>
  <li><strong>Dynamic FPS</strong> adapts to Wi-Fi RSSI automatically</li>
  <li><strong>Offline binary cache</strong> stores frames during disconnection</li>
  <li><strong>Loose coupling</strong> via message bus: easy to extend & swap upload backend</li>
</ul>

<p>For full details, see <a href="../camera_module.md">camera_module.md</a>.</p>