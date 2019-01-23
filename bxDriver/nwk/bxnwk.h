#ifndef __BX_NWK_H__
#define __BX_NWK_H__

#include "prefix.h"
#include "msglink.h"
#include "mt_app.h"


// 定义网络层底层
#define bxNWK_LowInit() ZbInit()
#define bxNwk_LowDataRequest(dstaddr, pData, len)  mtsapi_DataRequest(pData, len)

uint8_t bxNwkIsOnNet(void);
uint8_t *bxNwkBuildHdr(uint8_t *pDat, uint16_t dstaddr);
int bxNwkmsgsend(void *msg_ptr);

void bxNwkInit(void);
void bxNwkTask(void);

#endif

