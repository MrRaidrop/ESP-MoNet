#include "ble_service.h"
#include "esp_log.h"
#include "esp_gatts_api.h"
#include "esp_gap_ble_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

extern int light_sensor_get_cached_value(void);

#define TAG "BLE_SERVICE"

#define GATTS_SERVICE_UUID        0x00FF
#define GATTS_CHAR_UUID_RX        0xFF01
#define GATTS_CHAR_UUID_TX        0xFF02
#define GATTS_NUM_HANDLES         6
#define PROFILE_APP_ID            0
#define DEVICE_NAME               "ESP32-BLE"

static uint16_t gatt_if;
static uint16_t conn_id = 0xFFFF;
static bool is_connected = false;
static uint16_t tx_handle = 0;
static bool adv_data_ready = false;  // 标记广播数据是否准备好

// 全局广播 UUID 数据（必须全局，防止地址失效）
static uint8_t service_uuid[2] = {0xFF, 0x00};

// 广播数据（必须全局）
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x20,
    .max_interval = 0x40,
    .appearance = 0x00,
    .manufacturer_len = 0,
    .p_manufacturer_data = NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = 2,
    .p_service_uuid = service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

// 广播参数（可以全局复用）
static esp_ble_adv_params_t adv_params = {
    .adv_int_min        = 0x20,
    .adv_int_max        = 0x40,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

// GATT 属性数据库 UUID 定义
static const uint8_t uuid_primary_service[] = {0x00, 0x28};
static const uint8_t uuid_char_declare[]    = {0x03, 0x28};
static const uint8_t uuid_char_client_cfg[] = {0x02, 0x29};
static const uint8_t uuid_char_rx[]         = {0x01, 0xFF};
static const uint8_t uuid_char_tx[]         = {0x02, 0xFF};

// GATT 属性表定义
static esp_gatts_attr_db_t gatt_db[GATTS_NUM_HANDLES] = {
    [0] = { {ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)uuid_primary_service, ESP_GATT_PERM_READ, sizeof(uint16_t), sizeof(uint16_t), (uint8_t *)&(uint16_t){GATTS_SERVICE_UUID} } },
    [1] = { {ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)uuid_char_declare,    ESP_GATT_PERM_READ, sizeof(uint8_t),  sizeof(uint8_t),  (uint8_t *)&(uint8_t){ESP_GATT_CHAR_PROP_BIT_NOTIFY} } },
    [2] = { {ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)uuid_char_tx,         ESP_GATT_PERM_READ, 128, 0, NULL } },
    [3] = { {ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)uuid_char_declare,    ESP_GATT_PERM_READ, sizeof(uint8_t),  sizeof(uint8_t),  (uint8_t *)&(uint8_t){ESP_GATT_CHAR_PROP_BIT_WRITE} } },
    [4] = { {ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)uuid_char_rx,         ESP_GATT_PERM_WRITE, 128, 0, NULL } },
    [5] = { {ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)uuid_char_client_cfg, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, sizeof(uint16_t), sizeof(uint16_t), (uint8_t *)&(uint16_t){0x0000} } },
};

// BLE GAP 回调 - 广播数据配置完成后启动广播
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        ESP_LOGI(TAG, "广播数据配置完成，启动广播...");
        adv_data_ready = true;
        esp_err_t err = esp_ble_gap_start_advertising(&adv_params);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "BLE 广播已成功启动");
        } else {
            ESP_LOGE(TAG, "BLE 广播启动失败 err=0x%x", err);
        }
        break;
    default:
        break;
    }
}

// GATTS profile 回调
static void gatts_profile_event_handler(esp_gatts_cb_event_t event,
                                        esp_gatt_if_t interface,
                                        esp_ble_gatts_cb_param_t *param) {
    switch (event) {
    case ESP_GATTS_REG_EVT:
        ESP_LOGI(TAG, "BLE 注册完成，配置设备名称与广播数据...");
        esp_ble_gap_set_device_name(DEVICE_NAME);
        esp_ble_gap_config_adv_data(&adv_data);
        break;

    case ESP_GATTS_CREAT_ATTR_TAB_EVT:
        if (param->add_attr_tab.status != ESP_GATT_OK) {
            ESP_LOGE(TAG, "属性表创建失败，错误码=0x%x", param->add_attr_tab.status);
        } else if (param->add_attr_tab.num_handle == GATTS_NUM_HANDLES) {
            ESP_LOGI(TAG, "属性表创建成功，启动服务...");
            esp_ble_gatts_start_service(param->add_attr_tab.handles[0]);
            tx_handle = param->add_attr_tab.handles[2]; // TX Value
        } else {
            ESP_LOGE(TAG, "属性表句柄数量不一致");
        }
        break;

    case ESP_GATTS_CONNECT_EVT:
        conn_id = param->connect.conn_id;
        is_connected = true;
        ESP_LOGI(TAG, "BLE 设备已连接 conn_id=%d", conn_id);
        break;

    case ESP_GATTS_DISCONNECT_EVT:
        is_connected = false;
        conn_id = 0xFFFF;
        ESP_LOGI(TAG, "BLE 设备断开连接，重新广播...");
        if (adv_data_ready) {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;

    case ESP_GATTS_WRITE_EVT:
        if (!param->write.is_prep && param->write.len > 0) {
            char data[128] = {0};
            memcpy(data, param->write.value, param->write.len < sizeof(data) - 1 ? param->write.len : sizeof(data) - 1);
            ESP_LOGI(TAG, "接收到手机写入数据: %s", data);
        }
        break;

    default:
        break;
    }
}

// GATTS 主事件分发器
static void gatts_event_handler(esp_gatts_cb_event_t event,
                                esp_gatt_if_t interface,
                                esp_ble_gatts_cb_param_t *param) {
    if (event == ESP_GATTS_REG_EVT) {
        gatt_if = interface;
    }
    gatts_profile_event_handler(event, interface, param);
}

// 5 秒定时 notify task
static void notify_task(void *arg) {
    while (1) {
        if (is_connected && tx_handle != 0) {
            int val = light_sensor_get_cached_value();
            char buf[32];
            snprintf(buf, sizeof(buf), "light: %d", val);
            esp_ble_gatts_send_indicate(gatt_if, conn_id, tx_handle,
                                        strlen(buf), (uint8_t *)buf, false);
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// BLE 初始化启动入口
void ble_service_start(void) {
    ESP_LOGI(TAG, "初始化 BLE 服务...");

    esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    ESP_ERROR_CHECK(esp_ble_gap_register_callback(gap_event_handler));
    ESP_ERROR_CHECK(esp_ble_gatts_register_callback(gatts_event_handler));
    ESP_ERROR_CHECK(esp_ble_gatts_app_register(PROFILE_APP_ID));

    ESP_ERROR_CHECK(esp_ble_gatts_create_attr_tab(gatt_db, gatt_if, GATTS_NUM_HANDLES, PROFILE_APP_ID));

    xTaskCreate(notify_task, "ble_notify_task", 2048, NULL, 5, NULL);
}
