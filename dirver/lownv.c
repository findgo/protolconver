#include "lownv.h"

/**************************************************************************************************
 * @brief       This function reads 'bytecnt' bytes from the internal flash.
 *
 * @param       pg - Valid HAL flash page number (ie < 128).
 * @param       offset - Valid offset into the page (so < HAL_NV_PAGE_SIZE and byte-aligned is ok).
 * @param       buf - Valid buffer space at least as big as the 'bytecnt' parameter.
 * @param       bytecnt - Valid number of bytes to read: a read cannot cross into the next 32KB bank.
 *
 * @return      None.
 **************************************************************************************************
 */
void HalFlashRead(uint8_t pg, uint16_t offset, uint8_t *buf, uint16_t bytecnt)
{
    uint32_t addr = HAL_FLASH_BASE_ADDRESS + pg * HAL_FLASH_PAGE_SIZE + offset;

    while(bytecnt--)
    {
        *buf++ = *(volatile uint8_t *)addr;        
        addr++;
    }
}
/**************************************************************************************************

 * @brief       This function reads 'cnt' bytes from the internal flash.
 *
 * @param       pg - Valid HAL flash page number (ie < 128).
 * @param       offset - Valid HAL flash write offse into the apge:  quad-aligned.
 * @param       buf - Valid buffer space at least as big as 'wordcnt' X 4.
 * @param       wordcnt - Number of 4-byte blocks to write.
 *
 * @return      None.
 **************************************************************************************************
 */
void HalFlashWrite(uint8_t pg, uint16_t offset, uint8_t *buf, uint16_t wordcnt)
{
    uint32_t temp;
    FLASH_Status sta;
    uint32_t addr = HAL_FLASH_BASE_ADDRESS + pg * HAL_FLASH_PAGE_SIZE + offset;
    
    FLASH_Unlock();
    while(wordcnt--)
    {
        temp = BUILD_UINT32(buf[0], buf[1], buf[2], buf[3]);
        sta = FLASH_ProgramWord(addr, temp);
        buf += sizeof(uint32_t);
        addr += sizeof(uint32_t);
        temp += sta;
    }
    FLASH_Lock();
}


void HalFlashWriteHalfWord(uint8_t pg, uint16_t offset, uint16_t data)
{
    FLASH_Unlock();
    FLASH_ProgramHalfWord(HAL_FLASH_BASE_ADDRESS + pg * HAL_FLASH_PAGE_SIZE + offset, data);
    FLASH_Lock();
}
/**************************************************************************************************
 * @brief       This function erases 'cnt' pages of the internal flash.
 *
 * @param       pg - Valid HAL flash page number (ie < 128) to erase.
 *
 * @return      None.
 **************************************************************************************************
 */
void HalFlashErase(uint8_t pg)
{
    FLASH_Unlock();
    (void)FLASH_ErasePage(HAL_FLASH_BASE_ADDRESS + pg * HAL_FLASH_PAGE_SIZE);
    FLASH_Lock();
}



