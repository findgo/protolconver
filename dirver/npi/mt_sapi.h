#ifndef __MT_SAPI_H__
#define __MT_SAPI_H__

#include "app_cfg.h"
#include "mt_npi.h"

/***************************************************************************************************
 * SAPI COMMANDS
 ***************************************************************************************************/

// SAPI MT Command Identifiers
/* AREQ from Host */
#define MT_SAPI_SYS_RESET                   0x09
#define MT_SAPI_START_NWK                   0x5a

/* SREQ/SRSP */
#define MT_SAPI_START_REQ                   0x00
#define MT_SAPI_BIND_DEVICE_REQ             0x01
#define MT_SAPI_ALLOW_BIND_REQ              0x02
#define MT_SAPI_SEND_DATA_REQ               0x03
#define MT_SAPI_READ_CFG_REQ                0x04
#define MT_SAPI_WRITE_CFG_REQ               0x05
#define MT_SAPI_GET_DEV_INFO_REQ            0x06
#define MT_SAPI_FIND_DEV_REQ                0x07
#define MT_SAPI_PMT_JOIN_REQ                0x08
#define MT_SAPI_APP_REGISTER_REQ            0x0a

#define MT_SAPI_RESET_FACTORY               0x70
#define MT_SAPI_GET_DEV_ALL_INFO_REQ        0x71
#define MT_SAPI_READ_LOGICAL_TYPE           0x72
#define MT_SAPI_WRITE_LOGICAL_TYPE          0x73
#define MT_SAPI_AF_DATA_REQUEST             0x7a
/* AREQ to host */
#define MT_SAPI_START_CNF                   0x80
#define MT_SAPI_BIND_CNF                    0x81
#define MT_SAPI_ALLOW_BIND_CNF              0x82
#define MT_SAPI_SEND_DATA_CNF               0x83
#define MT_SAPI_READ_CFG_RSP                0x84
#define MT_SAPI_FIND_DEV_CNF                0x85
#define MT_SAPI_DEV_INFO_RSP                0x86
#define MT_SAPI_RCV_DATA_IND                0x87

#define MT_SAPI_RESET_IND                   0xf0
#define MT_SAPI_AF_INCOMING_MSG             0xfa

 // for bdb
enum
{
    BDB_COMMISSIONING_INITIALIZATION,
    BDB_COMMISSIONING_NWK_STEERING,
    BDB_COMMISSIONING_FORMATION,
    BDB_COMMISSIONING_FINDING_BINDING,
    BDB_COMMISSIONING_TOUCHLINK,
    BDB_COMMISSIONING_PARENT_LOST,
};

enum
{
    BDB_COMMISSIONING_SUCCESS,       
    BDB_COMMISSIONING_IN_PROGRESS,   
    BDB_COMMISSIONING_NO_NETWORK,          
    BDB_COMMISSIONING_TL_TARGET_FAILURE,
    BDB_COMMISSIONING_TL_NOT_AA_CAPABLE,
    BDB_COMMISSIONING_TL_NO_SCAN_RESPONSE,
    BDB_COMMISSIONING_TL_NOT_PERMITTED,
    BDB_COMMISSIONING_TCLK_EX_FAILURE,              //Many attempts made and failed, or parent lost during the TCLK exchange
    BDB_COMMISSIONING_FORMATION_FAILURE,
    BDB_COMMISSIONING_FB_TARGET_IN_PROGRESS,        //No callback from bdb when the identify time runs out, the application can figure out from Identify time callback
    BDB_COMMISSIONING_FB_INITITATOR_IN_PROGRESS,
    BDB_COMMISSIONING_FB_NO_IDENTIFY_QUERY_RESPONSE,
    BDB_COMMISSIONING_FB_BINDING_TABLE_FULL,
    BDB_COMMISSIONING_NETWORK_RESTORED,               
    BDB_COMMISSIONING_FAILURE,              //Generic failure status for no commissioning mode supported, not allowed, invalid state to perform commissioning
};

typedef struct {
    uint8_t devType;
    uint8_t state;
    uint16_t panid;
    
    //uint8_t nwkkey[Z_EXTADDR_LEN];
    
    uint16_t nwkaddr;
    uint16_t coor_nwkaddr;
    uint8_t macaddr[Z_EXTADDR_LEN];
    uint8_t coor_macaddr[Z_EXTADDR_LEN];
    
    uint8_t group;
    uint8_t channel;
    uint8_t txpower;
    uint8_t valid;  // mark info valid

    uint8_t err;
}nwk_info_t;

// 本地宏
#define MTSAPI_SendAsynchData(cmdID,pData,len)     NPISendAsynchData(MT_RPC_SYS_SAPI, cmdID, pData, len)
#define MTSAPI_SendSynchData(cmdID,pData,len)      NPISendSynchData(MT_RPC_SYS_SAPI, cmdID, pData, len)

// aync command 
#define mtsapi_SystemReset()    MTSAPI_SendAsynchData(MT_SAPI_SYS_RESET,NULL,0)
int mtsapi_StartNwk(uint8_t mode);

// sync command
//#define mtsapi_StartRequest()   MTSAPI_SendSynchData(MT_SAPI_START_REQ, NULL, 0)
//int mtsapi_WriteConfiguration(uint8_t configId, uint8_t len, uint8_t *valTab);
//int mtsapi_ReadConfiguration(uint8_t configId);
//int mtsapi_GeDeviceInfo(uint8_t Param);
//int mtsapi_PermitJoingRequest(uint16_t dstaddr, uint8_t timeout);


// by mo
#define mtsapi_ResetFactory()    MTSAPI_SendSynchData(MT_SAPI_RESET_FACTORY, NULL, 0)  
#define mtsapi_GeDeviceAllInfo() MTSAPI_SendSynchData(MT_SAPI_GET_DEV_ALL_INFO_REQ, NULL, 0)
#define mtsapi_ReadLogicalType() MTSAPI_SendSynchData(MT_SAPI_READ_LOGICAL_TYPE, NULL,0)
int mtsapi_writeLogicalType(uint8_t logicaltype);
#define mtsapi_DataRequest(pData,len)    MTSAPI_SendSynchData(MT_SAPI_AF_DATA_REQUEST, pData, len)

void ZbInit(void);
uint8_t ZbisOnNet(void);


int mtsapi_SyncHandle(uint8_t commandID, uint8_t *data, uint8_t len);
int mtsapi_AsncHandle(uint8_t commandID, uint8_t *data, uint8_t len);

#endif


