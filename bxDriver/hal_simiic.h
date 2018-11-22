

/* 本模块未测试 */
#ifndef __HAL_SIMIIC_H_
#define __HAL_SIMIIC_H_

#include "app_cfg.h"


//IO	操作脚 
//SCL
//SDA	 
//输入SDA 

#define SOFT_IIC_SDA_PORT	GPIOB
#define SOFT_IIC_SDA_PIN	GPIO_Pin_7
#define SOFT_IIC_SCL_PORT	GPIOB
#define SOFT_IIC_SCL_PIN	GPIO_Pin_6
#define SOFT_IIC_GPIO_PeriphClock_EN()  do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); }while(0)

#define SOFT_IIC_SDA_INPUT()	{GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=0x80000000;}
#define SOFT_IIC_SDA_OUTPUT()	{GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=0x30000000;}

#define SOFT_IIC_SDA_HIGH()		PBout(7) = 1
#define SOFT_IIC_SDA_LOW()		PBout(7) = 0
#define SOFT_IIC_SCL_HIGH()		PBout(6) = 1
#define SOFT_IIC_SCL_LOW()		PBout(6) = 0
#define SOFT_IIC_SDA_READ		PBin(7)
#define SOFT_IIC_DELAY_WIDE()	delay_us(1)


// 用于读内部寄存器? 表明操作的是个命令
#define IICDEV_NO_MEM_ADDR	0xff

void IIC_Init(void);
/* iic读写数据,成功返回success, 否则Failed */ 
uint8_t IICreadBuf(uint8_t devAddress, uint8_t memAddress, uint8_t length, uint8_t *buf);
uint8_t IICreadByte(uint8_t devAddress, uint8_t memAddress, uint8_t *pdata);
uint8_t IICwriteBuf(uint8_t devAddress, uint8_t memAddress, uint8_t length, uint8_t *buf);
uint8_t IICwriteByte(uint8_t devAddress, uint8_t memAddress, uint8_t data);

// 写命令寄存器
#define IICWriteReg(devAddress, data) IICwriteByte(devAddress, IICDEV_NO_MEM_ADDR, data)
// 读命令寄存器
#define IICReadReg(devAddress, pdata) IICreadByte(devAddress, IICDEV_NO_MEM_ADDR, pdata)

/* iic读写一个字节的位,成功返回success, 否则Failed */ 
/* 只能操作一个字节内,读多个位    ,从右往左数, 低位开始*/
uint8_t IICreadBits(uint8_t devAddress, uint8_t memAddress, uint8_t bitStart, uint8_t length, uint8_t *data);
/* 只能操作一个字节内,读一个位    */
uint8_t IICreadBit(uint8_t devAddress, uint8_t memAddress, uint8_t bitNum, uint8_t *data);
/* 只能操作一个字节内,写多个位, 正常位序,从右往左数, 低位开始    */
uint8_t IICwriteBits(uint8_t devAddress, uint8_t memAddress, uint8_t bitStart, uint8_t length, uint8_t bitVal);
/* 只能操作一个字节内,写一个位    */
uint8_t IICwriteBit(uint8_t devAddress, uint8_t memAddress, uint8_t bitNum, uint8_t bitVal);


#endif

//------------------End of File----------------------------







