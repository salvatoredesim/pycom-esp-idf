// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string.h>
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "bt_trace.h"
#include "btc_manage.h"
#include "btc_gap_bt.h"

esp_err_t esp_bt_gap_set_scan_mode(esp_bt_scan_mode_t mode)
{
    btc_msg_t msg;
    btc_gap_bt_args_t arg;

    if (esp_bluedroid_get_status() != ESP_BLUEDROID_STATUS_ENABLED) {
        return ESP_ERR_INVALID_STATE;
    }
    
    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_GAP_BT;
    msg.act = BTC_GAP_BT_ACT_SET_SCAN_MODE;
    arg.set_scan_mode.mode = mode;

    return (btc_transfer_context(&msg, &arg, sizeof(btc_gap_bt_args_t), NULL) == BT_STATUS_SUCCESS ? ESP_OK : ESP_FAIL);
}

esp_err_t esp_bt_gap_set_device_name(const char *name)
{
    btc_msg_t msg;
    btc_gap_bt_args_t arg;

    if (esp_bluedroid_get_status() != ESP_BLUEDROID_STATUS_ENABLED) {
        return ESP_ERR_INVALID_STATE;
    }

    if (name != NULL) {
        size_t len = strlen(name);
        if (len > 0 || len <= ESP_BT_GAP_DEVICE_NAME_MAX) {
            strcpy(arg.set_dev_name.device_name, name);
        } else {
            return ESP_ERR_INVALID_ARG;
        }
    } else {
        return ESP_ERR_INVALID_ARG;
    }

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_GAP_BT;
    msg.act = BTC_GAP_BT_ACT_SET_DEV_NAME;


    return (btc_transfer_context(&msg, &arg, sizeof(btc_gap_bt_args_t), NULL) == BT_STATUS_SUCCESS ? ESP_OK : ESP_FAIL);
}
