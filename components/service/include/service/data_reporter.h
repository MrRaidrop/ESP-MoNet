#ifndef DATA_REPORTER_H
#define DATA_REPORTER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 周期性采集光照数据并通过 HTTPS 上报至服务器
 *
 * @details 本任务在确保 Wi-Fi 连接成功后启动，每隔固定时间（默认 5 秒）：
 *          - 从 light_sensor_service 中获取光照值
 *          - 格式化为 JSON 字符串（使用 build_json_payload）
 *          - 通过 https_post 发送到服务器
 *
 * @note 若 Wi-Fi 连接失败，则任务自动退出。
 *
 * @param pvParameters 未使用，可设为 NULL
 */

void data_reporter_start(void);

#ifdef __cplusplus
}
#endif

#endif
