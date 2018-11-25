
#include "stm32f1xx.h"
#include "bsp_uniqueID.h"


static mcu_info_t mcuinfo;

mcu_info_t *bsp_GetChipInfo(void)
{
    mcuinfo.id0  =   *(__IO uint32_t*)(0x1FFFF7E8);
    mcuinfo.id1  =   *(__IO uint32_t*)(0x1FFFF7EC);
    mcuinfo.id2  =   *(__IO uint32_t*)(0x1FFFF7F0);    
    mcuinfo.inflashsize =   *(__IO uint16_t*)(0x1FFFF7E0); 

    return &mcuinfo;  
}



