#ifndef __SHT2x_H__
#define __SHT2x_H__
 
 
#ifdef __cplusplus
  extern "C" {
#endif
 
#define SHT2x_PIN_INDEPENDENT 1

 //fun location
#if SHT2x_PIN_INDEPENDENT
#include "./SHT/IIC/xiic.h"
#define _SHT2x_iic_init 						xIIC_init
#define _SHT2x_iic_CheckDevice 					xIICheckDevice
#define _SHT2x_iicDevWriteByte 					xIICWriteByte
#define _SHT2x_iicDevReadByte 					xIICReadByte
#define _SHT2x_iicDevReadMultiDelay 		    SHT_iicDevReadMultiDelay
#define _SHT2x_iicDevWriteMulti 				SHT_iicDevWriteMulti
#define _SHT2x_iicDevReadReg 					SHT_iicDevReadMultiReg
#define _SHT2x_iicDevWriteCmd 					xIICWriteReg
 
#else
#include "bsp_iic.h"
#define _SHT2x_iic_init 						iic_init
#define _SHT2x_iic_CheckDevice 					iic_CheckDevice
#define _SHT2x_iicDevWriteByte 					iicDevWriteByte
#define _SHT2x_iicDevReadByte 					iicDevReadByte
#define _SHT2x_iicDevReadMultiDelay 		    iicDevReadMultiDelay
#define _SHT2x_iicDevWriteMulti 				iicDevWriteMulti
#define _SHT2x_iicDevReadReg 					iicDevReadMultiReg
#define _SHT2x_iicDevWriteCmd 					iicDevWriteCmd
#endif
 
#define SHT2x_SLAVE_ADDRESS    0x80		/* I2C从机地址 */
 
 
 typedef enum {
     TRIG_TEMP_MEASUREMENT_HM   = 0xE3, //command trig. temp meas. hold master
     TRIG_HUMI_MEASUREMENT_HM   = 0xE5, //command trig. humidity meas. hold master
     TRIG_TEMP_MEASUREMENT_POLL = 0xF3, //command trig. temp meas. no hold master
     TRIG_HUMI_MEASUREMENT_POLL = 0xF5, //command trig. humidity meas. no hold master
     USER_REG_W                 = 0xE6, //command writing user register
     USER_REG_R                 = 0xE7, //command reading user register
     SOFT_RESET                 = 0xFE  //command soft reset
 } SHT2xCommand;

// 用于feature设置
enum {
    SHT2x_Resolution_12_14 = 0, //RH=12bit, T=14bit
    SHT2x_Resolution_8_12,      //RH= 8bit, T=12bit
    SHT2x_Resolution_10_13,     //RH=10bit, T=13bit
    SHT2x_Resolution_11_11,     //RH=11bit, T=11bit
};

typedef struct  {
     float TEMP_HM;
     float HUMI_HM;
     float TEMP_POLL;
     float HUMI_POLL;    
 } SHT2x_PARAM_t;
 

#define SHT2x_SoftReset()   _SHT2x_iicDevWriteCmd(SHT2x_SLAVE_ADDRESS,SOFT_RESET)
 
uint8_t SHT2x_Init(void); 
float SHT2x_MeasureTemp(uint8_t cmd);
float SHT2x_MeasureHumi(uint8_t cmd);
void SHT2x_GetSerialNumber(u8 *buf);
/*Resolution : RH,TMP的精度, 查看上面枚举 , isHeatterOn: TRUE,启动加热*/
void SHT2x_SetFeature(uint8_t Resolution, uint8_t isHeatterOn);
/* 检测功能是否是设置的,一样:Success, Failed */
uint8_t SHT2x_CheckFeature(uint8_t Resolution, uint8_t isHeatterOn);

SHT2x_PARAM_t *sht2x_Measure(void);
 
#ifdef __cplusplus
 }
#endif
 
#endif
