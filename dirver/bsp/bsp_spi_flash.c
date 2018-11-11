
#include "bsp_spi_flash.h"


#define SF_CMD_AAI       0xAD       /* AAI 连续编程指令(FOR SST25VF016B) */
#define SF_CMD_DISWR      0x04      /* 禁止写, 退出AAI状态 */
#define SF_CMD_EWRSR      0x50      /* 允许写状态寄存器的命令 */
 
#define SF_CMD_WRITE     0x02       /*!< Write to Memory instruction */
#define SF_CMD_RDSR      0x05       /*!< Write Status Register instruction */
#define SF_CMD_WRSR      0x01       /*!< Write enable instruction */
#define SF_CMD_WREN      0x06       /*!< Read from Memory instruction */
#define SF_CMD_READ      0x03       /*!< Read Status Register instruction  */
#define SF_CMD_RDID      0x9F       /*!< Read identification */
#define SF_CMD_SE        0x20       /*!< Sector Erase instruction */
#define SF_CMD_32KBE	 0x52		/*!< 32k Block Erase instruction */
#define SF_CMD_64KBE	 0xD8		/*!< 64k Erase instruction */
#define SF_CMD_CE        0xC7       /*!< Chip Erase instruction */
#define SF_DUMMY_BYTE    0xA5       /*!< Dummy commad */

#define WIP_FLAG      0x01          /*!< Write In Progress (WIP) flag */



static sflash_info_t g_sf_info;

static void __sf_WriteEnable(void)
{
    /*!< Select the FLASH: Chip Select low */
    SF_SPI_CS_ASSERT();                                 
    /*!< Send "Write Enable" instruction */
    sf_spiReadWriteByte(SF_CMD_WREN);                       
    /*!< Deselect the FLASH: Chip Select high */
    SF_SPI_CS_DEASSERT();                               
}

static void __sf_WaitForWriteEnd(void)
{
    /*!< Select the FLASH: Chip Select low */
    SF_SPI_CS_ASSERT();     
    /*!< Send "Read Status Register" instruction */
    sf_spiReadWriteByte(SF_CMD_RDSR);       
    /*!< Send a dummy byte to generate the clock needed by the FLASH
    and put the value of the status register in FLASH_Status variable */
    while(sf_spiReadWriteByte(SF_DUMMY_BYTE) & WIP_FLAG);

    /*!< Deselect the FLASH: Chip Select high */
    SF_SPI_CS_DEASSERT();                               
}

/*
*********************************************************************************************************
*   函 数 名: sf_WriteStatus
*   功能说明: 写状态寄存器
*   形    参:  _ucValue : 状态寄存器的值
*   返 回 值: 无
*********************************************************************************************************
*/
static void __sf_WriteStatus(uint8_t _ucValue)
{
    SF_SPI_CS_ASSERT();                                 /* 使能片选 */
    sf_spiReadWriteByte(SF_CMD_WRSR);                   /* 发送命令， 写状态寄存器 */
    sf_spiReadWriteByte(_ucValue);                      /* 发送数据：状态寄存器的值 */
    SF_SPI_CS_DEASSERT();                               /* 禁能片选 */
}

uint8_t sf_StatusBusy(void)
{
    uint8_t status;
    
    /*!< Select the FLASH: Chip Select low */
    SF_SPI_CS_ASSERT();     
    /*!< Send "Read Status Register" instruction */
    sf_spiReadWriteByte(SF_CMD_RDSR);       
    /*!< Send a dummy byte to generate the clock needed by the FLASH
    and put the value of the status register in FLASH_Status variable */
    status = sf_spiReadWriteByte(SF_DUMMY_BYTE) & WIP_FLAG;
    /*!< Deselect the FLASH: Chip Select high */
    SF_SPI_CS_DEASSERT();

    return status;
}

