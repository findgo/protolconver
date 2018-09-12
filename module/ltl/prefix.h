
#ifndef __PREFIX_H__
#define __PREFIX_H__


uint8_t ltlprefixsize(uint8_t *refer);
uint8_t *ltlPrefixBuildHdr( uint8_t *refer, uint8_t *pDat );
void ltlrequest(void *refer, uint8_t *pbuf,uint16_t buflen);
void ltlApduParsing(uint16_t src_addr, uint8_t *apdu, uint8_t apdu_len);



#endif
