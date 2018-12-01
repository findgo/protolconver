
#include "mt_af.h"


/*
int mtaf_Register(endPointDesc_t *endPointDesc)
{
    uint8_t *pbuf;
    uint8_t *buf;
    uint8_t flen;
    int status;
    uint8_t i;
    
    flen = (endPointDesc->AppNumInClusters + endPointDesc->AppNumOutClusters) * sizeof(uint16_t) + 9;
    buf = (uint8_t *) mo_malloc(flen);
    if(pbuf  == NULL)
        return NPI_LNX_FAILURE;

    pbuf = buf;

    *pbuf++ = endPointDesc->endPoint;
    *pbuf++ = LO_UINT16(endPointDesc->AppProfId);
    *pbuf++ = HI_UINT16(endPointDesc->AppProfId);
    *pbuf++ = LO_UINT16(endPointDesc->AppDeviceId);
    *pbuf++ = HI_UINT16(endPointDesc->AppDeviceId);
    
    *pbuf++ = endPointDesc->latencyReq;
    
    *pbuf++ = endPointDesc->AppNumInClusters;
    for(i = 0; i < endPointDesc->AppNumInClusters; i++){
        *pbuf++ = LO_UINT16(endPointDesc->pAppInClusterList[i]);
        *pbuf++ = HI_UINT16(endPointDesc->pAppInClusterList[i]);
    }
    
    *pbuf++ = endPointDesc->AppNumOutClusters;
    for(i = 0; i < endPointDesc->AppNumOutClusters; i++){
        *pbuf++ = LO_UINT16(endPointDesc->pAppOutClusterList[i]);
        *pbuf++ = HI_UINT16(endPointDesc->pAppOutClusterList[i]);
    }

    status = MTAF_SendSynchData(MT_AF_REGISTER, buf, flen)

    mo_free(buf);

    return status;
}
*/
// 注意,这里目的地址交给nwk层管理,前两字节必须为目地地址
//int mtaf_DataRequest(uint8_t *dat, uint8_t len)
//{
//    return MTAF_SendSynchData(MT_AF_REGISTER, dat, len);
//}


int mtaf_SyncHandle(uint8_t commandID, uint8_t *data, uint8_t len)
{
    return NPI_LNX_SUCCESS;
}


int mtaf_AsncHandle(uint8_t commandID, uint8_t *data, uint8_t len)
{
    uint8_t *msg;
    
    switch (commandID){
    case MT_AF_INCOMING_MSG:
        
        if((msg = msgalloc(len)) == NULL)
            return NPI_LNX_FAILURE;
        log_debug("af incoming msg!");
        bxNwkmsgsend(msg); // 将消息发送nwk邮箱
        break;
        
    default:
        break;
            
    }

    return NPI_LNX_SUCCESS;
}

