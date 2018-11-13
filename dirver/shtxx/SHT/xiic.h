/* 软件模拟IIC 模块 */
#ifndef __XIIC__H_
#define __XIIC__H_

#include "common_type.h"
#include "delay.h"


#define xIIC_SCL_PIN    GPIO_Pin_0
#define xIIC_SDA_PIN    GPIO_Pin_1
#define xIIC_SCL_PORT   GPIOA
#define xIIC_SDA_PORT   GPIOA
#define xIIC_RCC_PORT   RCC_APB2Periph_GPIOA

#define xIIC_SDA_OUTPUT()       PxCfgOutputS50PP(GPIOA,1)//通用推挽输出50MZ 
#define xIIC_SDA_INPUT()        PxCfgInputIPU(GPIOA, 1) //输入模式上拉下拉输入模式
#define xIIC_SCL_PIN_OUT        PAout(0)
#define xIIC_SCL_PIN_IN         PAin(0)

#define xIIC_SDA_PIN_OUT       PAout(1)
#define xIIC_SDA_PIN_IN        PAin(1)

// 定义scl sda电平保持时间
#define xIIC_DELAY_WIDE()   delay_us(1); //

/* 用于读内部寄存器使用,比如命令寄存器*/
#define xIICDEV_NO_MEM_ADDR     0xff  // 0xff一般是个空指令

void    xIIC_init(void);
/* 设备是否在线,  */
uint8_t xIICheckDevice(uint8_t devddress);

uint8_t xIICWriteByte(u8 devaddr, u8 memAddress, u8 data);
uint8_t xIICWriteMultiBytes(u8 devaddr,u8 memAddress,u8 len,u8 *wbuf);

uint8_t xIICReadByte(u8 devaddr,u8 memAddress);
uint8_t xIICReadMultiBytes(u8 devaddr,u8 memAddress,u8 len,u8 *wbuf);

// 写命令寄存器
#define xIICWriteReg(devaddr, data) xIICWriteByte(devaddr, xIICDEV_NO_MEM_ADDR, data)
// 读命令寄存器
#define xIICReadReg(devaddr) xIICReadByte(devaddr, xIICDEV_NO_MEM_ADDR)

/******************************非标准IIC Only for SHT2x*****************************************************/
/* 非主机模式 读取成功返回success*/ 
uint8_t SHT_DevReadbyPoll(u8 devaddr,u8 addr,u8 len,u8 *rbuf);
uint8_t SHT_DevReadMeasure(u8 devaddr,u8 len,u8 *wbuf);
void SHT_DevReadMultiReg(u8 devaddr,uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen);
void SHT_DevWriteMultiReg(u8 devaddr,uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen);
#endif
