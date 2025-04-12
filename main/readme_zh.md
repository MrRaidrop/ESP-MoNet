
# ESP32 Wi-Fi UART HTTPS 项目模板

本项目是一个基于 ESP32 的模块化嵌入式工程示例，集成了以下功能：

- 使用 UART 读取串口数据并自动回显
- 每隔 5 秒通过 HTTPS 向云端服务器上传 JSON 数据
- JSON 包含串口数据、时间戳和计数器
- Wi-Fi 自动重连机制
- 完整模块分离，便于扩展和维护

---

## 🔧 项目功能模块

| 模块 | 文件 | 功能说明 |
|------|------|-----------|
| 串口驱动 | `uart_handler.c/h` | 初始化 UART1，接收串口数据，提供写入函数 |
| Wi-Fi 管理 | `wifi_manager.c/h` | 初始化并连接 Wi-Fi，提供事件组等待机制 |
| JSON 构造 | `json_utils.c/h` | 将串口数据封装为带时间戳的 JSON 报文 |
| HTTPS 上传 | `https_client/https_post.c/h` | 通过 HTTPS POST 上传 JSON 到云服务器 |
| 主入口 | `main.c` | 启动各个模块任务，协调数据上传 |

---

## 📁 目录结构

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
├── log_wrapper.h      // 可选，用于控制日志开关
CMakeLists.txt
README.md
```

---

## 🚀 构建与运行

### 1. 准备环境

请使用 [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/index.html) v5.4 或兼容版本。建议通过 `esp-idf-tools` 配置环境。

```bash
cd hello_world
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

### 2. 修改 Wi-Fi 配置

在 `main.c` 中：

```c
#define WIFI_SSID      "你的SSID"
#define WIFI_PASS      "你的密码"
```

---

## 📡 上传服务器配置

当前上传地址写在 `https_client/https_post.c` 中：

```c
.url = "https://40.233.83.32:8443/data"
```

> ✅ 支持 HTTPS，自签名证书可用，默认不设置 CA 认证（仅测试环境使用）。

---

## 🔄 数据结构示例

每隔 5 秒，发送如下格式的 JSON 到服务器：

```json
{
  "esp32": "2025-04-11 14:23:52",
  "uart_data": "Hello from UART",
  "hello": 23
}
```

---

## 📦 后续功能开发规划（持续更新中）

本项目未来将逐步集成以下功能模块，分阶段开发与验证：

### ✅ 数据采集与缓存模块
- [ ] 使用 **I2C** 连接两个传感器：超声波测距 + 温湿度（如 SHT31 或 DHT12）
- [ ] 使用 **DMA + Ring Buffer** 实现无阻塞数据采集
- [ ] 实现 **DMA 半满中断**进行中间处理，满缓冲时上传
- [ ] 所有数据打上 **本地时间戳**，统一格式封装为 JSON

### ✅ BLE 模块（移动端通信）
- [ ] 使用 ESP32 BLE GATT 实现数据广播与特征值读取
- [ ] 使用 **Flutter 开发手机 App**，模拟器可运行：
  - 查看连接状态
  - 实时查看数据和时间戳
- [ ] 测试 BLE 收发性能与最大传输帧大小

### ✅ MQTTs 云服务模块
- [ ] 在 ESP32 端添加 MQTTs 客户端（带 TLS 证书）
- [ ] 数据缓冲上传至 MQTT 服务器（如 `mqtts://example.com:8883`）
- [ ] Linux 本地终端订阅并处理这些数据（如使用 `mosquitto_sub`）

### ✅ 项目集成阶段（每模块验证通过后）
- [ ] 将 DMA 数据流 → JSON 打包 → MQTTs 上传链路全流程闭环
- [ ] 同步 BLE 广播最近一包数据到手机
- [ ] 加入异常处理逻辑（掉线重连、缓冲溢出等）

---

🛠 当前进度：*模块设计与初步框架搭建中*  
📅 更新时间：2025-04-11

---

## 📜 License

MIT License - 自由使用、学习与修改。欢迎 fork 本项目作为你自己的 ESP32 项目模板！

---
