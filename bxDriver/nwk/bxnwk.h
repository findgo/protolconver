#ifndef __BX_NWK_H__
#define __BX_NWK_H__

#include "prefix.h"
#include "msglink.h"
#include "mt_app.h"

#define NWK_FC_DATA     0x00
#define NWK_FC_CMD      0x01
#define NWK_FC_PROTOCOL 0x02

// 定义网络层底层
#define bxNWK_LowInit() ZbInit()
#define bxNwk_LowDataRequest(dstaddr, pData, len)  mtsapi_DataRequest(pData, len)



typedef enum  bcast_addr_s {
  NWK_BROADCAST_SHORTADDR_RESRVD_F8  = 0xFFF8,
  NWK_BROADCAST_SHORTADDR_RESRVD_F9,
  NWK_BROADCAST_SHORTADDR_RESRVD_FA,
  NWK_BROADCAST_SHORTADDR_RESRVD_FB,
  NWK_BROADCAST_SHORTADDR_DEVZCZR,            // 0xFFFC: Routers and Coordinators
  NWK_BROADCAST_SHORTADDR_DEVRXON,            // 0xFFFD: Everyone with RxOnWhenIdle == TRUE
                                              // 0xFFFE: Reserved (legacy: used for 'invalid address')
  NWK_BROADCAST_SHORTADDR_DEVALL     = 0xFFFF
}bcast_addr_t;




uint8_t bxNwkIsOnNet(void);
int bxNwkmsgsend(void *msg_ptr);

uint8_t bxNwkHdrLen(void);
uint8_t *bxNwkBuildHdr(uint8_t *pDat,uint8_t fc, uint16_t dstaddr, uint8_t seq);

void bxNwkInit(void);
void bxNwkTask(void);

#endif

