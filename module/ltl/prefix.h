
#ifndef __PREFIX_H__
#define __PREFIX_H__

#include "app_cfg.h"

//传进来的APDU包 或NPDU包
typedef struct
{
    void *refer; 
    uint16_t apduLength; /* apdu length */
    uint8_t *apduData;  /* apdu pointer */
}MoIncomingMsgPkt_t;


uint8_t ltlprefixsize(uint8_t *refer);
uint8_t *ltlPrefixBuildHdr( uint8_t *refer, uint8_t *pDat );
uint8_t ltlrequest(void *refer, uint8_t *pbuf,uint16_t buflen);



#endif
