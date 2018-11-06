/******************************************************************************
  Filename:       MT.h
  Revised:        $Date: 2014-01-29 13:20:40 -0800 (Wed, 29 Jan 2014) $
  Revision:       $Revision: 37032 $

  Description:    MonitorTest command and response definitions.


  Copyright 2007-2014 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License"). You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product. Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
******************************************************************************/
#ifndef MT_H
#define MT_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************************************************************************************
 * INCLUDES
 ***************************************************************************************************/

#include "MT_RPC.h"
#include "ZComDef.h"

/***************************************************************************************************
 * MACROS
 ***************************************************************************************************/

/***************************************************************************************************
 * CONSTANTS
 ***************************************************************************************************/
//Special definitions for ZTOOL (Zigbee 0.7 release)
#define ZTEST_DEFAULT_PARAM_LEN              0x10  //( 16 Bytes)
#define ZTEST_DEFAULT_ADDR_LEN               0x08  //(  8 Bytes)
#define ZTEST_DEFAULT_DATA_LEN               0x75  //(117 Bytes)
#define ZTEST_DEFAULT_AF_DATA_LEN            0x20  //( 32 Bytes) - AF
#define ZTEST_DEFAULT_SEC_LEN                0x0B
#define ZTEST_DEFAULT_SEC_PARAM_LEN          0x1C  //( 28 Bytes)



/***************************************************************************************************
 * NWK COMMANDS
 ***************************************************************************************************/

/* AREQ from host */
#define MT_NWK_INIT                          0x00

/* SREQ/SRSP */
#define MT_NLDE_DATA_REQ                     0x01
#define MT_NLME_NETWORK_FORMATION_REQ        0x02
#define MT_NLME_PERMIT_JOINING_REQ           0x03
#define MT_NLME_JOIN_REQ                     0x04
#define MT_NLME_LEAVE_REQ                    0x05
#define MT_NLME_RESET_REQ                    0x06
#define MT_NLME_GET_REQ                      0x07
#define MT_NLME_SET_REQ                      0x08
#define MT_NLME_NETWORK_DISCOVERY_REQ        0x09
#define MT_NLME_ROUTE_DISCOVERY_REQ          0x0A
#define MT_NLME_DIRECT_JOIN_REQ              0x0B
#define MT_NLME_ORPHAN_JOIN_REQ              0x0C
#define MT_NLME_START_ROUTER_REQ             0x0D

/* AREQ to host */
#define MT_NLDE_DATA_CONF                    0x80
#define MT_NLDE_DATA_IND                     0x81
#define MT_NLME_NETWORK_FORMATION_CONF       0x82
#define MT_NLME_JOIN_CONF                    0x83
#define MT_NLME_JOIN_IND                     0x84
#define MT_NLME_LEAVE_CONF                   0x85
#define MT_NLME_LEAVE_IND                    0x86
#define MT_NLME_POLL_CONF                    0x87
#define MT_NLME_SYNC_IND                     0x88
#define MT_NLME_NETWORK_DISCOVERY_CONF       0x89
#define MT_NLME_START_ROUTER_CONF            0x8A



/***************************************************************************************************
 * DEBUG COMMANDS
 ***************************************************************************************************/

/* SREQ/SRSP: */
#define MT_DEBUG_SET_THRESHOLD               0x00

#define MT_DEBUG_MAC_DATA_DUMP               0x10

/* AREQ */
#define MT_DEBUG_MSG                         0x80

/***************************************************************************************************
 * APP COMMANDS
 ***************************************************************************************************/

/* SREQ/SRSP: */
#define MT_APP_MSG                           0x00
#define MT_APP_USER_TEST                     0x01

/* SRSP */
#define MT_APP_RSP                           0x80
#define MT_APP_ZLL_TL_IND                    0x81

/***************************************************************************************************
* FILE SYSTEM COMMANDS
***************************************************************************************************/
#define MT_OTA_FILE_READ_REQ                  0x00
#define MT_OTA_NEXT_IMG_REQ                   0x01

#define MT_OTA_FILE_READ_RSP                  0x80
#define MT_OTA_NEXT_IMG_RSP                   0x81
#define MT_OTA_STATUS_IND                     0x82

/***************************************************************************************************
 * UBL COMMANDS
 ***************************************************************************************************/

/* SREQ/SRSP: */

//efine SB_TGT_BOOTLOAD                       0x10 // Optional command in Target Application.
#define MT_UBL_ERASE                          0x10

#define MT_UBL_WRITE                          0x11
#define MT_UBL_READ                           0x12
#define MT_UBL_ENABLE                         0x13
#define MT_UBL_HANDSHAKE                      0x14
#define MT_UBL_FORCE                          0x15

/***************************************************************************************************
 * ZNP COMMANDS
 ***************************************************************************************************/

