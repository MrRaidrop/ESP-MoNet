---
layout: default
title: How to Add a Sensor
---

<script src="https://cdn.jsdelivr.net/npm/mermaid/dist/mermaid.min.js"></script>
<script>
  mermaid.initialize({ startOnLoad: true });
</script>

# How to Add a New Sensor

This guide walks you through integrating a new sensor (e.g., DHT22 for temperature/humidity) into the system.  
The project uses a clean, modular structure — all sensors follow the **3-step rule**.

---

## 1. Create a HAL Driver

**Location:** `components/my_hal/dht22_hal.[ch]`

```c
/// dht22_hal.h

/**
 * @brief Initialize DHT22 GPIO and timing
 */
esp_err_t dht22_hal_init(void);

/**
 * @brief Read temperature and humidity from DHT22 sensor
 * 
 * @param out_temp_deg_c Pointer to float storing temperature in °C
 * @param out_humidity_pct Pointer to float storing relative humidity in %
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t dht22_hal_read(float *out_temp_deg_c, float *out_humidity_pct);
```

The HAL should only handle **raw hardware access** (GPIO, timing, etc). Return stub values if testing without sensor.

---

## 2. Add a Sensor Service

**Location:** `components/service/dht22_service.[ch]`

```c
void dht22_service_start(void)
{
    if (dht22_hal_init() != ESP_OK) {
        LOGE("DHT22_SERVICE", "Failed to init HAL");
        return;
    }

    xTaskCreate([](void *) {
        while (1) {
            float temp = 0, hum = 0;
            if (dht22_hal_read(&temp, &hum) == ESP_OK) {
                msg_t msg = {
                    .topic = EVENT_SENSOR_TEMP,
                    .ts_ms = esp_log_timestamp(),
                };
                snprintf(msg.data.json_str, sizeof(msg.data.json_str),
                         "{\"type\":\"temp\",\"t\":%.2f,\"h\":%.2f}", temp, hum);
                msg_bus_publish(&msg);
            }
            vTaskDelay(pdMS_TO_TICKS(10000));
        }
    }, "dht22_task", 4096, NULL, 5, NULL);
}
```

You can publish either `.json_str` or raw `.value` data depending on your message model.

---

## 3. Add to CMake

Ensure you edit `CMakeLists.txt`:

- In `components/my_hal/CMakeLists.txt`, add:
  ```cmake
  srcs += src/dht22_hal.c
  ```

- In `components/service/CMakeLists.txt`, add:
  ```cmake
  srcs += src/dht22_service.c
  ```

---

## 4. Extend JSON Encoding

To support cloud upload for your new sensor,
you no longer need to snprintf() JSON strings manually.

Instead, the system uses a centralized encoder:

```c
#include "codec/json_encoder.h"

char json_buf[256];
json_encoder_encode(&msg, json_buf, sizeof(json_buf));
```

To support your sensor:

Add a new topic (e.g. EVENT_SENSOR_TEMP) in msg_bus.h

Extend the msg_t union with a matching data struct (e.g. temp_hum)

Add a new case block in json_encoder_encode() in json_encoder.c:

```c
case EVENT_SENSOR_TEMP:
    snprintf(out_buf, buf_size,
        "{ \"type\": \"temp\", \"temperature\": %.2f, \"humidity\": %.2f, \"ts\": %" PRIu32 " }",
        msg->data.temp_hum.temperature,
        msg->data.temp_hum.humidity,
        msg->ts_ms);
    return true;
```

No further change is needed in uploader or cache,
your new sensor will be automatically handled by both Wi-Fi and BLE.

---

## Example JSON Output

```json
{
  "type": "temp",
  "t": 24.65,
  "h": 52.1,
  "ts": 3432943
}
```

---

## Optional BLE Notification

To notify mobile devices via BLE, just:

1. Subscribe to `EVENT_SENSOR_TEMP` in `ble_service.c`
2. Format & call `notify_raw()` with the same JSON string

---


---

## 5.Modify `msg_t` Structure

To support new sensor data types (like temperature and humidity), you should update the `msg_t` definition in `core/msg_bus.h`:

```c
typedef struct {
    uint32_t ts_ms;
    union {
        struct {
            float t;  ///< temperature in °C
            float h;  ///< relative humidity %
        } temp;

        struct {
            int adc_value;
        } light;

        struct {
            camera_fb_t *fb;
        } jpeg;

        // Add more types here as needed
    } data;
    event_topic_t topic;
} msg_t;
```


## OK, Done!

You’ve added a new sensor in just **3 files + 2 CMake lines**.  
To add more (e.g. CO₂, PIR, gas, tilt), just repeat the pattern:

- HAL driver (`my_hal/`)
- Service module (`service/`)
- JSON/Notify integration (optional)

