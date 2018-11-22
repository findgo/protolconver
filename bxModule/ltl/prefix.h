

/* 这一层的作用是 前置填充网络层的数据头, 相头于直接填充头用向底层发送数据, 避免多次分配内存,提高效率 */
#ifndef __PREFIX_H__
#define __PREFIX_H__

#include "app_cfg.h"
#include "nwk.h"
//传进来的APDU包 或NPDU包
typedef struct
{
    void *pAddr; 
    uint8_t isbroadcast;
    uint16_t srcaddr;
    uint16_t apduLength; /* apdu length */
    uint8_t *apduData;  /* apdu pointer */
}MoIncomingMsgPkt_t;


//uint8_t ltlprefixHdrsize(uint8_t *pAddr);
//uint8_t *ltlPrefixBuildHdr( uint8_t *pAddr, uint8_t *pDat );
//uint8_t ltlPrefixrequest(uint8_t *pDat, uint16_t buflen);
// 提供网络帧头长度
#define ltlprefixHdrsize(pAddr) nwkHdrLen()
// 填充网络帧头
#define ltlPrefixBuildHdr(pAddr, pDat ) nwkBuildHdr(pDat, NWK_FC_DATA , *((uint16_t *)pAddr), 0)
// 直接发送
#define ltlPrefixrequest(pDat, buflen) NWK_LowDataRequest(pDat, buflen);  

#endif