#define MT_ZNP_BASIC_CFG                      0x00
#define MT_ZNP_ZCL_CFG                        0x10
#define MT_ZNP_SE_CFG                         0x20


/***************************************************************************************************
 * ZNP RESPONSES
 ***************************************************************************************************/

#define MT_ZNP_BASIC_RSP                      0x80
#define MT_ZNP_ZCL_RSP                        0x90
#define MT_ZNP_SE_RSP                         0xA0


/* Task Secure Random Number Generator ID */
#ifdef MT_SRNG
#define MT_SRNG_EVENT                   0x1000
#endif

/* Message Command IDs */
#define CMD_SERIAL_MSG                  0x01
#define CMD_DEBUG_MSG                   0x02
#define CMD_TX_MSG                      0x03
#define CB_FUNC                         0x04
#define CMD_SEQUENCE_MSG                0x05
#define CMD_DEBUG_STR                   0x06
#define AF_INCOMING_MSG_FOR_MT          0x0F

/* Error Response IDs */
#define UNRECOGNIZED_COMMAND            0x00
#define UNSUPPORTED_COMMAND             0x01
#define RECEIVE_BUFFER_FULL             0x02

/* Serial Message Command IDs */
#define SPI_CMD_DEBUG_MSG               0x4003
#define SPI_CMD_TRACE_SUB               0x0004
#define SPI_CMD_TRACE_MSG               0x4004

/* User CMD IDs */
#define SPI_CMD_USER0                   0x000A
#define SPI_CMD_USER1                   0x000B
#define SPI_CMD_USER2                   0x000C
#define SPI_CMD_USER3                   0x000D
#define SPI_CMD_USER4                   0x000E
#define SPI_CMD_USER5                   0x000F


#define SPI_SYS_STRING_MSG              0x0015
#define SPI_CMD_SYS_NETWORK_START       0x0020
#define SPI_CMD_ZIGNET_DATA             0x0022

/* system command response */
#define SPI_CB_SYS_CALLBACK_SUB_RSP     0x1006
#define SPI_CB_SYS_PING_RSP             0x1007
#define SPI_CB_SYS_GET_DEVICE_INFO_RSP  0x1014
#define SPI_CB_SYS_KEY_EVENT_RSP        0x1016
#define SPI_CB_SYS_HEARTBEAT_RSP        0x1017
#define SPI_CB_SYS_LED_CONTROL_RSP      0x1019

/* Message Sequence definitions */
#define SPI_CMD_SEQ_START               0x0600
#define SPI_CMD_SEQ_WAIT                0x0601
#define SPI_CMD_SEQ_END                 0x0602
#define SPI_CMD_SEQ_RESET               0x0603
#define DEFAULT_WAIT_INTERVAL           5000      //5 seconds

/* Serial Message Command Routing Bits */
#define SPI_RESPONSE_BIT                0x1000
#define SPI_SUBSCRIPTION_BIT            0x2000
#define SPI_DEBUGTRACE_BIT              0x4000

#define SPI_0DATA_MSG_LEN                5
#define SPI_RESP_MSG_LEN_DEFAULT         6

#define LEN_MAC_BEACON_MSDU             15
#define LEN_MAC_COORDEXTND_ADDR          8
#define LEN_MAC_ATTR_BYTE                1
#define LEN_MAC_ATTR_INT                 2

#define SOP_FIELD                        0
#define CMD_FIELD_HI                     1
#define CMD_FIELD_LO                     2
#define DATALEN_FIELD                    3
#define DATA_BEGIN                       4

/* MT PACKET (For Test Tool): FIELD IDENTIFIERS */
#define MT_MAC_CB_ID                0
#define MT_OFFSET                   1
#define MT_SOP_FIELD                MT_OFFSET + SOP_FIELD
#define MT_CMD_FIELD_HI             MT_OFFSET + CMD_FIELD_HI
#define MT_CMD_FIELD_LO             MT_OFFSET + CMD_FIELD_LO
#define MT_DATALEN_FIELD            MT_OFFSET + DATALEN_FIELD
#define MT_DATA_BEGIN               MT_OFFSET + DATA_BEGIN

#define MT_INFO_HEADER_LEN         1
#define MT_RAM_READ_RESP_LEN       0x02
#define MT_RAM_WRITE_RESP_LEN      0x01

/* Defines for the fields in the AF structures */
#define AF_INTERFACE_BITS          0x07
#define AF_INTERFACE_OFFSET        0x05
#define AF_APP_DEV_VER_MASK        0x0F
#define AF_APP_FLAGS_MASK          0x0F
#define AF_TRANSTYPE_MASK          0x0F
#define AF_TRANSDATATYPE_MASK      0x0F

