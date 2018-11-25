#ifndef __MT_NPI_UART_H_
#define __MT_NPI_UART_H_

#include "mt_rpc.h"
#include "mt.h"
#include "app_cfg.h"
#include "common_type.h"
#include "mserial.h"
#include "nwk.h"


#define NPI_LOGICAL_TYPE  ZG_DEVICETYPE_ROUTER

#define NPI_LNX_SUCCESS     0
#define NPI_LNX_FAILURE     (-1)

typedef int (*npiSubsystemHandle_t)(uint8_t commandID, uint8_t *data, uint8_t len);

// define uart interface
#define npi_write( buf, len)   Serial_WriteBuf(COM0,buf,len)
#define npi_read(buf,len)      Serial_Read(COM0,buf,len)
#define npi_readbufLen()       SerialRxValidAvail(COM0)


#define NPISendAsynchData(subSys, cmdId, pData, len) npisendframe(subSys | MT_RPC_CMD_AREQ, cmdId, pData, len)
#define NPISendSynchData(subSys, cmdId, pData, len) npisendframe(subSys | MT_RPC_CMD_SREQ, cmdId, pData, len)

void npiInit(void);
void npiTask(void);
int npisendframe(uint8_t cmd0, uint8_t cmd1, uint8_t *pData, uint8_t len);

#endif
