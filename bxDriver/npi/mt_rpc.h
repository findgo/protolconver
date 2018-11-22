
#ifndef MT_RPC_H
#define MT_RPC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common_type.h"

/***************************************************************************************************
 * CONSTANTS
 ***************************************************************************************************/

/* 1st byte is the length of the data field, 2nd/3rd bytes are command field. */
#define MT_RPC_FRAME_HDR_SZ   3  // length + cmd0 + cmd1

/* Maximum length of data in the general frame format. The upper limit is 255 because of the
 * 1-byte length protocol. But the operation limit is lower for code size and ram savings so that
 * the uart driver can use 256 byte rx/tx queues and so
 * (MT_RPC_DATA_MAX + MT_RPC_FRAME_HDR_SZ + MT_UART_FRAME_OVHD) < 256
 */
#define MT_RPC_DATA_MAX       250

/* The 3 MSB's of the 1st command field byte are for command type. */
#define MT_RPC_CMD_TYPE_MASK  0xE0

/* The 5 LSB's of the 1st command field byte are for the subsystem. */
#define MT_RPC_SUBSYSTEM_MASK 0x1F

/* position of fields in the general format frame */
#define MT_RPC_POS_LEN        0
#define MT_RPC_POS_CMD0       1
#define MT_RPC_POS_CMD1       2
#define MT_RPC_POS_DAT0       3

/* Error codes */
#define MT_RPC_SUCCESS        0     /* success */
#define MT_RPC_ERR_SUBSYSTEM  1     /* invalid subsystem */
#define MT_RPC_ERR_COMMAND_ID 2     /* invalid command ID */
#define MT_RPC_ERR_PARAMETER  3     /* invalid parameter */
#define MT_RPC_ERR_LENGTH     4     /* invalid length */

///////////////////////////////////////////////////////////////////////////////////////////////////
// UART Specific
///////////////////////////////////////////////////////////////////////////////////////////////////
#define MT_RPC_UART_FRAME_OVHD 2  // head and xoc

// Start of frame character value
#define MT_RPC_UART_SOF        0xFE

/***************************************************************************************************
 * TYPEDEF
 ***************************************************************************************************/
typedef enum {
  MT_RPC_CMD_POLL = 0x00,
  MT_RPC_CMD_SREQ = 0x20,
  MT_RPC_CMD_AREQ = 0x40,
  MT_RPC_CMD_SRSP = 0x60,
} mtRpcCmdType_t;
  
typedef enum {
  MT_RPC_SYS_RES0,   /* Reserved. */
  MT_RPC_SYS_SYS,
  MT_RPC_SYS_MAC,
  MT_RPC_SYS_NWK,
  MT_RPC_SYS_AF,
  MT_RPC_SYS_ZDO,
  MT_RPC_SYS_SAPI,   /* Simple API. */
  MT_RPC_SYS_UTIL,
  MT_RPC_SYS_DBG,
  MT_RPC_SYS_APP,    //0x09
  /***************/
  MT_RPC_SYS_OTA,   
  MT_RPC_SYS_ZNP,   
  MT_RPC_SYS_BOOT,   // 0x0d
  MT_RPC_SYS_UBL = 13,  // 13 to be compatible with existing RemoTI.
  MT_RPC_SYS_RES14,
  MT_RPC_SYS_APPCFG, // 0x0f APPconfig
  MT_RPC_SYS_RES16,
  MT_RPC_SYS_PROTOBUF,
  MT_RPC_SYS_MAX,     /* Maximum value, must be last */
  MT_RPC_SYS_GP = 0x15  // GreenPower
  /* 18-32 available, not yet assigned. */
} mtRpcSysType_t;

/***************************************************************************************************
***************************************************************************************************/

#ifdef __cplusplus
};
#endif

#endif /* MT_RPC_H */

