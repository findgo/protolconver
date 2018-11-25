
#include "nwk.h"
#include "mt_sapi.h"
#include "prefix.h"

/* 网络层帧格式 发送模式
 * | byte|      2  |    2   |  1    |   1   | variable |
 * |     | dstaddr | dstaddr| fcctl |   seq | payload  |
*/
/* 网络层帧格式 发送模式
 
 * | byte|    2    |    2   |  1    |   1   | variable |
 * |     | srcaddr | dstaddr| fcctl |   seq | payload  |
 */
typedef struct {
    uint8_t fc;   // nwk 控制帧
    uint8_t seq;  // 序列号
    uint16_t dstaddr; // 目的地址
    uint16_t srcaddr; // 源地址
}nwkHdr_t;

extern void ltl_ProcessInApdu(MoIncomingMsgPkt_t *pkt);

static uint8_t *nwkParseHdr(nwkHdr_t *hdr, uint8_t *pDat);
static void nwk_ProcessInNpdu(nwkHdr_t *phdr, MoIncomingMsgPkt_t *pkt);

static msgboxstatic_t nwkmsgboxHandlebuf = MSGBOX_STATIC_INIT(MSGBOX_UNLIMITED_CAP);


void nwkInit(void)
{
    NWK_LowInit();
}

uint8_t nwkIsOnNet(void)
{
    return ZbisOnNet();
}
uint8_t nwkHdrLen(void)
{
    return ( 2 + 2 + 1 + 1 );// src addr + dst addr + fc + seq
}
// get past hdr pointer
uint8_t *nwkBuildHdr(uint8_t *pDat,uint8_t fc, uint16_t dstaddr, uint8_t seq)
{
    *pDat++ = LO_UINT16(dstaddr); // API需求目标地址
    *pDat++ = HI_UINT16(dstaddr);
    // real data to user
    *pDat++ = LO_UINT16(dstaddr); // NWK层需求
    *pDat++ = HI_UINT16(dstaddr);
    *pDat++ = fc;
    *pDat++ = seq;
    
    return pDat;
}

// get past hdr pointer
static uint8_t *nwkParseHdr(nwkHdr_t *hdr, uint8_t *pDat)
{
    hdr->srcaddr = BUILD_UINT16(*pDat, *(pDat + 1));
    pDat += 2;
    hdr->dstaddr = BUILD_UINT16(*pDat, *(pDat + 1));
    pDat += 2;
    hdr->fc = *pDat++;
    hdr->seq = *pDat++;
    
    return pDat;
}

// 网络层数据请求
int nwkDatareq(uint8_t dstaddr, uint8_t *dat, uint8_t len)
{
    uint8_t *pbuf;
    uint8_t *ptemp;
    int status;
    uint8_t size;

    size =  nwkHdrLen() + len;
    pbuf = mo_malloc(size);
    if(pbuf == NULL)
        return FALSE;

    ptemp = nwkBuildHdr(pbuf, NWK_FC_DATA, dstaddr, 0x00);

    memcpy(ptemp, dat, len);
    status = NWK_LowDataRequest(pbuf, size);
    mo_free(pbuf);

    return status;
}

// 网络层命令请求
static int nwkCmdreq(uint8_t dstaddr,uint8_t seq, uint8_t *dat, uint8_t len)
{
    uint8_t *pbuf;
    uint8_t *ptemp;
    int status;
    uint8_t size;

    size =  nwkHdrLen() + len;
    pbuf = mo_malloc(size);
    if(pbuf == NULL)
        return FALSE;

    ptemp = nwkBuildHdr(pbuf, NWK_FC_CMD, dstaddr, seq);

    memcpy(ptemp, dat, len);
    status = NWK_LowDataRequest(pbuf, size);
    mo_free(pbuf);

    return status;
}

// 从底层发送一个消息过网络层
int nwkmsgsend(void *msg_ptr)
{
    return msgBoxpost(&nwkmsgboxHandlebuf, msg_ptr);
}

void nwkTask(void)
{
    uint8_t *msg;
    MoIncomingMsgPkt_t pkt;
    nwkHdr_t hdr;

    msg = msgBoxaccept(&nwkmsgboxHandlebuf);
    while(msg)
    {
        log_debugln("nwk msg process!");
        // process message
        pkt.apduData = nwkParseHdr(&hdr, msg);
        pkt.apduLength = msglen(msg) - nwkHdrLen();
        pkt.srcaddr = hdr.srcaddr;
        
        if(hdr.dstaddr == NWK_BROADCAST_SHORTADDR_DEVZCZR 
            || hdr.dstaddr == NWK_BROADCAST_SHORTADDR_DEVRXON
            || hdr.dstaddr == NWK_BROADCAST_SHORTADDR_DEVALL)
            pkt.isbroadcast = TRUE;
        else
            pkt.isbroadcast = FALSE;
        
        if(hdr.fc == NWK_FC_DATA){  // 数据应用层
            ltl_ProcessInApdu(&pkt);
        }
        else if(hdr.fc == NWK_FC_CMD){ // 网络层
            nwk_ProcessInNpdu(&hdr, &pkt);
        }
        else if(hdr.fc == NWK_FC_PROTOCOL){ // 协议栈层
            // process protocol
        }
        else { // 保留,暂定
            
        // drop it
        }
        
        msgdealloc(msg);
        msg = msgBoxaccept(&nwkmsgboxHandlebuf);
    } 
}


static void nwk_ProcessInNpdu(nwkHdr_t *phdr, MoIncomingMsgPkt_t *pkt)
{



}

