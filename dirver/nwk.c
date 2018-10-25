
#include "message.h"
#include "nwk.h"
#include "ebyteZB.h"
#include "prefix.h"

#define nwkmsgreceive() msg_queuepop(&nwk_msgq)
#define nwkmsglen(msg_ptr) msg_len(msg_ptr)

typedef struct {
    uint8_t fc;
    uint8_t seq;
    uint16_t dstaddr;
    uint16_t srcaddr;
}nwk_Hdr_t;

static uint8_t *nwkParseHdr(nwk_Hdr_t *hdr, uint8_t *pDat);

extern void ltl_ProcessInApdu(MoIncomingMsgPkt_t *pkt);
static void nwk_ProcessInNpdu(MoIncomingMsgPkt_t *pkt);





static msg_q_t nwk_msgq = NULL;


void nwkInit(void)
{
    ebyteZBInit();
}

void *nwkmsgallocate(uint16_t len)
{
    return msg_allocate(len);
}
void nwkmsgsend(void *msg_ptr)
{
    msg_queuecput(&nwk_msgq, msg_ptr);
}
void nwkmsgdeallocate(void *msg_ptr)
{
    msg_deallocate(msg_ptr);  
}

uint8_t nwkHdrLen(void)
{
    return (1 + 2 + 2 + 1);// fc + src addr + dst addr + seq
}

static uint8_t *nwkBuildHdr(uint8_t *pDat,uint8_t fc, uint16_t srcaddr, uint16_t dstaddr, uint8_t seq)
{
    *pDat++ = fc;
    *pDat++ = LO_UINT16(dstaddr);
    *pDat++ = HI_UINT16(dstaddr);
    *pDat++ = LO_UINT16(srcaddr);
    *pDat++ = HI_UINT16(srcaddr);
    *pDat++ = seq;
    
    return pDat;
}
// 数据上层专用
uint8_t *nwkDataBuildHdr(uint8_t *pDat, uint16_t dstaddr)
{
    return nwkBuildHdr(pDat, NWK_FC_DATA ,ebyte_nwkAddr(), dstaddr, 0);
}

// get past hdr pointer
static uint8_t *nwkParseHdr(nwk_Hdr_t *hdr, uint8_t *pDat)
{
    hdr->fc = *pDat++;
    hdr->srcaddr = BUILD_UINT16(*pDat, *(pDat + 1));
    pDat += 2;
    hdr->dstaddr = BUILD_UINT16(*pDat, *(pDat + 1));
    pDat += 2;
    hdr->seq = *pDat++;
    
    return pDat;
}


void nwkTask(void)
{
    uint8_t *msg;
    MoIncomingMsgPkt_t pkt;
    nwk_Hdr_t hdr;
    
    msg = nwkmsgreceive();

    while(msg)
    {
        mo_logln(DEBUG, "OSPF msg process!");
        // process you message
        pkt.apduData = nwkParseHdr(&hdr, msg);
        pkt.apduLength = nwkmsglen(msg) - nwkHdrLen();
        if(hdr.fc == NWK_FC_DATA){
            pkt.refer = (void *)&(hdr.srcaddr);
            ltl_ProcessInApdu(&pkt);
        }
        else if(hdr.fc == NWK_FC_CMD){
            pkt.refer = (void *)&hdr;
            nwk_ProcessInNpdu(&pkt);
        }
        else{
            // drop it
        }
        
        msg_deallocate(msg);
        msg = nwkmsgreceive();
    } 
}


static void nwk_ProcessInNpdu(MoIncomingMsgPkt_t *pkt)
{
           
}

