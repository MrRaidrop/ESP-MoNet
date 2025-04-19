#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// 启动光敏任务服务（自动创建 FreeRTOS 任务）
void light_sensor_service_start(void);
int light_sensor_get_cached_value(void);  // 供其他模块调用获取最新光照值

#ifdef __cplusplus
}
#endif