void __sf_ReadInfo(void)
{
    g_sf_info.ChipID = sf_ReadID();/* 芯片ID */
    
    switch (g_sf_info.ChipID){
    case W25Q256FV_ID:
        g_sf_info.PageSize = SF_PAGE_SIZE;          /* 页面大小 = 256字节 */
        g_sf_info.SectorSize = SF_SECTOR_SIZE;      /* 扇区大小 = 4K */
        g_sf_info.TotalSize = 32 * 1024 * 1024;     /* 总容量 = 32M */
        break;

    case W25Q128FV_ID:
        g_sf_info.PageSize = SF_PAGE_SIZE;          /* 页面大小 = 256字节 */
        g_sf_info.SectorSize = SF_SECTOR_SIZE;      /* 扇区大小 = 4K */
        g_sf_info.TotalSize = 16 * 1024 * 1024;     /* 总容量 = 16M */
        break;

    case W25Q64FV_ID:
    case W25Q64BV_ID:
    case W25X64_ID:
        g_sf_info.PageSize = SF_PAGE_SIZE;          /* 页面大小 = 256字节 */
        g_sf_info.SectorSize = SF_SECTOR_SIZE;      /* 扇区大小 = 4K */
        g_sf_info.TotalSize = 8 * 1024 * 1024;      /* 总容量 = 8M */
        break;
        
    case W25Q32FV_ID:
    case W25Q32BV_ID:
    case W25X32_ID:
        g_sf_info.PageSize = SF_PAGE_SIZE;           /* 页面大小 = 256字节 */
        g_sf_info.SectorSize = SF_SECTOR_SIZE;       /* 扇区大小 = 4K */
        g_sf_info.TotalSize = 4 * 1024 * 1024;      /* 总容量 = 4M */
        break;
        
    case MX25L1606E_ID:
    case W25Q16BV_ID:
    case W25X16_ID:
        g_sf_info.PageSize = SF_PAGE_SIZE;           /* 页面大小 = 256字节 */
        g_sf_info.SectorSize = SF_SECTOR_SIZE;       /* 扇区大小 = 4K */
        g_sf_info.TotalSize = 2 * 1024 * 1024;      /* 总容量 = 2M */
        break;
    
    case W25Q80DV_ID:
    case W25X80_ID:
        g_sf_info.PageSize = SF_PAGE_SIZE;          /* 页面大小 = 256字节 */
        g_sf_info.SectorSize = SF_SECTOR_SIZE;      /* 扇区大小 = 4K */
        g_sf_info.TotalSize = 1 * 1024 * 1024;      /* 总容量 = 1M */
        break;
    
    case W25Q40BW_ID:
    case W25X40_ID:
    case GD25Q40B_ID:
        g_sf_info.PageSize = SF_PAGE_SIZE;           /* 页面大小 = 256字节 */
        g_sf_info.SectorSize = SF_SECTOR_SIZE;      /* 扇区大小 = 4K */
        g_sf_info.TotalSize = 512 * 1024;           /* 总容量 = 512k */
        break;

    case W25X20_ID:
    case GD25Q20B_ID:
        g_sf_info.PageSize = SF_PAGE_SIZE;          /* 页面大小 = 256字节 */
        g_sf_info.SectorSize = SF_SECTOR_SIZE;      /* 扇区大小 = 4K */
        g_sf_info.TotalSize = 256 * 1024;           /* 总容量 = 256k */
        break;

    case W25X10_ID:
        g_sf_info.PageSize = SF_PAGE_SIZE;           /* 页面大小 = 256字节 */
        g_sf_info.SectorSize = SF_SECTOR_SIZE;      /* 扇区大小 = 4K */
        g_sf_info.TotalSize = 128 * 1024;           /* 总容量 = 128K */
        break;

    default:
        g_sf_info.PageSize = 0;
        g_sf_info.SectorSize = 0;
        g_sf_info.TotalSize = 0;
        break;
    }
}

sflash_info_t *sf_info(void)
{
    return &g_sf_info;
}

