/**
  ******************************************************************************
  * @file   Mserial.h
  * @author  
  * @version 
  * @date    
  * @brief      FIFO非阻塞串口通信，用于数据流传输
  ******************************************************************************
  * @attention      V1.2 jgb        201704023
  * @attention      V1.2.1 jgb        201808003  修改bug
  ******************************************************************************
  */

#ifndef __MOD_SERIAL_H_
#define __MOD_SERIAL_H_

#include "app_cfg.h"
#include <string.h>

#include "bxusart.h"


// 宏 控制支持的串口数量
#define COM_USE_NUM     3





//宏 用于控制非阻塞方式下，设置发送，接收最大缓冲区
#define COM0_RX_MAX_SIZE   512
#define COM0_TX_MAX_SIZE   255

#define COM1_RX_MAX_SIZE   128
#define COM1_TX_MAX_SIZE   128

#define COM2_RX_MAX_SIZE   1
#define COM2_TX_MAX_SIZE   255

#define COM3_RX_MAX_SIZE   255
#define COM3_TX_MAX_SIZE   255

#define COM0PutByte(dat)    LL_USART_TransmitData8(USART_USING1, dat)
#define COM0GetByte()       LL_USART_ReceiveData8(USART_USING1)
#define COM0TxIEEnable()    LL_USART_EnableIT_TXE(USART_USING1) 
#define COM0TxIEDisable()   LL_USART_DisableIT_TXE(USART_USING1)

#define COM1PutByte(dat)    LL_USART_TransmitData8(USART_USING2, dat)
#define COM1GetByte()     	LL_USART_ReceiveData8(USART_USING2)
#define COM1TxIEEnable()    LL_USART_EnableIT_TXE(USART_USING2)
#define COM1TxIEDisable()   LL_USART_DisableIT_TXE(USART_USING2)

#define COM2PutByte(dat)    LL_USART_TransmitData8(USART_USING3, dat)	
#define COM2GetByte()       LL_USART_ReceiveData8(USART_USING3)	
#define COM2TxIEEnable()    LL_USART_EnableIT_TXE(USART_USING3)	
#define COM2TxIEDisable()   LL_USART_DisableIT_TXE(USART_USING3)

#define COM3PutByte(dat)  	
#define COM3GetByte()     	
#define COM3TxIEEnable()    	
#define COM3TxIEDisable()   	

uint16_t Serial_WriteByte(uint8_t COM,uint8_t dat);
uint16_t Serial_WriteBuf(uint8_t COM,uint8_t *buf,uint16_t len);
uint16_t Serial_Read(uint8_t COM,uint8_t *buf,uint16_t len);
uint16_t SerialRxValidAvail(uint8_t COM);
#define Serial_WriteStr(COM,str) Serial_WriteBuf(COM,(uint8_t *)str,strlen(str))


#endif

