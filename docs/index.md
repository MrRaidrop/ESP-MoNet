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

我先把组件跑通，这个camera的测试方式，我是准备让他发给msgbus,项目文件里也有，但是问题就是我需要一个bool msg_bus_subscribe(msg_topic_t topic, QueueHandle_t queue)
{
    if (!queue || topic >= EVENT_SENSOR_MAX) return false;

    if (bus_mutex == NULL) bus_mutex = xSemaphoreCreateMutex();
    xSemaphoreTake(bus_mutex, portMAX_DELAY);

    for (int i = 0; i < MAX_SUBSCRIBERS_PER_TOPIC; ++i) {
        if (subscriber_queues[topic][i] == NULL) {
            subscriber_queues[topic][i] = queue;
            xSemaphoreGive(bus_mutex);
            LOGI(TAG, "Subscribed to topic %d", topic);
            return true;
        }
    }

    xSemaphoreGive(bus_mutex);
    LOGW(TAG, "Topic %d has too many subscribers", topic);
    return false;
}这是我目前的subscibe,我希望能管理这些组件，比如现在camera我想让他从uart串口发送，让串口去订阅camera？我其实还有http post和ble post, 也就是sensor的data通过msgbus的订阅跑到uart上，我这个msgbus的逻辑应该怎么改，弄一个table？还是提供注册函数，然后在main里注册？

你推荐我在uart http post ble post （以及未来可能的mqttpost）这几种service添加队列，然后自己去subscribe这个sensor,但是我有一个问题，我项目的初衷是让别人能无痛，最小改动的添加自己的sensor功能，如果他添加一个sensor,然后还要在uart或者ble里加上subscribe,就很烦，我希望能统一管理谁订阅谁