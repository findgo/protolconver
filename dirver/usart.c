/**
  ******************************************************************************
  * @file    usart.c
  * @author  
  * @version 
  * @date    
  * @brief   
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
#include "usart.h"

#if defined(RTE_Drivers_USART1)
extern ARM_DRIVER_USART Driver_USART1;
#endif

#if defined(RTE_Drivers_USART2)
extern ARM_DRIVER_USART Driver_USART2;
#endif

#if defined(RTE_Drivers_USART3)
extern ARM_DRIVER_USART Driver_USART3;
#endif

/**
  * @brief  init the serial
  * @param  port      which port
  *         ulBaudRate  
  *         ucDataBits  
  *         eParity      
  * @retval TRUE: success FALSE: failed
  */
int SerialDrvInit(uint8_t port, uint32_t ulBaudRate, uint8_t ucDataBits, DRV_Parity_t eParity)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;  
	USART_ClockInitTypeDef USART_ClockInitStructure;
    
    (void)ucDataBits; //���޸�����λ����
    (void)eParity;    //���޸�У���ʽ
    
    switch (port){
    case COM0:
    #if !defined(RTE_Drivers_USART1)
        USART_USING1_GPIO_PeriphClock_EN();
        USART_USING1_PeriphClock_EN();
        
        //PB6 ?????USAR1 TX, PB7 ?????USART1 RX
        GPIO_InitStructure.GPIO_Pin   = USART_USING1_TX_PIN;//TX AF mode
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(USART_USING1_TX_PORT, &GPIO_InitStructure);
        
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
        GPIO_InitStructure.GPIO_Pin   = USART_USING1_RX_PIN;//RX AF mode
        GPIO_Init(USART_USING1_RX_PORT, &GPIO_InitStructure);
        
        
        NVIC_InitStructure.NVIC_IRQChannel = USART_USING1_IRQ;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  =  1;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
        
        USART_ClockStructInit(&USART_ClockInitStructure);
        USART_ClockInit(USART_USING1, &USART_ClockInitStructure);
        
        USART_InitStructure.USART_BaudRate = ulBaudRate;
        USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
        USART_InitStructure.USART_HardwareFlowControl = DISABLE;
        USART_InitStructure.USART_Parity = USART_Parity_No;
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;// 8 bit
        USART_Init(USART_USING1, &USART_InitStructure);
        
        USART_ClearFlag(USART_USING1, USART_FLAG_TXE | USART_IT_RXNE | USART_FLAG_TC);
        USART_ITConfig(USART_USING1, USART_IT_TXE,  DISABLE);
        USART_ITConfig(USART_USING1, USART_IT_TC, DISABLE);
        USART_ITConfig(USART_USING1, USART_IT_RXNE, ENABLE);
        
        USART_Cmd(USART_USING1, ENABLE);//enable USART1
    #else
        Driver_USART1.Initialize(NULL);
        Driver_USART1.PowerControl(ARM_POWER_FULL);
        Driver_USART1.Control(ARM_USART_MODE_ASYNCHRONOUS 
            | ARM_USART_DATA_BITS_8 
            | ARM_USART_PARITY_NONE 
            | ARM_USART_STOP_BITS_1
            | ARM_USART_FLOW_CONTROL_NONE , ulBaudRate);

        Driver_USART1.Control(ARM_USART_CONTROL_TX | ARM_USART_CONTROL_RX, 1);
        #endif
        break;
        
    case COM1:
    #if !defined(RTE_Drivers_USART2)
        USART_USING2_GPIO_PeriphClock_EN();
        USART_USING2_PeriphClock_EN();
        
        //PB6 ?????USAR1 TX, PB7 ?????USART1 RX
        GPIO_InitStructure.GPIO_Pin   = USART_USING2_TX_PIN;//TX AF mode
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(USART_USING2_TX_PORT, &GPIO_InitStructure);
        
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
        GPIO_InitStructure.GPIO_Pin   = USART_USING2_RX_PIN;//RX AF mode
        GPIO_Init(USART_USING2_RX_PORT, &GPIO_InitStructure);
        
        
        NVIC_InitStructure.NVIC_IRQChannel = USART_USING2_IRQ;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  =  1;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
        
        USART_ClockStructInit(&USART_ClockInitStructure);
        USART_ClockInit(USART_USING2, &USART_ClockInitStructure);
        
        USART_InitStructure.USART_BaudRate = ulBaudRate;
        USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
        USART_InitStructure.USART_HardwareFlowControl = DISABLE;
        USART_InitStructure.USART_Parity = USART_Parity_No;
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;// 8 bit
        USART_Init(USART_USING2, &USART_InitStructure);
        
        USART_ClearFlag(USART_USING2, USART_FLAG_TXE | USART_IT_RXNE | USART_FLAG_TC);
        USART_ITConfig(USART_USING2, USART_IT_TXE,  DISABLE);
        USART_ITConfig(USART_USING2, USART_IT_TC, DISABLE);
        USART_ITConfig(USART_USING2, USART_IT_RXNE, ENABLE);
        
        USART_Cmd(USART_USING2, ENABLE);//enable USART2
    #else
        Driver_USART2.Initialize(NULL);
        Driver_USART2.PowerControl(ARM_POWER_FULL);
        Driver_USART2.Control(ARM_USART_MODE_ASYNCHRONOUS 
            | ARM_USART_DATA_BITS_8 
            | ARM_USART_PARITY_NONE 
            | ARM_USART_STOP_BITS_1
            | ARM_USART_FLOW_CONTROL_NONE , ulBaudRate);

        Driver_USART2.Control(ARM_USART_CONTROL_TX | ARM_USART_CONTROL_RX, 1);
    #endif
        break;
         
    case COM2:
    #if !defined(RTE_Drivers_USART3)
        USART_USING3_GPIO_PeriphClock_EN();
        USART_USING3_PeriphClock_EN();
        
        //PB10 -- USAR1 TX, PB11 -- USART1 RX
        GPIO_InitStructure.GPIO_Pin   = USART_USING3_TX_PIN;//TX AF mode
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(USART_USING3_TX_PORT, &GPIO_InitStructure);
        
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
        GPIO_InitStructure.GPIO_Pin   = USART_USING3_RX_PIN;//RX AF mode
        GPIO_Init(USART_USING3_RX_PORT, &GPIO_InitStructure);
        
        
        NVIC_InitStructure.NVIC_IRQChannel = USART_USING3_IRQ;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  =  2;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
        
        USART_ClockStructInit(&USART_ClockInitStructure);
        USART_ClockInit(USART_USING3, &USART_ClockInitStructure);
        
        USART_InitStructure.USART_BaudRate = ulBaudRate;
        USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
        USART_InitStructure.USART_HardwareFlowControl = DISABLE;
        USART_InitStructure.USART_Parity = USART_Parity_No;
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;// 8 bit
        USART_Init(USART_USING3, &USART_InitStructure);
        
        USART_ClearFlag(USART_USING3, USART_FLAG_TXE | USART_IT_RXNE | USART_FLAG_TC);
        USART_ITConfig(USART_USING3, USART_IT_TXE,  DISABLE);
        USART_ITConfig(USART_USING3, USART_IT_TC, DISABLE);
        USART_ITConfig(USART_USING3, USART_IT_RXNE, ENABLE);
        
        USART_Cmd(USART_USING3, ENABLE);//enable USART2
    #else
        Driver_USART3.Initialize(NULL);
        Driver_USART3.PowerControl(ARM_POWER_FULL);
        Driver_USART3.Control(ARM_USART_MODE_ASYNCHRONOUS 
            | ARM_USART_DATA_BITS_8 
            | ARM_USART_PARITY_NONE 
            | ARM_USART_STOP_BITS_1
            | ARM_USART_FLOW_CONTROL_NONE , ulBaudRate);
    
        Driver_USART3.Control(ARM_USART_CONTROL_TX | ARM_USART_CONTROL_RX, 1);
    #endif
        break;       
        
     default:
        return FALSE;
        
    }

  return TRUE;
}

