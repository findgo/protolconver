
#include "usart.h"


typedef enum
{
    STATE_RX_IDLE,              /*!< RTU Receiver is in idle state. */
    STATE_RX_RCV,               /*!< RTU Frame is beeing received. */
    STATE_TX_XMIT               /*!< RTU Transmitter is in transfer state. */
} Rs485SndRcvState;  
    
typedef struct {   
    uint8_t port;
    uint8_t sndrcvState;
    int eventFlag;
    uint16_t sndAduBufCount;
    uint16_t sndAduBufPos;

    uint16_t rcvAduBufPos;

    uint16_t AduBufSize;
    uint8_t *pAduBuf;
}Rs485_dev_t;

static Rs485_dev_t rsdev1;
static uint8_t AduBuf1[16];

uint8_t Rs485Init(Rs485_dev_t *pdev, uint8_t port, uint32_t ulBaudRate, mb_Parity_t eParity )
{
    uint8_t eStatus = TRUE;
    
    switch(port){
    case COM0:
        eStatus = FALSE;
        break;
    
    case COM1:
        Rs485BaseStructInit( pdev, COM1, AduBuf1, sizeof(AduBuf1));
        
        if( SerialDrvInit( port, ulBaudRate, 8, eParity ) == TRUE ){
            // 启动 485设备
            pdev->sndrcvState = STATE_RX_IDLE;
            SerialDrvEnable(pdev->port, TRUE, FALSE);
        }
        else{
            eStatus = FALSE;
        }

        break;

    default:
        eStatus = FALSE;
        break;
    }
    return eStatus;
}


void Rs485BaseStructInit(Rs485_dev_t *pdev, uint8_t port, uint8_t *buf, uint16_t bufsize)
{
    configASSERT(pdev);
        
    pdev->port = port;
    pdev->pAduBuf = buf;
    pdev->AduBufSize = bufsize;
    pdev->sndAduBufCount = 0;
    pdev->sndAduBufPos = 0;
    pdev->rcvAduBufPos = 0;
    pdev->eventFlag = FALSE;
    pdev->sndrcvState = STATE_RX_IDLE;    
}

int Rs485Send(Rs485_dev_t *pdev, const uint8_t *dat, uint16_t uslen)
{
    int result = FALSE;
    
    taskENTER_CRITICAL();
    /* Check if the receiver is still in idle state. If not we where to
     * slow with processing the received frame and the master sent another
     * frame on the network. We have to abort sending the frame.
     */
    if(pdev->sndrcvState == STATE_RX_IDLE){
        // copy to sendbuff
        pdev->sndAduBufCount = MIN(uslen, pdev->AduBufSize);
        memcpy( (uint8_t *)pdev->pAduBuf, dat, pdev->sndAduBufCount );

        /* Activate the transmitter. */
		pdev->sndrcvState = STATE_TX_XMIT;
        
		/* start the first transmitter then into serial tc interrupt */
        SerialDrvPutByte(pdev->port, dat[0]);
        pdev->sndAduBufPos = 1;  /* next byte in sendbuffer. */
        pdev->sndAduBufCount--;
     	
        SerialDrvEnable(pdev->port, FALSE, TRUE);

        result = TRUE;
    }
    taskEXIT_CRITICAL();

    return result;
}

int Rs485Receive(Rs485_dev_t *pdev, uint8_t **pAdu, uint16_t *pLength)
{
    int eStatus = FALSE;

    taskENTER_CRITICAL();
    if(pdev->eventFlag == TRUE){
        pdev->eventFlag = FALSE;

        *pAdu = (uint8_t *)(pdev->pAduBuf);
        *pLength = pdev->rcvAduBufPos;
        
        eStatus = TRUE;
    }
    taskEXIT_CRITICAL();

    return eStatus;
}
//
void Rs485ResetState( Rs485_dev_t *pdev )
{
    pdev->sndrcvState == STATE_RX_IDLE;  
}

void Rs485TransmitFSM(  Rs485_dev_t *pdev)
{
    /* We should get a transmitter event in transmitter state.  */
    if(pdev->sndrcvState == STATE_TX_XMIT){
        /* check if we are finished. */
        if( pdev->sndAduBufCount != 0 ){
            SerialDrvPutByte(pdev->port, ( char )pdev->pAduBuf[pdev->sndAduBufPos]);
            pdev->sndAduBufPos++;  /* next byte in sendbuffer. */
            pdev->sndAduBufCount--;
        }
        else{
            /* Disable transmitter. This prevents another transmit buffer empty interrupt. */             
            SerialDrvEnable(pdev->port, TRUE, FALSE);
            pdev->sndrcvState = STATE_RX_IDLE;
        }
    }
    else {
        /* enable receiver/disable transmitter. */
        SerialDrvEnable(pdev->port, TRUE, FALSE);
    }
}


void Rs485ReceiveFSM(Rs485_dev_t *pdev)
{
    uint8_t ucByte;

    /* Always read the character. */
    ( void )SerialDrvGetByte(pdev->port, (char *)&ucByte);

    if((pdev->sndrcvState == STATE_RX_IDLE) || (pdev->sndrcvState == STATE_RX_RCV)) {
        if(pdev->sndrcvState == STATE_RX_IDLE){
            pdev->rcvAduBufPos = 0;
            pdev->pAduBuf[pdev->rcvAduBufPos++] = ucByte;
            pdev->sndrcvState = STATE_RX_RCV;
        }
        else { // STATE_RX_RCV
            /* We are currently receiving a frame. Reset the timer after
             * every character received. If more than the maximum possible
             * number of bytes in a modbus frame is received the frame is
             * ignored.
             */
            if(pdev->rcvAduBufPos < pdev->AduBufSize){
                pdev->pAduBuf[pdev->rcvAduBufPos++] = ucByte;
            }
            else{
                /* In the error state we wait until all characters in the
                 * damaged frame are transmitted.*/
            }
        }
        //
        if(Rs485ProcessRcvFSM(pdev,ucByte)){
            pdev->eventFlag = TRUE;
            pdev->sndrcvState = STATE_RX_IDLE;
            
        }
        
    }
    
    /* if use time ,then Enable expired timers. */
    //
}


void Rs485TimerReceiveExpired(  Rs485_dev_t *pdev)
{
    /* Disable expired timers. */
    //
    pdev->sndrcvState = STATE_RX_IDLE;
}

void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET){
        
        USART_ClearITPendingBit(USART2, USART_IT_TXE);
    }
    
    if(USART_GetITStatus(USART2,USART_IT_RXNE) != RESET){
        Rs485ReceiveFSM(&rsdev1);
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
    
    if(USART_GetITStatus(USART2, USART_IT_TC) != RESET){
        Rs485TransmitFSM(&rsdev1);
        USART_ClearITPendingBit(USART2, USART_IT_TC);
    }
    
    NVIC_ClearPendingIRQ(USART2_IRQn);
}

int Rs485ProcessRcvFSM(Rs485_dev_t *pdev) 
{
    int status = FALSE;
    
    switch (pdev->port){
    case COM0:

        break;
    case COM1:



        break;

    default:

        break;
    }
    
    return FALSE;   
}
