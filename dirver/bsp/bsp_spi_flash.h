/**
  ******************************************************************************
  * @file   hal_spi.h
  * @author  
  * @version 
  * @date    
  * @brief    
  ******************************************************************************
  * @attention    v1.1      jgb                 20170515 
  ******************************************************************************
  */

#ifndef __BSP_SPI_FLASH_H_
#define __BSP_SPI_FLASH_H_

/* spi flash 需要spi支持模式3 */
#include "app_cfg.h"
#include "hal_spi.h"

/* 使能高级写模式，将会开辟4K Ram 扇区空间用于读写缓冲 */
#define SF_FLASH_WRITE_ADVANCED_MODE    (0)

// 抽像SPI底层
#define SF_SPI_CS_ASSERT()   HAL_SPI1_CS_LOW()  
#define SF_SPI_CS_DEASSERT() HAL_SPI1_CS_HIGH() 
#define sf_spiReadWriteByte(_ucValue)  halSPI1_ReadWrite(_ucValue)

// 具体芯片参数
#define SF_PAGE_SIZE        (uint16_t)0x100   // 256
#define SF_SECTOR_SIZE      (uint16_t)0x1000  // 4K
#define SF_TOTAL_SIZE       (uint32_t)(8 * 1024 * 1024)  // 64M bit 8M byte

#define SF_TOTAL_SECTOR_CNT     (SF_TOTAL_SIZE / SF_SECTOR_SIZE)
#define SF_PAGE_IN_SECTOR_CNT   (SF_SECTOR_SIZE / SF_PAGE_SIZE)

/* 定义串行Flash ID */
enum
{
    MX25L1606E_ID  = 0xC22015,
    W25Q256FV_ID   = 0xEF6019,
    W25Q128FV_ID   = 0xEF6018,
    
    W25Q64FV_ID    = 0xEF6017,
    W25Q64BV_ID    = 0xEF4017,
    W25X64_ID      = 0xEF3017,
    
    W25Q32FV_ID    = 0xEF6016,
    W25Q32BV_ID    = 0xEF4016,
    W25X32_ID      = 0xEF3016,
    
    W25Q16BV_ID    = 0xEF4015,
    W25X16_ID      = 0xEF3015,
    
    W25Q80DV_ID    = 0xEF4014,
    W25X80_ID      = 0xEF3014,
    
    W25Q40BW_ID    = 0xEF5013,
    W25X40_ID      = 0xEF3013,

    W25X20_ID      = 0xEF3012,

    W25X10_ID      = 0xEF3011,
    
    GD25Q40B_ID    = 0xC84013,
    GD25Q20B_ID    = 0xC84012
};

typedef struct
{
    uint32_t ChipID;        /* 芯片ID */
    uint16_t PageSize;      /* 页面大小 */
    uint16_t SectorSize;    /* 扇区大小 */  
    uint32_t TotalSize;     /* 总容量 */
}sflash_info_t;

typedef enum 
{
	BE32,
	BE64
}beraseType_t;

// 初始化flash , 在这之前先初始化spi
void sf_InitFlash(void);
// 读芯片信息
sflash_info_t *sf_info(void);
// 读id
uint32_t sf_ReadID(void);
/* 擦除扇区, 扇起始地址 */
void sf_EraseSector(uint32_t _uiSectorStartAddr);
// 擦除指定, 扇区号码 sc < SF_TOTAL_SECTOR_CNT  
#define sf_EraseSectorSC(sc)        sf_EraseSector(sc * SF_SECTOR_SIZE);
/* 擦除扇区, 块起始始地址 */
void sf_EraseBlock(uint32_t _uiBlockAddr,beraseType_t betype);
/* 擦除芯片 */
void sf_EraseChip(void);
/* 读数据可以任意地址,   不得超出整个芯片地址范围
 * TRUE: 成功, 否则FALSE*/
uint8_t sf_Read(uint32_t _uiReadAddr, uint8_t * _pBuf,uint32_t _uiSize);

#if SF_FLASH_WRITE_ADVANCED_MODE == 1
/* 写数据可以任意地址,   不得超出整个芯片地址范围
 * 具有自动擦除和纠错功能,需要开4K扇区地址 
 * 如果发现写地方的数据不一样,可能会引起擦除*/
uint8_t sf_Write(uint32_t _uiWriteAddr, uint8_t* _pBuf,  uint32_t _usWriteSize);
#endif

/*************************以下只写数据,不对数据进行任何校验 *************************/
/* 在一页内写数据, 0 - 256字节, 必需保证实的数据在同一页内,否则会写到页前面去  */
uint8_t sf_WriteWithinOnePage(uint32_t _PageAddr,uint8_t * _pBuf,uint32_t NumByteToWrite);
/* 写多个整页数据, _PageStartAddr 页起始地址
NumByteToWrite : 为整页大小的倍数      */
uint8_t sf_WriteMulWholePage(uint32_t _PageStartAddr,uint8_t * _pBuf,uint32_t NumByteToWrite);
/* 写数据可以任意地址,   不得超出整个芯片地址范围*/
uint8_t sf_WriteBuffer(uint32_t _uiWriteAddr, uint8_t * _pBuf, uint32_t _usWriteSize);



// 获得芯片当前是否忙
uint8_t sf_StatusBusy(void);
// 启动一个擦扇区序列 
void sf_StartEraseSectoreSequeue(uint32_t _uiSectorAddr); 
// 启动一个写页数据序列  NumByteToWrite <= 256
void sf_StartWritePageSequeue(uint32_t _PageAddr,uint8_t * _pBuf,uint16_t NumByteToWrite);

#endif