uint32_t sf_ReadID(void)
{
    uint32_t uiID;
    uint8_t id1, id2, id3;

    SF_SPI_CS_ASSERT(); 
    /*!< Send "RDID " instruction */
    sf_spiReadWriteByte(SF_CMD_RDID);   
    /*!< Read a byte from the FLASH */
    id1 = sf_spiReadWriteByte(SF_DUMMY_BYTE);               
    id2 = sf_spiReadWriteByte(SF_DUMMY_BYTE);               
    id3 = sf_spiReadWriteByte(SF_DUMMY_BYTE);
    
    SF_SPI_CS_DEASSERT();                               

    uiID = ((uint32_t)id1 << 16) | ((uint32_t)id2 << 8) | id3;

    return uiID;
}

void sf_EraseSector(uint32_t _uiSectorStartAddr)
{
    // 超出地址范围直接不处理
    if(_uiSectorStartAddr >=  g_sf_info.TotalSize)
        return;

    _uiSectorStartAddr &= ~(g_sf_info.SectorSize - 1); // 防止出错,处理成起始地址
    
    /*!< Send write enable instruction */
    __sf_WriteEnable();                             

    SF_SPI_CS_ASSERT();                                 
    /*!< Send Sector Erase instruction */
    sf_spiReadWriteByte(SF_CMD_SE);
    /*!< Send Sector Erase instruction */
    sf_spiReadWriteByte((_uiSectorStartAddr & 0xFF0000) >> 16);
    sf_spiReadWriteByte((_uiSectorStartAddr & 0xFF00) >> 8); 
    sf_spiReadWriteByte(_uiSectorStartAddr & 0xFF);          
    SF_SPI_CS_DEASSERT();                               

    /*!< Wait the end of Flash writing */
    __sf_WaitForWriteEnd();                         
}
void sf_EraseBlock(uint32_t _uiBlockStartAddr, beraseType_t betype)
{
    // 超出地址范围直接不处理
    if(_uiBlockStartAddr >=  g_sf_info.TotalSize)
        return;

    /*!< Send write enable instruction */
    __sf_WriteEnable();                             

    SF_SPI_CS_ASSERT();
	
    /*!< Send block Erase instruction */
	if(betype == BE32){
    	sf_spiReadWriteByte(SF_CMD_32KBE);        
        _uiBlockStartAddr &= 0xffff8000; // 防止出错,处理成block起始地址
   }
	else{
		sf_spiReadWriteByte(SF_CMD_64KBE);
        _uiBlockStartAddr &= 0xffff0000; // 防止出错,处理成block起始地址
    }
	
    /*!< Send block Erase instruction */
    sf_spiReadWriteByte((_uiBlockStartAddr & 0xFF0000) >> 16);
    sf_spiReadWriteByte((_uiBlockStartAddr & 0xFF00) >> 8); 
    sf_spiReadWriteByte(_uiBlockStartAddr & 0xFF);          
    SF_SPI_CS_DEASSERT();                               

    /*!< Wait the end of Flash writing */
    __sf_WaitForWriteEnd();                         
}

void sf_EraseChip(void)
{
    /*!< Send write enable instruction */
    __sf_WriteEnable();             

    SF_SPI_CS_ASSERT();             
    /*!< Bulk Erase */
    sf_spiReadWriteByte(SF_CMD_CE); 
    SF_SPI_CS_DEASSERT();           
    /*!< Wait the end of Flash writing */
    __sf_WaitForWriteEnd();         
}
void sf_ReadMulBytes(uint32_t _uiReadAddr,uint8_t * _pBuf,uint32_t _uiSize)
{
    //如果读取的数据长度为0 
    if(_uiSize == 0)
        return;

    SF_SPI_CS_ASSERT();                                     
    /*!< Send "Read from Memory " instruction */
    sf_spiReadWriteByte(SF_CMD_READ);                           
    /*!< Send ReadAddr high nibble address byte to read from */
    sf_spiReadWriteByte((_uiReadAddr & 0xFF0000) >> 16);    
    sf_spiReadWriteByte((_uiReadAddr & 0xFF00) >> 8);       
    sf_spiReadWriteByte(_uiReadAddr & 0xFF);                
    while (_uiSize--)
    {
        *_pBuf = sf_spiReadWriteByte(SF_DUMMY_BYTE);
        _pBuf++;
    }
    SF_SPI_CS_DEASSERT();  
}


