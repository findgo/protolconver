
#include <stdint.h>
/*********************************************************************
 * @fn     
 *
 * @brief   header of the prefix length
 *
 * @param   refer - in ---- information
 *
 * @return  the header length
 */
uint8_t ltlprefixsize(uint8_t *refer)
{

    return 0;
}

/*********************************************************************
 * @fn     
 *
 * @brief   Build header of the prefix format
 *
 * @param   refer - in ---- information
 * @param   pDat -out ---- outgoing header space
 *
 * @return  pointer past the header
 */

uint8_t *ltlPrefixBuildHdr( uint8_t *refer, uint8_t *pDat )
{
    
    return pDat;
}

void ltlrequest(uint8_t *pbuf,uint16_t buflen)
{
    (void)pbuf;
    (void)buflen;
}

