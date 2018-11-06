

#ifndef __MT_AF_H__
#define __MT_AF_H__

#include "mt_npi.h"

/***************************************************************************************************
 * AF COMMANDS
 ***************************************************************************************************/

/* SREQ/SRSP */
#define MT_AF_REGISTER                       0x00
#define MT_AF_DATA_REQUEST                   0x01  /* AREQ optional, but no AREQ response. */
#define MT_AF_DATA_REQUEST_EXT               0x02  /* AREQ optional, but no AREQ response. */
#define MT_AF_DATA_REQUEST_SRCRTG            0x03
#define MT_AF_DELETE                         0x04

#define MT_AF_INTER_PAN_CTL                  0x10
#define MT_AF_DATA_STORE                     0x11
#define MT_AF_DATA_RETRIEVE                  0x12
#define MT_AF_APSF_CONFIG_SET                0x13
#define MT_AF_APSF_CONFIG_GET                0x14

/* AREQ to host */
#define MT_AF_DATA_CONFIRM                   0x80
#define MT_AF_INCOMING_MSG                   0x81
#define MT_AF_INCOMING_MSG_EXT               0x82
#define MT_AF_REFLECT_ERROR                  0x83



// 本地宏
#define MTAF_SendAsynchData(cmdID,dat,len)     NPISendAsynchData(MT_RPC_SYS_AF, cmdID, dat, len)
#define MTAF_SendSynchData(cmdID,dat,len)      NPISendSynchData(MT_RPC_SYS_AF, cmdID, dat, len)

// asnc command 


// sync command 
// 注意,这里目的地址交给nwk层管理,前两字节必须为目地地址
#define mtaf_DataRequest(dat, len)  MTAF_SendSynchData(MT_AF_REGISTER, dat, len)


int mtaf_SyncHandle(uint8_t commandID, uint8_t *data, uint8_t len);
int mtaf_AsncHandle(uint8_t commandID, uint8_t *data, uint8_t len);

#endif