void sf_WriteMulBytes(uint32_t _uiWriteAddr,uint8_t * _pBuf,uint32_t NumByteToWrite)
{
    //如果读取的数据长度为0 
    if(NumByteToWrite == 0)
        return;
    
    /* for MX25L1606E 、 W25Q64BV */
    __sf_WriteEnable();                             

    SF_SPI_CS_ASSERT();                             
    /*!< Send "Write to Memory " instruction */
    sf_spiReadWriteByte(SF_CMD_WRITE);  
    /*!< Send WriteAddr high nibble address byte to write to */
    sf_spiReadWriteByte((_uiWriteAddr & 0xFF0000) >> 16);
    sf_spiReadWriteByte((_uiWriteAddr & 0xFF00) >> 8);     
    sf_spiReadWriteByte(_uiWriteAddr & 0xFF);              

    while(NumByteToWrite--)
    {
        sf_spiReadWriteByte(*_pBuf);
        _pBuf++;
    }

    SF_SPI_CS_DEASSERT();                               

    __sf_WaitForWriteEnd();
}

uint8_t sf_Read(uint32_t _uiReadAddr, uint8_t * _pBuf,uint32_t _uiSize)
{
    //如果读取的数据长度为0 
    if(_uiSize == 0)
        return Success;
    
    // 超出芯片地址空间，则直接返回
    // 大小超出芯片容量
    // 尾地址超出地址空间
    if (( _uiReadAddr >= g_sf_info.TotalSize) 
        || (_uiSize > g_sf_info.TotalSize) || 
        ((_uiReadAddr + _uiSize) >= g_sf_info.TotalSize))
        return Failed;

    sf_ReadMulBytes(_uiReadAddr, _pBuf, _uiSize);
    
    return Success;
}
uint8_t sf_Write(uint32_t _uiWriteAddr, uint8_t * _pBuf, uint32_t _usWriteSize)
{
    uint32_t addr;
    uint32_t rem; 
    uint32_t NumOfPage;
    
    //如果数据长度为0 
    if(_usWriteSize == 0)
        return Success;
    
    // 超出芯片地址空间，则直接返回
    // 大小超出芯片容量
    // 尾地址超出地址空间
    if (( _uiWriteAddr >= g_sf_info.TotalSize) 
        || (_usWriteSize > g_sf_info.TotalSize) || 
        ((_uiWriteAddr + _usWriteSize) >= g_sf_info.TotalSize))
        return Failed;

    addr = _uiWriteAddr & (g_sf_info.PageSize - 1);// 计算是否扇区为起始地址 
    // 判断是否是页起始地址,
    if(addr > 0){
        rem = g_sf_info.PageSize - addr; // 计算页内容纳数据个数
        rem = MIN(_usWriteSize, rem); // 得最小值,有可能要写的数据在一个页内
        sf_WriteMulBytes(_uiWriteAddr, _pBuf, rem );
        _uiWriteAddr += rem;
        _pBuf += rem;
        _usWriteSize -= rem;
    }

    // 是否还有没写完,并且有整页, 写页
    NumOfPage = _usWriteSize / g_sf_info.PageSize; // 计算页数
    while(NumOfPage--)
    {
        sf_WriteMulBytes(_uiWriteAddr, _pBuf, g_sf_info.PageSize);
        _uiWriteAddr += g_sf_info.PageSize;
        _pBuf += g_sf_info.PageSize;
        _usWriteSize -= g_sf_info.PageSize;
    }

    // 还有没写完, 写余下页内
    if(_usWriteSize > 0){
        sf_WriteMulBytes(addr, _pBuf, _usWriteSize );
    }
}

