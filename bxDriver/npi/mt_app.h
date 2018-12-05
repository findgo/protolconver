#ifndef __MT_APP_H__
#define __MT_APP_H__

#include "app_cfg.h"
#include "mt_npi.h"

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

// aync command 
#define mtsapi_SystemReset()    NPISendAsynchData(MT_SAPI_SYS_RESET, NULL, 0)
int mtsapi_StartNwk(uint8_t mode);

// sync command
//#define mtsapi_StartRequest()   NPISendSynchData(MT_SAPI_START_REQ, NULL, 0)
//int mtsapi_WriteConfiguration(uint8_t configId, uint8_t len, uint8_t *valTab);
//int mtsapi_ReadConfiguration(uint8_t configId);
//int mtsapi_GeDeviceInfo(uint8_t Param);
//int mtsapi_PermitJoingRequest(uint16_t dstaddr, uint8_t timeout);


// by mo
#define mtsapi_ResetFactory()       NPISendSynchData(MT_SAPI_RESET_FACTORY, NULL, 0)  
#define mtsapi_GeDeviceAllInfo()    NPISendSynchData(MT_SAPI_GET_DEV_ALL_INFO_REQ, NULL, 0)
int mtsapi_writeLogicalType(uint8_t logicaltype);
#define mtsapi_DataRequest(pData,len)    NPISendSynchData(MT_AF_DATA_REQUEST, pData, len)

void ZbInit(void);
uint8_t ZbisOnNet(void);


int mt_SyncHandle(uint16_t commandID, uint8_t *data, uint8_t len);
int mt_AsncHandle(uint16_t commandID, uint8_t *data, uint8_t len);

#endif


