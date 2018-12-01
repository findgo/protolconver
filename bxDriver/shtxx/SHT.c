#include "SHT.h"
#include "timers.h"

// 定义使能调试信息
// 模块调试
#if  ( 1 ) && defined(GLOBAL_DEBUG)
#define BspSHTlog(format,args...) printf(format,##args) 
#else
#define BspSHTlog(format,args...)
#endif

static uint8_t SHT_Init_ready = FALSE;
static TimerHandle_t shttimer;
static TimerStatic_t shttimerstaticbuf;
static void shttimerCB(void *arg);
static uint8_t flag = FALSE;

uint8_t bsp_InitSHT(void)
{
	BspSHTlog("SHT....\r\n");
    
    shttimer = timerAssign(&shttimerstaticbuf, shttimerCB, NULL);
    SHT2x_Init(); // 初始化iic
	if(SHT2x_CheckDevice() == Failed)
	{
		BspSHTlog("SHT init hardware failed\r\n");
        SHT_Init_ready = FALSE;
        timerStart(shttimer, 100); // 启动,便于后续初始化
        return Failed;
    }

    SHT2x_SetFeature(SHT2x_Resolution_12_14, FALSE); // 初始化精度 和 加热
    SHT_Init_ready = TRUE;
    SHT2x_MeasureStart(TRIG_TEMP_MEASUREMENT_POLL);
    timerStart(shttimer, SHT2x_GetInfo()->measure_temp_time);
    BspSHTlog("SHT init successful\r\n");
    
	return Success ;
}

// success : on line ,failed : not on line
static uint8_t SHT_Check_Online(void)
{
	return SHT2x_CheckDevice();
}

static void shttimerCB(void *arg)
{
    SHT2x_info_t *psht; 

	if(SHT_Init_ready == FALSE){ 	
	    if(bsp_InitSHT() != Success)
            return;
	}
    
    //check dev online
    if(SHT_Check_Online() != Success){
        SHT_Init_ready = FALSE;
        timerStart(shttimer, 100); // 启动,便于后续初始化
        return;
    }

    SHT2x_MeasureDeal();

    psht = SHT2x_GetInfo();
    BspSHTlog("\t poll(T = %.3f H = %.3f)\r\n", psht->TEMP,psht->HUMI);

    if(flag){
        SHT2x_MeasureStart(TRIG_TEMP_MEASUREMENT_POLL);
        timerStart(shttimer, SHT2x_GetInfo()->measure_temp_time);
    }
    else{
        SHT2x_MeasureStart(TRIG_HUMI_MEASUREMENT_POLL);
        timerStart(shttimer, SHT2x_GetInfo()->measure_humi_time);
    }
    
    flag = !flag;
}

void SHT_PeriodicHandle(void)
{	
    SHT2x_info_t *info;
	if(SHT_Init_ready == FALSE){ 	
	    if(bsp_InitSHT() != Success)
            return;
	}
    
    //check dev online
    if(SHT_Check_Online() != Success){
        SHT_Init_ready = FALSE;
        return;
    }
#if SHT2x_MOD
	SHT2x_Measure(TRIG_TEMP_MEASUREMENT_POLL);//获取SHT20 温度
    delay_ms(200);
	SHT2x_Measure(TRIG_HUMI_MEASUREMENT_POLL);//获取SHT20 湿度
    delay_ms(200);
    info = SHT2x_GetInfo();
    BspSHTlog("\t SHT2x:T = %.2f H = %.2f", info->TEMP, info->HUMI );
#endif

}