/*
*********************************************************************************************************
*   函 数 名: sf_NeedErase
*   功能说明: 判断写PAGE前是否需要先擦除。
*   形    参:   _ucpOldBuf ： 旧数据
*              _ucpNewBuf ： 新数据
*              _uiLen ：数据个数，不能超过页面大小
*   返 回 值: FALSE : 不需要擦除， TRUE ：需要擦除
*********************************************************************************************************
*/
uint8_t sf_NeedErase(uint8_t * _ucpOldBuf, uint8_t *_ucpNewBuf, uint32_t _usLen)
{
    uint8_t ucOld;

    /*
    算法第1步：old 求反, new 不变
          old    new
          1101   0101
    ~     1111
        = 0010   0101

    算法第2步: old 求反的结果与 new 位与
          0010   old
    &     0101   new
         =0000

    算法第3步: 结果为0,则表示无需擦除. 否则表示需要擦除
    */

    while(_usLen--)
    {
        ucOld = *_ucpOldBuf++;
        ucOld = ~ucOld;

        /* 注意错误的写法: if (ucOld & (*_ucpNewBuf++) != 0) 符号优先级问题*/
        if ((ucOld & (*_ucpNewBuf++)) != 0) {
            return TRUE;
        }
    }
    return FALSE;
}


/*
*********************************************************************************************************
*   函 数 名: sf_CmpData
*   功能说明: 比较Flash的数据.
*   形    参:   _ucpTar : 数据缓冲区
*               _uiSrcAddr ：Flash地址
*               _uiSize ：数据个数, 可以大于PAGE_SIZE,但是不能超出芯片总容量
*   返 回 值: 0 = 相等, 1 = 不等
*********************************************************************************************************
*/
uint8_t sf_CmpData(uint32_t _uiSrcAddr, uint8_t *_ucpTar, uint32_t _uiSize)
{
    uint8_t ucValue;

    if (_uiSize == 0)
        return 0;

    SF_SPI_CS_ASSERT();                                     /* 使能片选 */
    sf_spiReadWriteByte(SF_CMD_READ);                       /* 发送读命令 */
    sf_spiReadWriteByte((_uiSrcAddr & 0xFF0000) >> 16);     /* 发送扇区地址的高8bit */
    sf_spiReadWriteByte((_uiSrcAddr & 0xFF00) >> 8);        /* 发送扇区地址中间8bit */
    sf_spiReadWriteByte(_uiSrcAddr & 0xFF);                 /* 发送扇区地址低8bit */
    while (_uiSize--)
    {
        /* 读一个字节 */
        ucValue = sf_spiReadWriteByte(SF_DUMMY_BYTE);
        if (*_ucpTar++ != ucValue){
            SF_SPI_CS_DEASSERT();
            
            return 1;
        }
    }
    SF_SPI_CS_DEASSERT();
    
    return 0;
}

#if SF_FLASH_WRITE_ADVANCED_MODE == 1

/* 用于写函数，先读出整个sector，修改缓冲区后，再整个sector回写 */
static uint8_t g_sfBuf[SF_SECTOR_SIZE]; 

