#ifndef _SHT1X_H_
#define _SHT1X_H_

#include "globals.h"

#include <math.h>

#define SHT1x_IIC_SCL_Pin  						GPIO_Pin_6
#define SHT1x_IIC_SDA_Pin  						GPIO_Pin_7
#define SHT1x_GPIO_IIC     						GPIOB
#define SHT1x_RCC_APB2Periph_GPIOIIC 	RCC_APB2Periph_GPIOB

//#define SHT1x_DATA_OUT()  {GPIOB->CRL&=0XFF0FFFFF;GPIOB->CRL|=0x00300000;}//输入模式上拉下拉输入模式
//#define SHT1x_DATA_IN()   {GPIOB->CRL&=0XFF0FFFFF;GPIOB->CRL|=0x00800000;}//通用推挽输出50MZ

#define SHT1x_SDA_PIN_OUT      	PBout(7)
#define SHT1x_SCL_PIN_OUT       PBout(6)
#define SHT1x_SDA_PIN_IN        PBin(7)


#define _nop_() delay_us(1) 
#define bool        _Bool
#define false        0
#define true        1

typedef enum {
    TEMP,  
    HUMI 
} SHT2xMeasureType;


#define C1 (-4.0)              // for 12 Bit
#define C2 (+0.0405)           // for 12 Bit
#define C3 (-0.0000028)        // for 12 Bit
#define T1 (+0.01)             // for 14 Bit @ 5V
#define T2 (+0.00008)           // for 14 Bit @ 5V

typedef union 
{ uint16_t i;
  float f;
} value;

//----------------------------------------------------------------------------------
// modul-var
//----------------------------------------------------------------------------------
enum {SHT1x_TEMP,SHT1x_HUMI};

#define SHT1X_noACK 0
#define SHT1X_ACK   1
                            //adr  command  r/w
#define SHT1X_STATUS_REG_W 0x06   //000   0011    0
#define SHT1X_STATUS_REG_R 0x07   //000   0011    1
#define SHT1X_MEASURE_TEMP 0x03   //000   0001    1
#define SHT1X_MEASURE_HUMI 0x05   //000   0010    1
#define SHT1X_RESET        0x1e   //000   1111    0

bool SHT1x_Periodic_Handle(void);
void SHT1x_Init(void);

#endif

