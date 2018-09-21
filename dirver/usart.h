#ifndef _USER_DIRVER_UART_H_
#define _USER_DIRVER_UART_H_

#include "app_cfg.h"

enum{
    COM0,
    COM1,
    COM2,
    COM3
};
    
typedef enum
{
    DRV_PAR_NONE,                /*!< No parity. */
    DRV_PAR_ODD,                 /*!< Odd parity. */
    DRV_PAR_EVEN                 /*!< Even parity. */
}DRV_Parity_t;


#define USART_USING1   					USART1
#define USART_USING1_IRQ				USART1_IRQn

#define USART_USING1_TX_PORT			GPIOA
#define USART_USING1_TX_PIN				GPIO_Pin_9

#define USART_USING1_RX_PORT			GPIOA
#define USART_USING1_RX_PIN				GPIO_Pin_10

#define USART_USING1_GPIO_PeriphClock_EN()  do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE); }while(0)
#define USART_USING1_PeriphClock_EN()	do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);}while(0)

#define USART_USING2   					USART2
#define USART_USING2_IRQ				USART2_IRQn

#define USART_USING2_TX_PORT			GPIOA
#define USART_USING2_TX_PIN				GPIO_Pin_2

#define USART_USING2_RX_PORT			GPIOA
#define USART_USING2_RX_PIN				GPIO_Pin_3

#define USART_USING2_GPIO_PeriphClock_EN()  do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE); }while(0)
#define USART_USING2_PeriphClock_EN()	do{ RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);}while(0)

int SerialDrvInit(uint8_t port, uint32_t ulBaudRate, uint8_t ucDataBits, DRV_Parity_t eParity);
int SerialDrvPutByte(uint8_t port, char ucByte );
int SerialDrvGetByte(uint8_t port, char *pucByte );

void SerialDrvEnable(uint8_t port, uint8_t xRxEnable, uint8_t xTxEnalbe);


#endif