static void __sf_WriteMulWholePage(uint32_t _PageStartAddr,uint8_t * _pBuf,uint32_t NumByteToWrite)
{
    NumByteToWrite = NumByteToWrite / g_sf_info.PageSize; // 共几页
    while(NumByteToWrite--)
    {
        sf_WriteMulBytes(_PageStartAddr, _pBuf, g_sf_info.PageSize);                       
        _PageStartAddr += g_sf_info.PageSize;
        _pBuf += g_sf_info.PageSize;
    }
}
/*
*********************************************************************************************************
*   函 数 名: __sf_AutoWriteSector
*   功能说明: 写1个Sector并校验,如果不正确则再重写两次。本函数自动完成擦除操作。
*   形    参:   _pBuf : 数据源缓冲区；
*               _uiWriteAddr ：目标区域首地址
*               _usSize ：数据个数，不能超过扇区面大小
*   返 回 值: Failed : 错误， Success ： 成功
*********************************************************************************************************
*/
static uint8_t __sf_AutoWriteSector( uint32_t _uiWrAddr, uint8_t *_ucpSrc, uint16_t _usWrLen)
{
    uint16_t i;
    uint16_t j;                     /* 用于延时 */
    uint32_t uiFirstAddr;           /* 扇区首址 */
    uint8_t ucNeedErase;    /* TRUE表示需要擦除 */
    uint8_t cRet;

    /* 长度为0时不继续操作,直接认为成功 */
    if (_usWrLen == 0)
        return Success;

    // 如果偏移地址超过芯片地址则退出
    // 如果数据长度大于扇区容量，则退出
    // 尾地址超出芯片最大地址
    if ((_uiWrAddr >= g_sf_info.TotalSize ) 
        || (_usWrLen > g_sf_info.SectorSize) 
        || (_uiWrAddr + _usWrLen) >= g_sf_info.TotalSize )
        return Failed;

    /* 如果FLASH中的数据没有变化,则不写FLASH */
    sf_Read(_uiWrAddr,g_sfBuf,_usWrLen);
    if (memcmp(g_sfBuf, _ucpSrc, _usWrLen) == 0){
        return Success;
    }

    /* 判断是否需要先擦除扇区 */
    /* 如果旧数据修改为新数据，所有位均是 1->0 或者 0->0, 则无需擦除,提高Flash寿命 */
    ucNeedErase = FALSE;
    if (sf_NeedErase(g_sfBuf, _ucpSrc, _usWrLen)){
        ucNeedErase = TRUE;
    }

    uiFirstAddr = _uiWrAddr & (~(g_sf_info.SectorSize - 1));

    if (_usWrLen == g_sf_info.SectorSize){      /* 整个扇区都改写 */
        for (i = 0; i < g_sf_info.SectorSize; i++){
            g_sfBuf[i] = _ucpSrc[i];
        }
    }
    else{ /* 改写部分数据 */
        /* 先将整个扇区的数据读出 */
        sf_Read(uiFirstAddr,g_sfBuf,g_sf_info.SectorSize);

        /* 再用新数据覆盖 */
        i = _uiWrAddr & (g_sf_info.SectorSize - 1);
        memcpy(&g_sfBuf[i], _ucpSrc, _usWrLen);
    }

    /* 写完之后进行校验，如果不正确则重写，最多3次 */
    cRet = Failed;
    for (i = 0; i < 3; i++){
        /* 如果旧数据修改为新数据，所有位均是 1->0 或者 0->0, 则无需擦除,提高Flash寿命 */
        if (ucNeedErase == TRUE){
            sf_EraseSector(uiFirstAddr);        /* 擦除1个扇区 */
        }

        /* 编程一个Sector */
        __sf_WriteMulWholePage(uiFirstAddr, g_sfBuf, g_sf_info.SectorSize);

        if (sf_CmpData(_uiWrAddr, _ucpSrc, _usWrLen) == 0){
            cRet = Success;
            break;
        }
        else{
            if (sf_CmpData(_uiWrAddr, _ucpSrc, _usWrLen) == 0){
                cRet = Success;
                break;
            }

            /* 失败后延迟一段时间再重试 */
            for (j = 0; j < 500; j++);
        }
    }

    return cRet;
}


