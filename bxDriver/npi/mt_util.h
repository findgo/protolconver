#ifndef __MT_UTIL_H__
#define __MT_UTIL_H__

#include "mt_npi.h"


/***************************************************************************************************
 * UTIL COMMANDS
 ***************************************************************************************************/

/* SREQ/SRSP: */
#define MT_UTIL_GET_DEVICE_INFO              0x00
#define MT_UTIL_GET_NV_INFO                  0x01
#define MT_UTIL_SET_PANID                    0x02
#define MT_UTIL_SET_CHANNELS                 0x03
#define MT_UTIL_SET_SECLEVEL                 0x04
#define MT_UTIL_SET_PRECFGKEY                0x05
#define MT_UTIL_CALLBACK_SUB_CMD             0x06
#define MT_UTIL_KEY_EVENT                    0x07
#define MT_UTIL_TIME_ALIVE                   0x09
#define MT_UTIL_LED_CONTROL                  0x0A

#define MT_UTIL_TEST_LOOPBACK                0x10
#define MT_UTIL_DATA_REQ                     0x11

#define MT_UTIL_GPIO_SET_DIRECTION           0x14
#define MT_UTIL_GPIO_READ                    0x15
#define MT_UTIL_GPIO_WRITE                   0x16

#define MT_UTIL_SRC_MATCH_ENABLE             0x20
#define MT_UTIL_SRC_MATCH_ADD_ENTRY          0x21
#define MT_UTIL_SRC_MATCH_DEL_ENTRY          0x22
#define MT_UTIL_SRC_MATCH_CHECK_SRC_ADDR     0x23
#define MT_UTIL_SRC_MATCH_ACK_ALL_PENDING    0x24
#define MT_UTIL_SRC_MATCH_CHECK_ALL_PENDING  0x25

#define MT_UTIL_ADDRMGR_EXT_ADDR_LOOKUP      0x40
#define MT_UTIL_ADDRMGR_NWK_ADDR_LOOKUP      0x41
#define MT_UTIL_APSME_LINK_KEY_DATA_GET      0x44
#define MT_UTIL_APSME_LINK_KEY_NV_ID_GET     0x45
#define MT_UTIL_ASSOC_COUNT                  0x48
#define MT_UTIL_ASSOC_FIND_DEVICE            0x49
#define MT_UTIL_ASSOC_GET_WITH_ADDRESS       0x4A
#define MT_UTIL_APSME_REQUEST_KEY_CMD        0x4B
#ifdef MT_SRNG
#define MT_UTIL_SRNG_GENERATE                0x4C
#endif
#define MT_UTIL_BIND_ADD_ENTRY               0x4D

#define MT_UTIL_ZCL_KEY_EST_INIT_EST         0x80
#define MT_UTIL_ZCL_KEY_EST_SIGN             0x81

/* AREQ from/to host */
#define MT_UTIL_SYNC_REQ                     0xE0
#define MT_UTIL_ZCL_KEY_ESTABLISH_IND        0xE1


// 本地宏
#define MTUTIL_SendAsynchData(cmdID,dat,len)     NPISendAsynchData(MT_RPC_SYS_UTIL, cmdID, dat, len)
#define MTUTIL_SendSynchData(cmdID,dat,len)      NPISendSynchData(MT_RPC_SYS_UTIL, cmdID, dat, len)

// sync command 
#define mtutil_GetDeviceInfo()  MTUTIL_SendSynchData(MT_UTIL_GET_DEVICE_INFO,NULL,0) 
#define mtutil_GetNvInfo()      MTUTIL_SendSynchData(MT_UTIL_GET_NV_INFO,NULL,0) 


int mtutil_SyncHandle(uint8_t commandID, uint8_t *data, uint8_t len);
int mtutil_AsncHandle(uint8_t commandID, uint8_t *data, uint8_t len);

#endif

