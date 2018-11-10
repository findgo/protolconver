/** 
 * @file     SHT2x.c
 * @brief    湿温度传感器
 * @details  
 * @author   xzl
 * @email    
 * @date     
 * @version  
 * @par Copyright (c):  
 *           
 * @par History:          
 *   version: author, date, desc\n 
 *             mo 20181109 抽像调试宏,便于裁减和移植,缩减代码尺寸,优化API接口
 */

#include "SHT2x.h"

// 定义使能调试信息
// 模块调试
#if  ( 1 ) && defined(GLOBAL_DEBUG)
#define SHT2Xlog(format,args...) printf(format,##args) 
#else
#define SHT2Xlog(format,args...)
#endif


typedef enum {
    SHT2x_RES_12_14BIT         = 0x00, //RH=12bit, T=14bit
    SHT2x_RES_8_12BIT          = 0x01, //RH= 8bit, T=12bit
    SHT2x_RES_10_13BIT         = 0x80, //RH=10bit, T=13bit
    SHT2x_RES_11_11BIT         = 0x81, //RH=11bit, T=11bit
    SHT2x_RES_MASK             = 0x81  //Mask for res. bits (7,0) in user reg.
} SHT2xResolution;

typedef enum {
    SHT2x_EOB_ON               = 0x40, //end of battery
    SHT2x_EOB_MASK             = 0x40  //Mask for EOB bit(6) in user reg.
} SHT2xEob;

typedef enum {
    SHT2x_HEATER_ON            = 0x04, //heater on
    SHT2x_HEATER_OFF           = 0x00, //heater off
    SHT2x_HEATER_MASK          = 0x04  //Mask for Heater bit(2) in user reg.
} SHT2xHeater;


#define SHT2x_ReadUserReg()        _SHT2x_iicDevReadByte( SHT2x_SLAVE_ADDRESS , USER_REG_R )
#define SHT2x_WriteUserReg(reg)    _SHT2x_iicDevWriteByte( SHT2x_SLAVE_ADDRESS , USER_REG_W, reg )

static SHT2x_PARAM_t g_sht2x_param;

//正常
uint8_t SHT2x_Init(void)
{
	_SHT2x_iic_init();
	if( _SHT2x_iic_CheckDevice(SHT2x_SLAVE_ADDRESS) == Failed ){	
    	SHT2x_SoftReset();
    	delay_ms(15);
    
        return Failed;
    }
    
    SHT2x_SetFeature(SHT2x_Resolution_11_11, FALSE);
    
	return Success;
}

static uint8_t SHT2x_calCrc(uint8_t data[],uint8_t nbrOfBytes)
{
#define POLYNOMIAL 0x131 //P(x) = x^8+ x^5 + x^4 + x^0
	uint8_t crc = 0;
	uint8_t byteCtr;
    
	for(byteCtr = 0;byteCtr <nbrOfBytes; byteCtr++ )
	{
		crc ^=(data[byteCtr]);
		for(uint8_t bit = 0;bit<8;bit++)
		{
			if(crc & 0x80) 
                crc = (crc << 1) ^ 0x31;
			else 
                crc = ( crc << 1);      
		}
	}
    
	return crc;
}

float SHT2x_MeasureTemp(uint8_t cmd)
{	
	float TEMP;
	u8  tmp[3];
	u16 ST;
	
	_SHT2x_iicDevReadMultiDelay(SHT2x_SLAVE_ADDRESS,cmd,sizeof(tmp),tmp,11);
    
	ST = (tmp[0] << 8) | (tmp[1] << 0);
	ST &= ~0x0003;

#if 1
    TEMP = -46.85 + 175.72/65536 *(float)ST;
#else
    TEMP = ((float)ST * 0.00268127) - 46.85;
#endif

	return (TEMP);	
}

float SHT2x_MeasureHumi(uint8_t cmd)
{
	float HUMI;
	u8  tmp[3];
	u16 SRH;	
	uint8_t crc;
    
    _SHT2x_iicDevReadMultiDelay(SHT2x_SLAVE_ADDRESS,cmd,sizeof(tmp),tmp,15);

	if( (crc = SHT2x_calCrc(tmp,2)) == tmp[2]){
	    SHT2Xlog("crc check success!\r\n");
    }
    else{
	    SHT2Xlog("crc check failed!receive: %x,cal:%x\r\n", tmp[2],crc);
    }
	
	SRH = (tmp[0] << 8) | (tmp[1] << 0);
	SRH &= ~0x0003;

#if 1
    HUMI = -6.0 + 125.0/65536 * (float)SRH;
#else
    HUMI = ((float)SRH * 0.00190735) - 6;
#endif

	return (HUMI);
}

//正常
void SHT2x_GetSerialNumber(u8 *buf)
{
	u8 temp[8], i = 0;
    
	_SHT2x_iicDevReadReg( SHT2x_SLAVE_ADDRESS ,0xFA0F,temp,4);	
	_SHT2x_iicDevReadReg( SHT2x_SLAVE_ADDRESS ,0xFCC9,temp+4,4);
	
	buf[5] = temp[i++];
	buf[4] = temp[i++];
	buf[3] = temp[i++];
	buf[2] = temp[i++];
	
	buf[1] = temp[i++];
	buf[0] = temp[i++];
	buf[7] = temp[i++];
	buf[6] = temp[i];
}

