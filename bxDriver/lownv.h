

#ifndef __LOWNV_H__
#define __LOWNV_H__


#include "common_type.h"
#include "common_def.h"
#include "stm32f1xx.h"
/*********************************************************************
 * specific in stm32F1 series
 */
/* Exported constants --------------------------------------------------------*/
/* Define the STM32F10Xxx Flash page size depending on the used STM32 device */
#if defined (STM32F10X_LD) || defined (STM32F10X_MD)
  #define HAL_FLASH_PAGE_SIZE  (uint16_t)0x400  /* Page size = 1KByte */
#elif defined (STM32F10X_HD) || defined (STM32F10X_CL)
  #define HAL_FLASH_PAGE_SIZE  (uint16_t)0x800  /* Page size = 2KByte */
#endif

#define HAL_FLASH_BASE_ADDRESS FLASH_BASE   


/* nv start page in Flash */
#define HAL_NV_PAGE_BEG    (124)
#define HAL_NV_PAGE_CNT     4


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
void HalFlashRead(uint8_t pg, uint16_t offset, uint8_t *buf, uint16_t bytecnt);

/**************************************************************************************************

 * @brief       This function write 'wordcnt' word into the internal flash.
 *
 * @param       pg - Valid HAL flash page number (ie < 128).
 * @param       offset - Valid HAL flash write offse into the apge:  quad-aligned.
 * @param       buf - Valid buffer space at least as big as 'wordcnt' X 4.
 * @param       wordcnt - Number of 4-byte blocks to write.
 *
 * @return      None.
 **************************************************************************************************
 */
void HalFlashWriteMulWord(uint8_t pg, uint16_t offset, uint8_t *buf, uint16_t wordcnt);
/**************************************************************************************************

 * @brief       This function write a word into the internal flash.
 *
 * @param       pg - Valid HAL flash page number (ie < 128).
 * @param       offset - Valid HAL flash write offse into the apge:  quad-aligned.
 * @param       data- Valid data.
 *
 * @return      None.
 **************************************************************************************************
 */
void HalFlashWriteHalfWord(uint8_t pg, uint16_t offset, uint16_t data);

/**************************************************************************************************
 * @brief       This function erases the pages of the internal flash.
 *
 * @param       pg - Valid HAL flash page number (ie < 128) to erase.
 *
 * @return      None.
 **************************************************************************************************
 */
void HalFlashErase(uint8_t pg);




#endif
