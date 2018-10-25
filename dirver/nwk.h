#ifndef __NWK_H__
#define __NWK_H__

#define NWK_FC_DATA     0x00
#define NWK_FC_CMD      0x01






#define NWK_MSG_ID       0x0001  // zigbee模块发给网络层的消息

#define NWK_MSG_EVENT    EVENTGROUP_BITS_0

typedef struct {
    uint16_t spare;
    uint16_t len;
    uint8_t *pdat;
}nwk_msg_t;



void *nwkmsgallocate(uint16_t len);
void nwkmsgsend(void *msg_ptr);
void nwkmsgdeallocate(void *msg_ptr);

uint8_t nwkHdrLen(void);
uint8_t *nwkDataBuildHdr(uint8_t *pDat, uint16_t dstaddr);
#define nwkreq(dstaddr, dat, length)  ebyte_OSPFreq(dstaddr, dat, length)
void nwkInit(void);
void nwkTask(void);

#endif

