

/* 这一层的作用是 前置填充网络层的数据头, 相头于直接填充头用向底层发送数据, 避免多次分配内存,提高效率 */
#ifndef __PREFIX_H__
#define __PREFIX_H__

#include "app_cfg.h"
#include "bxnwk.h"
//传进来的APDU包 或NPDU包
typedef struct
{
    uint8_t isbroadcast;
    uint16_t srcaddr;
    uint16_t apduLength; /* apdu length */
    uint8_t *apduData;  /* apdu pointer */
}MoIncomingMsgPkt_t;


//uint8_t ltlprefixHdrsize(uint8_t *pAddr);
//uint8_t *ltlPrefixBuildHdr( uint8_t *pAddr, uint8_t *pDat );
//uint8_t ltlPrefixrequest(uint8_t *pDat, uint16_t buflen);
// 如果采用前置填充的话, 发送将忽略 dstaddr 的标识 , 这里前置填充的网络层帧头和目标地址
// 主要是减少多次分配内存,但使用要注意
// 如果不习惯使用,  直接调用网络层数据发送API
// 提供网络帧头长度
#define ltlprefixHdrsize(dstAddr) bxNwkHdrLen()
// 填充网络帧头,并返回越过前置头的指置
#define ltlPrefixBuildHdr(dstAddr, pDat ) bxNwkBuildHdr(pDat, NWK_FC_DATA , dstAddr, 0)
// 直接发送
#define ltlPrefixrequest(dstaddr, pDat, buflen) bxNwk_LowDataRequest(dstaddr, pDat, buflen);  

#endif
