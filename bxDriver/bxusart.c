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
#include "bxusart.h"

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
    (void)port; //���޸�����λ����
    (void)ulBaudRate; //���޸�����λ����
    (void)ucDataBits; //���޸�����λ����
    (void)eParity;    //���޸�У���ʽ
    
    switch (port){
    case COM0:
        MX_USART1_UART_Init();

        LL_USART_ClearFlag_TC(USART_USING1);
        LL_USART_ClearFlag_RXNE(USART_USING1);
        LL_USART_DisableIT_TXE(USART_USING1);
        LL_USART_DisableIT_TC(USART_USING1);
        LL_USART_EnableIT_RXNE(USART_USING1);
        break;
        
    case COM1:
        MX_USART2_UART_Init();
        
        LL_USART_ClearFlag_TC(USART_USING2);
        LL_USART_ClearFlag_RXNE(USART_USING2);
        LL_USART_DisableIT_TXE(USART_USING2);
        LL_USART_DisableIT_TC(USART_USING2);
        LL_USART_EnableIT_RXNE(USART_USING2);
        break;
         
    case COM2:
        MX_USART3_UART_Init();
        
        LL_USART_ClearFlag_TC(USART_USING3);
        LL_USART_ClearFlag_RXNE((USART_USING3));
        LL_USART_DisableIT_TXE((USART_USING3));
        LL_USART_DisableIT_TC((USART_USING3));
        LL_USART_EnableIT_RXNE((USART_USING3));
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
        LL_USART_TransmitData8(USART_USING1, ucByte);
        break;
    case COM1:
        LL_USART_TransmitData8(USART_USING2, ucByte);
        break;
    case COM2:
        LL_USART_TransmitData8(USART_USING3, ucByte);
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
        *pucByte = LL_USART_ReceiveData8(USART_USING1);
        break;
    case COM1:
        *pucByte = LL_USART_ReceiveData8(USART_USING2);
        break;
    case COM2:
        *pucByte = LL_USART_ReceiveData8(USART_USING3);
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
            LL_USART_EnableIT_RXNE(USART_USING1);
            //MAX485 rts ping disable enable receive 
            //GPIO_ResetBits(GPIOD, GPIO_Pin_8);
        }
        else{
            LL_USART_DisableIT_RXNE(USART_USING1); 
            //MAX485 rts ping enable enable send 
            //GPIO_SetBits(GPIOD, GPIO_Pin_8);
        }

        if(xTxEnable){
            LL_USART_EnableIT_TC(USART_USING1);
        }
        else{
            LL_USART_DisableIT_TC(USART_USING1);
        }
        break;
        
     case COM1:
        if(xRxEnable){
            LL_USART_EnableIT_RXNE(USART_USING2);
            //MAX485 rts ping disable enable receive 
            //GPIO_ResetBits(GPIOD, GPIO_Pin_9);
        }
        else{
            LL_USART_DisableIT_RXNE(USART_USING2); 
            //MAX485 rts pin enable enable send 
            //GPIO_SetBits(GPIOD, GPIO_Pin_9);
        }

        if(xTxEnable){
            LL_USART_EnableIT_TC(USART_USING2);
        }
        else{
            LL_USART_DisableIT_TC(USART_USING2);
        }
        break;
        
     case COM2:
        if(xRxEnable){
            LL_USART_EnableIT_RXNE(USART_USING3);
            //MAX485 rts ping disable enable receive 
           // GPIO_ResetBits(GPIOD, GPIO_Pin_9);
        }
        else{
            LL_USART_DisableIT_RXNE(USART_USING3); 
            //MAX485 rts pin enable enable send 
            //GPIO_SetBits(GPIOD, GPIO_Pin_9);
        }

        if(xTxEnable){
            LL_USART_EnableIT_TC(USART_USING3);
        }
        else{
            LL_USART_DisableIT_TC(USART_USING3);
        }
        break;    
        
     default:
        break;
    }
    
}