/*
*********************************************************************************************************
*   函 数 名: sf_Write
*   功能说明: 写1个扇区并校验,如果不正确则再重写两次。本函数自动完成擦除操作。
*   形    参:   _pBuf : 数据源缓冲区；
*               _uiWrAddr ：目标区域首地址
*               _usSize ：数据个数，不能超过页面大小
*   返 回 值: Success : 成功， Failed ： 失败
*********************************************************************************************************
*/
uint8_t sf_AutoWrite(uint32_t _uiWriteAddr, uint8_t* _pBuf,  uint32_t _usWriteSize)
{
    uint32_t NumOfSector, Addr, rem;

    // 超出芯片地址空间，则直接返回
    // 大小超出芯片容量
    // 尾地址超出地址空间
    if (( _uiWriteAddr >= g_sf_info.TotalSize) 
        || (_uiWriteAddr > g_sf_info.TotalSize) || 
        ((_uiWriteAddr + _usWriteSize) >= g_sf_info.TotalSize))
        return Failed;
    
    //如果写的数据长度为0 
    if(_usWriteSize == 0)
        return Success;


    Addr = _uiWriteAddr & (g_sf_info.SectorSize - 1); // 计算是否扇区为起始地址 
    if(Addr > 0) {
        rem = g_sf_info.SectorSize - Addr; // 计算扇区容纳数据个数
        rem = MIN(_usWriteSize, rem); // 得最小值,有可能要写的数据在一个扇区内
        if (__sf_AutoWriteSector( _uiWriteAddr, _pBuf, rem) != Success){
            return Failed;
        }
        _uiWriteAddr +=  rem;
        _pBuf += rem;
        _usWriteSize -= rem;
    }

    NumOfSector = _usWriteSize / g_sf_info.SectorSize; // 计算扇区数
    while (NumOfSector--)
    {
        if (__sf_AutoWriteSector( _uiWriteAddr, _pBuf, g_sf_info.SectorSize) != Success){
            return Failed;
        }
        _uiWriteAddr +=  g_sf_info.SectorSize;
        _pBuf += g_sf_info.SectorSize;
        _usWriteSize -= g_sf_info.SectorSize;
    }
    
    // 是否还有没写完的
    if (_usWriteSize > 0){
        if (__sf_AutoWriteSector( _uiWriteAddr, _pBuf, _usWriteSize) != Success){
            return Failed;
        }
    }
    
    return Success;   /* 成功 */
}
#endif

void sf_InitFlash(void)
{
    __sf_ReadInfo();                /* 自动识别芯片型号 */

    SF_SPI_CS_ASSERT();             /* 软件方式，使能串行Flash片选 */
    sf_spiReadWriteByte(SF_CMD_DISWR);      /* 发送禁止写入的命令,即使能软件写保护 */
    SF_SPI_CS_DEASSERT();               /* 软件方式，禁能串行Flash片选 */

    __sf_WaitForWriteEnd();     /* 等待串行Flash内部操作完成 */
    __sf_WriteStatus(0);            /* 解除所有BLOCK的写保护 */
}

#if 0
void sf_StartEraseSectoreSequeue(uint32_t _uiSectorAddr)
{
    /*!< Send write enable instruction */
    __sf_WriteEnable();                             

    SF_SPI_CS_ASSERT();                                 
    /*!< Send Sector Erase instruction */
    sf_spiReadWriteByte(SF_CMD_SE);
    /*!< Send Sector Erase instruction */
    sf_spiReadWriteByte((_uiSectorAddr & 0xFF0000) >> 16);
    sf_spiReadWriteByte((_uiSectorAddr & 0xFF00) >> 8); 
    sf_spiReadWriteByte(_uiSectorAddr & 0xFF);          
    SF_SPI_CS_DEASSERT();                                   
}

void sf_StartWritePageSequeue(uint32_t _PageAddr,uint8_t * _pBuf,uint16_t NumByteToWrite)
{
    /* for MX25L1606E 、 W25Q64BV */
    __sf_WriteEnable();                             

    SF_SPI_CS_ASSERT();                             
    /*!< Send "Write to Memory " instruction */
    sf_spiReadWriteByte(SF_CMD_WRITE);  
    /*!< Send WriteAddr high nibble address byte to write to */
    sf_spiReadWriteByte((_PageAddr & 0xFF0000) >> 16);
    sf_spiReadWriteByte((_PageAddr & 0xFF00) >> 8);     
    sf_spiReadWriteByte(_PageAddr & 0xFF);              

    while(NumByteToWrite--)
    {
        sf_spiReadWriteByte(*_pBuf);
        _pBuf++;
    }

    SF_SPI_CS_DEASSERT();                               
}
#endif
