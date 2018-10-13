#include "lownv.h"


void HalFlashRead(uint8_t pg, uint16_t offset, uint8_t *buf, uint16_t bytecnt)
{
    uint32_t addr = HAL_FLASH_BASE_ADDRESS + pg * HAL_FLASH_PAGE_SIZE + offset;

    while(bytecnt--)
    {
        *buf++ = *(volatile uint8_t *)addr;        
        addr++;
    }
}
void HalFlashWriteMulWord(uint8_t pg, uint16_t offset, uint8_t *buf, uint16_t wordcnt)
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
void HalFlashErase(uint8_t pg)
{
    FLASH_Unlock();
    (void)FLASH_ErasePage(HAL_FLASH_BASE_ADDRESS + pg * HAL_FLASH_PAGE_SIZE);
    FLASH_Lock();
}



