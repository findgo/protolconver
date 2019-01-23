
#include "bxnwk.h"

/* 网络层帧格式 发送模式
 * | byte|      2  | variable |
 * |     | dstaddr | payload  |
*/
/* 网络层帧格式 发送模式
 
 * | byte|    2    |        1      | variable |
 * |     | srcaddr | wasbroadcast  | payload  |
 */

extern void ltl_ProcessInApdu(MoIncomingMsgPkt_t *pkt);

static msgbox_t nwkmsgboxHandlebuf = MSGBOX_STATIC_INIT(MSGBOX_UNLIMITED_CAP);

void bxNwkInit(void)
{
    bxNWK_LowInit();
}

uint8_t bxNwkIsOnNet(void)
{
    return ZbisOnNet();
}

// get past hdr pointer
uint8_t *bxNwkBuildHdr(uint8_t *pDat, uint16_t dstaddr)
{
    *pDat++ = LO_UINT16(dstaddr); // API需求目标地址
    *pDat++ = HI_UINT16(dstaddr);

    return pDat;
}

// 从底层发送一个消息到网络层
int bxNwkmsgsend(void *msg_ptr)
{
    return msgBoxpost(&nwkmsgboxHandlebuf, msg_ptr);
}

void bxNwkTask(void)
{
    uint8_t *msg;
    MoIncomingMsgPkt_t pkt;

    msg = msgBoxaccept(&nwkmsgboxHandlebuf);
    while(msg)
    {
        log_debugln("nwk msg process!");
        // process message
        
        pkt.srcaddr = BUILD_UINT16(msg[0], msg[1]);
        pkt.isbroadcast = msg[2];
        pkt.apduData = &msg[3];
        pkt.apduLength = msglen(msg) - 3;
         
        ltl_ProcessInApdu(&pkt);
 
        msgdealloc(msg);
        msg = msgBoxaccept(&nwkmsgboxHandlebuf);
    } 
}

