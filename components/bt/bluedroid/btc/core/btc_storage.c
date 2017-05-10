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

#include "btc_storage.h"
#include "btc_util.h"
#include "osi.h"
#include "bt_trace.h"
#include "esp_system.h"
#include "bta_api.h"
#include "bdaddr.h"
#include "btc_config.h"

/*******************************************************************************
**
** Function         btc_storage_add_bonded_device
**
** Description      BTIF storage API - Adds the newly bonded device to NVRAM
**                  along with the link-key, Key type and Pin key length
**
** Returns          BT_STATUS_SUCCESS if the store was successful,
**                  BT_STATUS_FAIL otherwise
**
*******************************************************************************/

bt_status_t btc_storage_add_bonded_device(bt_bdaddr_t *remote_bd_addr,
        LINK_KEY link_key,
        uint8_t key_type,
        uint8_t pin_length)
{
    bdstr_t bdstr;

    bdaddr_to_string(remote_bd_addr, bdstr, sizeof(bdstr));
    LOG_INFO("add to storage: Remote device:%s\n", bdstr);

    int ret = btc_config_set_int(bdstr, "LinkKeyType", (int)key_type);
    ret &= btc_config_set_int(bdstr, "PinLength", (int)pin_length);
    ret &= btc_config_set_bin(bdstr, "LinkKey", link_key, sizeof(LINK_KEY));
    /* write bonded info immediately */
    btc_config_flush();
    LOG_INFO("Storage add rslt %d\n", ret);
    return ret ? BT_STATUS_SUCCESS : BT_STATUS_FAIL;
}

/*******************************************************************************
**
** Function         btc_in_fetch_bonded_devices
**
** Description      Internal helper function to fetch the bonded devices
**                  from NVRAM
**
** Returns          BT_STATUS_SUCCESS if successful, BT_STATUS_FAIL otherwise
**
*******************************************************************************/
static bt_status_t btc_in_fetch_bonded_devices(int add)
{
    BOOLEAN bt_linkkey_file_found = FALSE;

    for (const btc_config_section_iter_t *iter = btc_config_section_begin(); iter != btc_config_section_end(); iter = btc_config_section_next(iter)) {
        const char *name = btc_config_section_name(iter);
        if (!string_is_bdaddr(name)) {
            continue;
        }

        LOG_INFO("Remote device:%s\n", name);
        LINK_KEY link_key;
        size_t size = sizeof(link_key);
        if (btc_config_get_bin(name, "LinkKey", link_key, &size)) {
            int linkkey_type;
            if (btc_config_get_int(name, "LinkKeyType", &linkkey_type)) {
                //int pin_len;
                //btc_config_get_int(name, "PinLength", &pin_len))
                bt_bdaddr_t bd_addr;
                string_to_bdaddr(name, &bd_addr);
                if (add) {
                    DEV_CLASS dev_class = {0, 0, 0};
                    int cod;
                    int pin_length = 0;
                    if (btc_config_get_int(name, "DevClass", &cod)) {
                        uint2devclass((UINT32)cod, dev_class);
                    }
                    btc_config_get_int(name, "PinLength", &pin_length);
                    BTA_DmAddDevice(bd_addr.address, dev_class, link_key, 0, 0,
                                    (UINT8)linkkey_type, 0, pin_length);
                }
                bt_linkkey_file_found = TRUE;
            } else {
                LOG_ERROR("bounded device:%s, LinkKeyType or PinLength is invalid\n", name);
            }
        }
        if (!bt_linkkey_file_found) {
            LOG_INFO("Remote device:%s, no link key\n", name);
        }
    }
    return BT_STATUS_SUCCESS;
}


/*******************************************************************************
**
** Function         btc_storage_load_bonded_devices
**
** Description      BTC storage API - Loads all the bonded devices from NVRAM
**                  and adds to the BTA.
**                  Additionally, this API also invokes the adaper_properties_cb
**                  and remote_device_properties_cb for each of the bonded devices.
**
** Returns          BT_STATUS_SUCCESS if successful, BT_STATUS_FAIL otherwise
**
*******************************************************************************/
bt_status_t btc_storage_load_bonded_devices(void)
{
    bt_status_t status;
    status = btc_in_fetch_bonded_devices(1);
    LOG_INFO("Storage load rslt %d\n", status);
    return status;
}

/*******************************************************************************
**
** Function         btc_storage_remove_bonded_device
**
** Description      BTC storage API - Deletes the bonded device from NVRAM
**
** Returns          BT_STATUS_SUCCESS if the deletion was successful,
**                  BT_STATUS_FAIL otherwise
**
*******************************************************************************/
bt_status_t btc_storage_remove_bonded_device(bt_bdaddr_t *remote_bd_addr)
{
    bdstr_t bdstr;
    bdaddr_to_string(remote_bd_addr, bdstr, sizeof(bdstr));
    LOG_INFO("Add to storage: Remote device:%s\n", bdstr);

    int ret = 1;
    if (btc_config_exist(bdstr, "LinkKeyType")) {
        ret &= btc_config_remove(bdstr, "LinkKeyType");
    }
    if (btc_config_exist(bdstr, "PinLength")) {
        ret &= btc_config_remove(bdstr, "PinLength");
    }
    if (btc_config_exist(bdstr, "LinkKey")) {
        ret &= btc_config_remove(bdstr, "LinkKey");
    }
    /* write bonded info immediately */
    btc_config_flush();
    return ret ? BT_STATUS_SUCCESS : BT_STATUS_FAIL;
}
