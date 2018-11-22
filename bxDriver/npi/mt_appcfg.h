#ifndef __MT_APPCFG_H_
#define __MT_APPCFG_H_

#include "mt_npi.h"





// 本地宏
#define MTAPPCFG_SendAsynchData(cmdID,dat,len)     NPISendAsynchData(MT_RPC_SYS_APPCFG, cmdID, dat, len)
#define MTAPPCFG_SendSynchData(cmdID,dat,len)      NPISendSynchData(MT_RPC_SYS_APPCFG, cmdID, dat, len)

// sync command 





int mtappcfg_SyncHandle(uint8_t commandID, uint8_t *data, uint8_t len);
int mtappcfg_AsncHandle(uint8_t commandID, uint8_t *data, uint8_t len);

#endif