void SHT2x_SetFeature(uint8_t Resolution, uint8_t isHeatterOn)
{
    uint8_t tmp,reg;
    
    if(Resolution == SHT2x_Resolution_11_11) {//RH=11bit, T=11bit
        tmp = SHT2x_RES_11_11BIT;
    }
    else if (Resolution == SHT2x_Resolution_8_12){      //RH= 8bit, T=12bit
        tmp = SHT2x_RES_8_12BIT;
    }
    else if (Resolution == SHT2x_Resolution_10_13){     //RH=10bit, T=13bit
        tmp = SHT2x_RES_10_13BIT;
    }
    else { //RH=12bit, T=14bit 其它采用默认
        tmp = SHT2x_RES_12_14BIT;
    }
    
	reg = SHT2x_ReadUserReg();
	//set the RES_MASK status	
	reg = (reg & (~SHT2x_RES_MASK) ) | tmp;
	//set the RES_MASK status	
	reg = reg & (~SHT2x_HEATER_MASK);
    if(isHeatterOn)
        reg |= SHT2x_HEATER_ON;

	SHT2x_WriteUserReg(reg);
}


uint8_t SHT2x_CheckFeature(uint8_t Resolution, uint8_t isHeatterOn)
{
    uint8_t resol,reg,temp;

    if(Resolution == SHT2x_Resolution_11_11) {//RH=11bit, T=11bit
        resol = SHT2x_RES_11_11BIT;
    }
    else if (Resolution == SHT2x_Resolution_8_12){      //RH= 8bit, T=12bit
        resol = SHT2x_RES_8_12BIT;
    }
    else if (Resolution == SHT2x_Resolution_10_13){     //RH=10bit, T=13bit
        resol = SHT2x_RES_10_13BIT;
    }
    else { //RH=12bit, T=14bit 其它采用默认
        resol = SHT2x_RES_12_14BIT;
    }

    reg = SHT2x_ReadUserReg();
    
    SHT2Xlog("Get user reg: 0x%x\r\n", reg);
    
	//check the RES_MASK status
	temp = reg & SHT2x_RES_MASK;
	if( temp != resol ) // is you want?
	{
		SHT2Xlog("SHT2x mode is incorrect\r\n");
		SHT2x_SetFeature(Resolution, isHeatterOn);
	}
	else
	{
		return Success;
	}

	if( temp == SHT2x_RES_12_14BIT )
	{
		SHT2Xlog("SHT2x_RES_12_14BIT\r\n");
	}
	else 	if( temp == SHT2x_RES_8_12BIT )
	{
		SHT2Xlog("SHT2x_RES_8_12BIT\r\n");	
	}
	else if( temp == SHT2x_RES_10_13BIT )
	{
		SHT2Xlog("SHT2x_RES_10_13BIT\r\n");	
	}
	else 	if( temp == SHT2x_RES_11_11BIT )
	{
		SHT2Xlog("SHT2x_RES_11_11BIT\r\n");	
	}
	
	//check the battery status
	if(( reg & SHT2x_EOB_MASK ) == SHT2x_EOB_ON )
	{
		SHT2Xlog("Battery <2.25V\r\n");
	}else
	{
		SHT2Xlog("Battery >2.25V\r\n");
	}
	
	//check the HEATER status
	if(( reg & SHT2x_HEATER_MASK ) == SHT2x_HEATER_ON )
	{
		SHT2Xlog("SHT2x_HEATER_ON\r\n");
	}else
	{
		SHT2Xlog("SHT2x_HEATER_OFF\r\n");
	}
    
	return Failed;
}
SHT2x_PARAM_t *sht2x_Measure(void)
{
	memset((char*)&g_sht2x_param,0,sizeof(SHT2x_PARAM_t));
	if(SHT2x_CheckFeature(SHT2x_Resolution_11_11, FALSE) != Success)
	{
		return &g_sht2x_param;
	}
	
	g_sht2x_param.TEMP_HM = SHT2x_MeasureTemp(TRIG_TEMP_MEASUREMENT_HM);//获取SHT20 温度
	g_sht2x_param.HUMI_HM = SHT2x_MeasureHumi(TRIG_HUMI_MEASUREMENT_HM);//获取SHT20 湿度
	
	g_sht2x_param.TEMP_POLL = SHT2x_MeasureTemp(TRIG_TEMP_MEASUREMENT_POLL);//获取SHT20 温度
	g_sht2x_param.HUMI_POLL = SHT2x_MeasureHumi(TRIG_HUMI_MEASUREMENT_POLL);//获取SHT20 湿度
			
    //SHT2Xlog(g_sht2x_param.SerialNumber,sizeof(g_sht2x_param.SerialNumber));
	//SHT2Xlog("reg  %d UserReg = %d\r\n",reg,);
    SHT2Xlog("\t T = %.1f,H = %.1f,poll(T = %.1f H = %.1f)\r\n",
        g_sht2x_param.TEMP_HM,g_sht2x_param.HUMI_HM,
        g_sht2x_param.TEMP_POLL,g_sht2x_param.HUMI_POLL
        );

	return &g_sht2x_param;	
}
