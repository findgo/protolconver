#ifndef __MT_NPI_UART_H_
#define __MT_NPI_UART_H_

#include "mt_rpc.h"
#include "mt.h"
#include "app_cfg.h"
#include "common_type.h"
#include "mserial.h"
#include "bxnwk.h"


#define NPI_LOGICAL_TYPE    ZG_DEVICETYPE_ROUTER

#define NPI_LNX_SUCCESS     0
#define NPI_LNX_FAILURE     (-1)

// define uart interface
#define npi_write( buf, len)   Serial_WriteBuf(COM0,buf,len)
#define npi_read(buf,len)      Serial_Read(COM0,buf,len)
#define npi_readbufLen()       SerialRxValidAvail(COM0)

void npiInit(void);
void npiTask(void);
#define NPISendAsynchData(cmdId, pData, len) npisendframe(cmdId | MT_RPC_CMD_AREQ, pData, len)
#define NPISendSynchData(cmdId, pData, len) npisendframe(cmdId | MT_RPC_CMD_SREQ, pData, len)

// 内部API
int npisendframe(uint16_t cmd, uint8_t *pData, uint8_t len);

#endif
