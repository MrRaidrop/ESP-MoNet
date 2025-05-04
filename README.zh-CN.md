# ESP32 模块化物联网框架

[中文版本 README.zh-CN.md](README.zh-CN.md) | [English Version README.md](README.md)

本项目基于 ESP32‑S3 与 ESP‑IDF 5.4，提供完整的模块化嵌入式系统示例。系统集成了 UART、Wi‑Fi、HTTPS 云通信、ADC 光照传感器，并支持 BLE GATT。后续计划加入 MQTT。

[传感器接入指南](#如何添加新传感器)

[![BLE Module CI](https://github.com/MrRaidrop/esp32_ble_mqtt_https_sensors/actions/workflows/ci.yml/badge.svg)](https://github.com/MrRaidrop/esp32_ble_mqtt_https_sensors/actions)

---

## 项目亮点

* 模块化架构 – **HAL / Core / Service / Net** 四层分离，任何模块都可替换、删除或扩展  
* 事件驱动消息总线 – 发布/订阅模式，松耦合、易扩展  
* 安全 OTA 与 HTTPS 上传 – 预配置 HTTPS OTA，未来可接入 AWS IoT Jobs  
* CI 支持 – 已集成 GitHub Actions，示例包含 BLE 单元测试  
* 双通道上传 – Wi‑Fi 与 BLE 自动切换，离线缓存确保数据不丢失  
* 双语文档与 Mermaid 图 – 中英文对照文档与交互式架构图，便于全球协作  

---

## 版本发布

| 版本  | 日期        | 要点                                                         |
|-------|-------------|--------------------------------------------------------------|
| v0.5  | 2025‑05‑03 | 零拷贝相机、二进制缓存、自适应 FPS、新文档结构               |

---

## 功能列表

* Wi‑Fi 自动重连
* 通过 ADC 周期读取光照传感器数据
* 通过 HTTPS 发送 JSON 到服务器
* 数据上传任务默认 5 秒循环，可根据 RSSI 调整间隔
* UART 回显与调试
* BLE GATT 通知移动端传感器数据
* 组件化目录结构：所有源码位于 `components/`
* OTA 固件更新（启动后 30 秒触发）
* 相机（OV2640）JPEG 拍照上传
* 通过 `EVENT_SENSOR_JPEG` 在消息总线上传二进制帧
* 零拷贝 JPEG：`camera_fb_t*` 直接传递，无 `memcpy`
* 离线二进制缓存：PSRAM 环形缓冲，重新上线自动上传
* 动态 FPS：根据 Wi‑Fi RSSI 自动调整帧率
* 预留 MQTT 与自定义传感器扩展

---

## 项目结构

```
project-root
├── components/
│   ├── hal/        # 硬件抽象层
│   ├── core/       # 消息总线等核心机制
│   ├── net/        # 网络（HTTPS，未来 MQTT）
│   ├── OTA/        # OTA 更新
│   ├── service/    # 业务逻辑任务
│   └── utils/      # 通用工具
├── main/           # 应用入口
└── README.md
```

---

## 系统架构图

```mermaid
graph TD
    SENSOR[光照传感器（ADC）]
    DHT[DHT22 温湿度传感器]
    CAMERA[摄像头 camera_service.c<br><small>（Zero-copy JPEG）</small>]
    CAM_HAL[camera_hal.c]
    LIGHT[light_sensor_service.c]
    DHT_SERVICE[dht22_service.c]
    BUS[消息总线 msg_bus]
    ENCODER[json_encoder.c<br><small>统一 JSON 编码</small>]
    UPLOADER[data_uploader_service.c]
    UART[UART 服务]
    BLE[BLE 服务]
    CACHE[缓存系统<br><small>二进制环形缓冲</small>]
    HTTP[http_post_hal.c]
    CLOUD[云服务器]
    MOBILE[手机 App（如 nRF Connect）]
    PC[电脑终端]

    SENSOR --> LIGHT
    DHT --> DHT_SERVICE
    LIGHT --> BUS
    DHT_SERVICE --> BUS
    CAMERA --> BUS
    CAM_HAL --> CAMERA
    BUS --> UPLOADER
    BUS --> UART
    BUS --> BLE
    UPLOADER --> ENCODER
    ENCODER --> HTTP
    HTTP --> CLOUD
    UPLOADER --> CACHE
    CACHE --> UPLOADER
    BLE --> MOBILE
    UART --> PC


---

## 入门指南

参见英文版 README 的 Getting Started 部分或 GitHub Pages 文档：  
<https://mrraidrop.github.io/ESP-MoNet/>  

默认配置已写入 `sdkconfig.default`，克隆后可直接 `idf.py build flash monitor`。

---

## JSON 上传格式

所有 `msg_t` 消息通过 `json_encoder_encode()` 统一转换为 JSON，并依网络状况经 HTTPS 或 BLE 发送。

光照示例：

```json
{
  "type": "light",
  "value": 472,
  "ts": 1713302934
}
```

温湿度示例：

```json
{
  "type": "temp",
  "temperature": 24.65,
  "humidity": 63.10,
  "ts": 1713302971
}
```

如需添加新格式，请在 `msg_bus.h` 扩展 `msg_t`，并在 `json_encoder.c` 增加相应 `case`。无需改动上传逻辑。

---

## 路线图

### 功能概览

| 功能                          | 状态           | 备注                                           |
|-------------------------------|----------------|------------------------------------------------|
| ADC 光照驱动                  | 已完成         | 1 秒采样，经 msg_bus 发布                      |
| JSON 编码工具                 | 已完成         | 统一 `type+value+timestamp`                    |
| HTTPS 上传                    | 已完成         | 模块化 `http_post_hal()`                       |
| BLE GATT 通知                 | 已完成         | Subscribe msg_bus → `notify_raw()`             |
| UART 转发                     | 已完成         | UART 订阅 msg_bus                              |
| 上传重试缓存                  | 已完成         | RAM 环形缓冲                                   |
| 模块化任务架构                | 已完成         | 每个 Service 独立 FreeRTOS 任务                |
| OTA (Wi‑Fi)                   | 已完成         | `esp_https_ota()`                              |
| 文档与 CI                     | 已完成         | README、架构图、GitHub Actions                 |
| 相机 JPEG                     | 已完成         | OV2640 + `camera_hal`                          |
| 零拷贝 JPEG                   | 已完成         | `camera_fb_t*` 直通 msg_bus                    |
| 离线 JPEG 缓存                | 已完成         | PSRAM 缓冲 + 自动重传                          |
| 动态 FPS                      | 已完成         | 根据 RSSI 调整间隔                             |
| MQTT 安全上传                 | 计划中         | TLS MQTT Broker                                |
| OTA (BLE)                     | 计划中         | BLE‑based OTA                                  |
| DMA + 高速 Ring Buffer        | 计划中         | 超声波等高速传感器                             |
| Flutter 移动端 UI             | 计划中         | 实时 BLE Dashboard                             |

### 已知限制与改进计划

| 分类          | 问题描述                 | 改进方向                                   | 状态         |
|---------------|--------------------------|--------------------------------------------|--------------|
| 架构          | 无统一 Service 生命周期  | 增加 `service_registry` 和 `app_init`      | 进行中       |
| 配置系统      | 配置硬编码在 `.c`        | 使用 Kconfig + NVS                         | 计划中       |
| 日志          | 无模块级别控制           | 引入 `LOG_MODULE_REGISTER`                 | 进行中       |
| 单元测试      | 仅 BLE 工具已测          | 增加 `json_encoder`、cache、uploader 测试  | 进行中       |
| HTTPS 安全    | 未验证服务器证书         | 添加 CA 证书校验选项                        | 计划中       |
| OTA 机制      | 缺乏回滚验证             | SHA256 校验 + 双分区回滚                   | 计划中       |
| BLE 扩展      | 仅单通道 Notify          | 扩展可写特征值                             | 进行中       |

---

## 示例应用场景

* Wi‑Fi JPEG 相机节点，离线缓存防丢帧  
* BLE 环境监测仪，实时通知手机  
* 图像与传感器混合上传的边缘节点  
* 可扩展的自定义 IoT 原型平台  
* 动态帧率的低功耗边缘计算节点  
* 支持 OTA 的现场测试平台  

---

## 如何添加新传感器

完整步骤请参阅 `docs/how_to_add_sensor.md`。核心流程：

1. 创建 HAL 驱动（GPIO/时序）  
2. 编写 Service 任务，发布 `msg_t`  
3. 在 `msg_bus.h` 扩展结构，在 `json_encoder.c` 增加编码逻辑  
4. 更新各自组件的 `CMakeLists.txt`

---

## 相机模块深入

详细介绍请阅读 [docs/camera_module.md](docs/camera_module.md)。

---

## 许可证

MIT License — 欢迎使用、修改和集成。