#define TGEN_START									0x000a
#define TGEN_STOP										0x000b
#define TGEN_COUNT									0x000c
#define DEBUG_GET					          0x000d
#define HW_TEST                     0x000e
#define HW_DISPLAY_RESULT						0x000f
#define HW_SEND_STATUS							0x0010

#if defined ( APP_TP ) || defined ( APP_TP2 )
#if defined ( APP_TP )
#define TP_SEND_NODATA              0x0011
#else
#define TP_SEND_BCAST_RSP           0x0011
#endif
#define TP_SEND_BUFFERTEST          0x0012
#if defined (APP_TP)
#define TP_SEND_UINT8               0x0013
#define TP_SEND_INT8                0x0014
#define TP_SEND_UINT16              0x0015
#define TP_SEND_INT16               0x0016
#define TP_SEND_SEMIPREC            0x0017
#endif
#define TP_SEND_FREEFORM            0x0018
#if defined ( APP_TP )
#define TP_SEND_ABS_TIME            0x0019
#define TP_SEND_REL_TIME            0x001A
#define TP_SEND_CHAR_STRING         0x001B
#define TP_SEND_OCTET_STRING        0x001C
#endif
#define TP_SET_DSTADDRESS           0x001D
#if defined ( APP_TP2 )
#define TP_SEND_BUFFER_GROUP        0x001E
#endif
#define TP_SEND_BUFFER              0x001F
#if defined ( APP_TP )
#define TP_SEND_CON_INT8						0x0020
#define TP_SEND_CON_INT16						0x0021
#define TP_SEND_CON_TIME						0x0022

#define TP_SEND_MULT_KVP_8BIT       0x0023
#define TP_SEND_MULT_KVP_16BIT      0x0024
#define TP_SEND_MULT_KVP_TIME       0x0025
#define TP_SEND_MULT_KVP_STRING     0x0026
#endif

#define TP_SEND_COUNTED_PKTS        0x0027
#define TP_SEND_RESET_COUNTER       0x0028
#define TP_SEND_GET_COUNTER         0x0029

#if defined ( APP_TP )
#define TP_SEND_MULTI_KVP_STR_TIME  0x0030
#endif

#define TP_SET_PERMIT_JOIN          0x0040

#define TP_ADD_GROUP                0x0041
#define TP_REMOVE_GROUP             0x0042

#define TP_SEND_UPDATE_KEY          0x0043
#define TP_SEND_SWITCH_KEY          0x0044

#if defined ( APP_TP2 )
#define TP_SEND_BUFFERTEST_GROUP    0x0045
#define TP_SEND_ROUTE_DISC_REQ      0x0046
#define TP_SEND_ROUTE_DISCOVERY     0x0047
#define TP_SEND_NEW_ADDR            0x0048
#define TP_SEND_NWK_UPDATE          0x0049
#define TP_AK_SETUP_PARTNER         0x0050
#define TP_AK_REQ_KEY               0x0051
#define TP_AK_PARTNER_NWKADDR       0x0052
#define TP_AK_PARTNER_EXTADDR7654   0x0053
#define TP_AK_PARTNER_EXTADDR3210   0x0054
#define TP_AK_PARTNER_SET           0x0055
#define TP_AK_TYPE_SET              0x0056
#define TP_FRAG_SKIP_BLOCK          0x0057
#define TP_APS_REMOVE               0x0058
#define TP_NWK_ADDR_CONFLICT        0x0059
#if defined ( APP_TP2_TEST_MODE )
#define TP_GU_SET_TX_APS_SEC        0x005A
#define TP_GU_SET_RX_APS_SEC        0x005B
#endif
#define TP_SET_LEAVE_REQ_ALLOWED    0x005C
#define TP_SEND_REJOIN_REQ_SECURE   0x005D
#endif

#endif

/* Capabilities - PING Response */
/* Capabilities - PING Response */
#define MT_CAP_SYS    0x0001
#define MT_CAP_MAC    0x0002
#define MT_CAP_NWK    0x0004
#define MT_CAP_AF     0x0008
#define MT_CAP_ZDO    0x0010
#define MT_CAP_SAPI   0x0020
#define MT_CAP_UTIL   0x0040
#define MT_CAP_DEBUG  0x0080
#define MT_CAP_APP    0x0100
#define MT_CAP_ZOAD   0x1000

/* ZNP NV items, 1-4 2-bytes each, 5-6 16-bytes each */
#define ZNP_NV_APP_ITEM_1       0x0F01
#define ZNP_NV_APP_ITEM_2       0x0F02
#define ZNP_NV_APP_ITEM_3       0x0F03
#define ZNP_NV_APP_ITEM_4       0x0F04
#define ZNP_NV_APP_ITEM_5       0x0F05
#define ZNP_NV_APP_ITEM_6       0x0F06

#define ZNP_NV_RF_TEST_PARMS    0x0F07


#ifdef __cplusplus
}
#endif

#endif

