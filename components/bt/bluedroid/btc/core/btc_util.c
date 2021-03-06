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

/************************************************************************************
 *
 *  Filename:      btc_util.c
 *
 *  Description:   Miscellaneous helper functions
 *
 *
 ***********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "btc_util.h"
#include "bta_av_api.h"
#include "bt_defs.h"

/************************************************************************************
**  Constants & Macros
************************************************************************************/
#define ISDIGIT(a)  ((a>='0') && (a<='9'))
#define ISXDIGIT(a) (((a>='0') && (a<='9'))||((a>='A') && (a<='F'))||((a>='a') && (a<='f')))

/************************************************************************************
**  Local type definitions
************************************************************************************/

/************************************************************************************
**  Static variables
************************************************************************************/

/************************************************************************************
**  Static functions
************************************************************************************/

/************************************************************************************
**  Externs
************************************************************************************/

/************************************************************************************
**  Functions
************************************************************************************/

/*****************************************************************************
**   Logging helper functions
*****************************************************************************/
const char *dump_rc_event(UINT8 event)
{
    switch (event) {
        CASE_RETURN_STR(BTA_AV_RC_OPEN_EVT)
        CASE_RETURN_STR(BTA_AV_RC_CLOSE_EVT)
        CASE_RETURN_STR(BTA_AV_REMOTE_CMD_EVT)
        CASE_RETURN_STR(BTA_AV_REMOTE_RSP_EVT)
        CASE_RETURN_STR(BTA_AV_VENDOR_CMD_EVT)
        CASE_RETURN_STR(BTA_AV_VENDOR_RSP_EVT)
        CASE_RETURN_STR(BTA_AV_META_MSG_EVT)
        CASE_RETURN_STR(BTA_AV_RC_FEAT_EVT)
    default:
        return "UNKNOWN_EVENT";
    }
}

const char *dump_rc_notification_event_id(UINT8 event_id)
{
    switch (event_id) {
        CASE_RETURN_STR(AVRC_EVT_PLAY_STATUS_CHANGE)
        CASE_RETURN_STR(AVRC_EVT_TRACK_CHANGE)
        CASE_RETURN_STR(AVRC_EVT_TRACK_REACHED_END)
        CASE_RETURN_STR(AVRC_EVT_TRACK_REACHED_START)
        CASE_RETURN_STR(AVRC_EVT_PLAY_POS_CHANGED)
        CASE_RETURN_STR(AVRC_EVT_BATTERY_STATUS_CHANGE)
        CASE_RETURN_STR(AVRC_EVT_SYSTEM_STATUS_CHANGE)
        CASE_RETURN_STR(AVRC_EVT_APP_SETTING_CHANGE)
        CASE_RETURN_STR(AVRC_EVT_VOLUME_CHANGE)

    default:
        return "Unhandled Event ID";
    }
}

const char  *dump_rc_pdu(UINT8 pdu)
{
    switch (pdu) {
        CASE_RETURN_STR(AVRC_PDU_LIST_PLAYER_APP_ATTR)
        CASE_RETURN_STR(AVRC_PDU_LIST_PLAYER_APP_VALUES)
        CASE_RETURN_STR(AVRC_PDU_GET_CUR_PLAYER_APP_VALUE)
        CASE_RETURN_STR(AVRC_PDU_SET_PLAYER_APP_VALUE)
        CASE_RETURN_STR(AVRC_PDU_GET_PLAYER_APP_ATTR_TEXT)
        CASE_RETURN_STR(AVRC_PDU_GET_PLAYER_APP_VALUE_TEXT)
        CASE_RETURN_STR(AVRC_PDU_INFORM_DISPLAY_CHARSET)
        CASE_RETURN_STR(AVRC_PDU_INFORM_BATTERY_STAT_OF_CT)
        CASE_RETURN_STR(AVRC_PDU_GET_ELEMENT_ATTR)
        CASE_RETURN_STR(AVRC_PDU_GET_PLAY_STATUS)
        CASE_RETURN_STR(AVRC_PDU_REGISTER_NOTIFICATION)
        CASE_RETURN_STR(AVRC_PDU_REQUEST_CONTINUATION_RSP)
        CASE_RETURN_STR(AVRC_PDU_ABORT_CONTINUATION_RSP)
        CASE_RETURN_STR(AVRC_PDU_SET_ABSOLUTE_VOLUME)
    default:
        return "Unknown PDU";
    }
}

UINT32 devclass2uint(DEV_CLASS dev_class)
{
    UINT32 cod = 0;

    if (dev_class != NULL) {
        /* if COD is 0, irrespective of the device type set it to Unclassified device */
        cod = (dev_class[2]) | (dev_class[1] << 8) | (dev_class[0] << 16);
    }
    return cod;
}
void uint2devclass(UINT32 cod, DEV_CLASS dev_class)
{
    dev_class[2] = (UINT8)cod;
    dev_class[1] = (UINT8)(cod >> 8);
    dev_class[0] = (UINT8)(cod >> 16);
}

static const UINT8  sdp_base_uuid[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
                                       0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB
                                      };

void uuid16_to_uuid128(uint16_t uuid16, bt_uuid_t *uuid128)
{
    uint16_t uuid16_bo;
    memset(uuid128, 0, sizeof(bt_uuid_t));

    memcpy(uuid128->uu, sdp_base_uuid, MAX_UUID_SIZE);
    uuid16_bo = ntohs(uuid16);
    memcpy(uuid128->uu + 2, &uuid16_bo, sizeof(uint16_t));
}

void string_to_uuid(char *str, bt_uuid_t *p_uuid)
{
    uint32_t uuid0, uuid4;
    uint16_t uuid1, uuid2, uuid3, uuid5;

    sscanf(str, "%08x-%04hx-%04hx-%04hx-%08x%04hx",
           &uuid0, &uuid1, &uuid2, &uuid3, &uuid4, &uuid5);

    uuid0 = htonl(uuid0);
    uuid1 = htons(uuid1);
    uuid2 = htons(uuid2);
    uuid3 = htons(uuid3);
    uuid4 = htonl(uuid4);
    uuid5 = htons(uuid5);

    memcpy(&(p_uuid->uu[0]), &uuid0, 4);
    memcpy(&(p_uuid->uu[4]), &uuid1, 2);
    memcpy(&(p_uuid->uu[6]), &uuid2, 2);
    memcpy(&(p_uuid->uu[8]), &uuid3, 2);
    memcpy(&(p_uuid->uu[10]), &uuid4, 4);
    memcpy(&(p_uuid->uu[14]), &uuid5, 2);

    return;

}

void uuid_to_string_legacy(bt_uuid_t *p_uuid, char *str)
{
    uint32_t uuid0, uuid4;
    uint16_t uuid1, uuid2, uuid3, uuid5;

    memcpy(&uuid0, &(p_uuid->uu[0]), 4);
    memcpy(&uuid1, &(p_uuid->uu[4]), 2);
    memcpy(&uuid2, &(p_uuid->uu[6]), 2);
    memcpy(&uuid3, &(p_uuid->uu[8]), 2);
    memcpy(&uuid4, &(p_uuid->uu[10]), 4);
    memcpy(&uuid5, &(p_uuid->uu[14]), 2);

    sprintf((char *)str, "%.8x-%.4x-%.4x-%.4x-%.8x%.4x",
            ntohl(uuid0), ntohs(uuid1),
            ntohs(uuid2), ntohs(uuid3),
            ntohl(uuid4), ntohs(uuid5));
    return;
}
