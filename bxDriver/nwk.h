#ifndef __NWK_H__
#define __NWK_H__

#include "msglink.h"
#include "mt_sapi.h"

#define NWK_FC_DATA     0x00
#define NWK_FC_CMD      0x01
#define NWK_FC_PROTOCOL 0x02

// 定义网络层底层
#define NWK_LowInit() ZbInit()
#define NWK_LowDataRequest(pData, len)  mtsapi_DataRequest(pData, len)



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




uint8_t nwkIsOnNet(void);
int nwkmsgsend(void *msg_ptr);

uint8_t nwkHdrLen(void);
uint8_t *nwkBuildHdr(uint8_t *pDat,uint8_t fc, uint16_t dstaddr, uint8_t seq);

void nwkInit(void);
void nwkTask(void);

#endif