/**
  * @brief  send a byte to usart
  * @param  None
  * @retval TRUE: success FALSE: failed
  */
int SerialDrvPutByte(uint8_t port, char ucByte )
{
    switch (port){
    case COM0:
        USART_SendData(USART1, ucByte);
        break;
    case COM1:
        USART_SendData(USART2, ucByte);
        break;
    case COM2:
        USART_SendData(USART3, ucByte);
        break;
    default:
        return FALSE;
    }
  
  return TRUE;
}

/**
  * @brief  get a byte from usart
  * @param  None
  * @retval TRUE: success FALSE: failed
  */
int SerialDrvGetByte(uint8_t port, char *pucByte )
{
    switch (port){
    case COM0:    
        *pucByte = USART_ReceiveData(USART1);
        break;
    case COM1:
        *pucByte = USART_ReceiveData(USART2);
        break;
    case COM2:
        *pucByte = USART_ReceiveData(USART3);
        break;
    default:
        return FALSE;
    }
    
    return TRUE;
}

/**
  * @brief  enable Rs485 RTS pin only for 485
  * @param  None
  * @retval None
  */
void SerialDrvEnable(uint8_t port, uint8_t xRxEnable, uint8_t xTxEnable)
{
    switch(port){
    case COM0:
        if(xRxEnable){
            USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
            //MAX485 rts ping disable enable receive 
            GPIO_ResetBits(GPIOD, GPIO_Pin_8);
        }
        else{
            USART_ITConfig(USART1, USART_IT_RXNE, DISABLE); 
            //MAX485 rts ping enable enable send 
            GPIO_SetBits(GPIOD, GPIO_Pin_8);
        }

        if(xTxEnable){
            USART_ITConfig(USART1, USART_IT_TC, ENABLE);
        }
        else{
            USART_ITConfig(USART1, USART_IT_TC, DISABLE);
        }
        break;
        
     case COM1:
        if(xRxEnable){
            USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
            //MAX485 rts ping disable enable receive 
            GPIO_ResetBits(GPIOD, GPIO_Pin_9);
        }
        else{
            USART_ITConfig(USART2, USART_IT_RXNE, DISABLE); 
            //MAX485 rts pin enable enable send 
            GPIO_SetBits(GPIOD, GPIO_Pin_9);
        }

        if(xTxEnable){
            USART_ITConfig(USART2, USART_IT_TC, ENABLE);
        }
        else{
            USART_ITConfig(USART2, USART_IT_TC, DISABLE);
        }
        break;
        
     case COM2:
        if(xRxEnable){
            USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
            //MAX485 rts ping disable enable receive 
            GPIO_ResetBits(GPIOD, GPIO_Pin_9);
        }
        else{
            USART_ITConfig(USART3, USART_IT_RXNE, DISABLE); 
            //MAX485 rts pin enable enable send 
            GPIO_SetBits(GPIOD, GPIO_Pin_9);
        }

        if(xTxEnable){
            USART_ITConfig(USART3, USART_IT_TC, ENABLE);
        }
        else{
            USART_ITConfig(USART3, USART_IT_TC, DISABLE);
        }
        break;    
        
     default:
        break;
    }
    
}

