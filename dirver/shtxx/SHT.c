#include "SHT.h"

// 定义使能调试信息
// 模块调试
#if  ( 1 ) && defined(GLOBAL_DEBUG)
#define BspSHTlog(format,args...) printf(format,##args) 
#else
#define BspSHTlog(format,args...)
#endif

static uint8_t SHT_Init_ready = FALSE;

static uint8_t bsp_InitSHT(void)
{
	BspSHTlog("SHT....\r\n");
#if (SHT_MOD == SHT2x_MOD)		
	if(SHT2x_Init() == Failed)
#else
    if(Failed == Failed)
#endif
	{
		BspSHTlog("SHT init hardware failed\r\n");
        return Failed;
    }

    SHT_Init_ready = TRUE;
    BspSHTlog("SHT init successful\r\n");
    
	return Success ;
}

// success : on line ,failed : not on line
static uint8_t SHT_Check_Online(void)
{
#if SHT2x_MOD	
	return _SHT2x_iic_CheckDevice(SHT2x_SLAVE_ADDRESS);
#endif
}


void SHT_PeriodicHandle(void)
{	
	if(SHT_Init_ready == FALSE)
	{ 	
	    if(bsp_InitSHT() != Success)
            return;
        else
            SHT_Init_ready = TRUE;
	}
    
    //check dev online
    if(SHT_Check_Online() != Success){
        SHT_Init_ready = FALSE;
        return;
    }
#if SHT2x_MOD
    sht2x_Measure();
#endif

}


