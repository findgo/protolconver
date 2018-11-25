
 
#include "mserial.h"


typedef struct {
    uint8_t *ptxbuf;
    uint16_t txHead;    //发送缓冲区头
    uint16_t txTail;    //发送缓冲区尾
    uint16_t txcount;   //发送缓冲有效个数
    uint16_t txsize;    //发送缓冲区大小

    uint8_t *prxbuf;
    uint16_t rxHead;    //接收缓冲区头
    uint16_t rxTail;    //接收缓冲区尾
    uint16_t rxcount;   //接收缓冲有效个数
    uint16_t rxsize;    //接收缓冲区大小
}comcfg_t;

//发送缓冲区空闲字节数
#define SERIAL_TX_IDLE_AVAIL(ptr)   (ptr->txsize - ptr->txcount)
//发送缓冲区有效节数
#define SERIAL_TX_VALID_AVAIL(ptr)  (ptr->txcount)
//发送缓冲区是否有可读数据
#define IS_SERIAL_TX_VALID(ptr) (ptr->txcount > 0)

//接收缓冲区空闲字节数
#define SERIAL_RX_IDLE_AVAIL(ptr)   (ptr->rxsize - ptr->rxcount)
//接收缓冲区有效字节数   
#define SERIAL_RX_VALID_AVAIL(ptr)  (ptr->rxcount)
//接收缓冲区是否有可读数据
#define IS_SERIAL_RX_VALID(ptr) (ptr->rxcount > 0)

//private 
#if COM_USE_NUM >= 1
static uint8_t Com0TxBuf[COM0_TX_MAX_SIZE]; //发送缓冲区
static uint8_t Com0RxBuf[COM0_RX_MAX_SIZE]; //接收缓冲区
static comcfg_t comcfg0 = {&Com0TxBuf[0], 0, 0, 0, COM0_TX_MAX_SIZE, &
Com0RxBuf[0], 0, 0, 0, COM0_RX_MAX_SIZE}; // serial对象实例
#endif
#if COM_USE_NUM >= 2
static uint8_t Com1TxBuf[COM1_TX_MAX_SIZE]; //发送缓冲区
static uint8_t Com1RxBuf[COM1_RX_MAX_SIZE]; //接收缓冲区
static comcfg_t comcfg1 = {&Com1TxBuf[0], 0, 0, 0, COM1_TX_MAX_SIZE, &
Com1RxBuf[0], 0, 0, 0, COM1_RX_MAX_SIZE}; // serial对象实例
#endif
#if COM_USE_NUM >= 3
static uint8_t Com2TxBuf[COM2_TX_MAX_SIZE]; //发送缓冲区
static uint8_t Com2RxBuf[COM2_RX_MAX_SIZE]; //接收缓冲区
static comcfg_t comcfg2 = {&Com2TxBuf[0], 0, 0, 0, COM2_TX_MAX_SIZE, &
Com2RxBuf[0], 0, 0, 0, COM2_RX_MAX_SIZE}; // serial对象实例
#endif
#if COM_USE_NUM >= 4
static uint8_t Com3TxBuf[COM3_TX_MAX_SIZE]; //发送缓冲区
static uint8_t Com3RxBuf[COM3_RX_MAX_SIZE]; //接收缓冲区
static comcfg_t comcfg3 = {&Com3TxBuf[0], 0, 0, 0, COM3_TX_MAX_SIZE, &
Com3RxBuf[0], 0, 0, 0, COM3_RX_MAX_SIZE}; // serial对象实例
#endif




static comcfg_t *GetUseCom(uint8_t COM)
{
    switch(COM){
#if COM_USE_NUM >= 1
    case COM0:
        return &comcfg0;
#endif
#if COM_USE_NUM >= 2
    case COM1:
        return &comcfg1;
#endif
#if COM_USE_NUM >= 3
    case COM2:
        return &comcfg2;
#endif
#if COM_USE_NUM >= 4
    case COM3:
        return &comcfg3;
#endif
    default:
        return &comcfg0;
    }
}



/**
  * @brief  向发送缓冲区存多个数据
  * @param  buf: 要存入缓冲区的数据 Len: 
存入缓冲区的数据长度 
  * @note   
  * @retval number successful put into buf
  */
static uint16_t SerialTxPut( comcfg_t *cfg, uint8_t *buf, uint16_t len )
{
    uint16_t availcount;

    len = MIN( SERIAL_TX_IDLE_AVAIL(cfg), len );    
    availcount = len;
    
    while(availcount){
        cfg->ptxbuf[cfg->txTail] = *buf++;
        cfg->txcount++;
        if (++cfg->txTail >= cfg->txsize){
            cfg->txTail = 0;
        } 
        availcount--;
    }

    return len;
}
/**
  * @brief  发送缓冲区取出一个字节数据
  * @param  dat: 要取缓冲区的数据地址
  * @note   
  * @retval 返回取出的字节数
  * @note       中断发送取数据时需要调用
  */
uint16_t SerialTxPop( comcfg_t *cfg, uint8_t *dat )
{
    if(IS_SERIAL_TX_VALID(cfg)){//鏄惁鏈夋湁鏁堟暟鎹?
        *dat = cfg->ptxbuf[cfg->txHead];
        cfg->txcount--;
        if(++cfg->txHead >= cfg->txsize){
            cfg->txHead = 0;
        }
        return 1; 
    }

    return 0;
}
/**
  * @brief  读发送缓冲区有效字节数
  * @param  
  * @note   
  * @retval 返回发送缓冲区有效字节数
  * @note       
  */
uint16_t SerialTxValidAvail( uint8_t COM )
{
    comcfg_t *cfg;
    
    cfg = GetUseCom(COM);
    return SERIAL_TX_VALID_AVAIL(cfg);
}

/**
  * @brief  向接收缓冲区存入一个数据
  * @param  dat:存入的数据
  * @note   
  * @retval     ture or false
  * @note       中断接收存数据时中断调用
  */
bool SerialRxPut( comcfg_t *cfg, uint8_t dat )
{
    if(SERIAL_RX_IDLE_AVAIL(cfg) > 0){
        cfg->prxbuf[cfg->rxTail] = dat;
        cfg->rxcount++;
        if(++cfg->rxTail >= cfg->rxsize){
            cfg->rxTail = 0;
        } 
        return true;
    }

    return false;
}

/**
  * @brief  从接收缓冲区取出len个数据
  * @param  buf:取出数据的缓冲区,len: 数据长度
  * @note   
  * @retval  返回取出数据的实际个数
  */
static uint16_t SerialRxPop( comcfg_t *cfg, uint8_t *buf, uint16_t len )
{
    uint16_t cnt = 0;
    
    while(IS_SERIAL_RX_VALID(cfg) && (cnt < len))//接收缓存区有数据
    {
        *buf++ = cfg->prxbuf[cfg->rxHead];//从接收缓冲区提取数据
        cfg->rxcount--;
        if(++cfg->rxHead >= cfg->rxsize){
            cfg->rxHead = 0;
        }
        cnt++;
    }

    return cnt;
}
/**
  * @brief  从接收缓冲区取出len个数据
  * @param  buf:取出数据的缓冲区,len: 数据长度
  * @note   
  * @retval  返回取出数据的实际个数
  */
uint16_t SerialRxValidAvail( uint8_t COM )
{
    comcfg_t *cfg;
    
    cfg = GetUseCom(COM);
    return SERIAL_RX_VALID_AVAIL(cfg);
}

//启动一个发送空中断
static void Start_TXEtransmit( uint8_t COM, comcfg_t *cfg )
{
    //采用发送完成空中断
    if(IS_SERIAL_TX_VALID(cfg)){//有数据
        switch(COM){
#if COM_USE_NUM >= 1
            case COM0:
                COM0TxIEEnable();
                break;
#endif
#if COM_USE_NUM >= 2
            case COM1:
                COM1TxIEEnable();
                break;
#endif
#if COM_USE_NUM >= 3
            case COM2:
                COM2TxIEEnable();
                break;
#endif
#if COM_USE_NUM >= 4
            case COM3:
                COM3TxIEEnable();
                break;
#endif
            default:
                COM0TxIEEnable();
                break;
        }
    }
}
//启动一个发送完成中断
static void Start_TXCtransmit(uint8_t COM,comcfg_t *cfg)
{
    uint8_t temp;
        
    if(SerialTxPop(cfg,&temp)){
         switch(COM){
#if COM_USE_NUM >= 1
            case COM0:
                COM0PutByte(temp);
                break;
#endif
#if COM_USE_NUM >= 2
            case COM1:
                COM1PutByte(temp);
                break;
#endif
#if COM_USE_NUM >= 3
            case COM2:
                COM2PutByte(temp);
                break;
#endif
#if COM_USE_NUM >= 4
            case COM3:
                COM3PutByte(temp);
                break;
#endif
            default:
                COM0PutByte(temp);
                break;
        }
    }
   
}
/**
  * @brief  TX输出长度为len的数据
  * @param  buf:输出数据缓冲区,len:输出数据长度
  * @note   
  * @retval  number write
  */
uint16_t Serial_WriteBuf(uint8_t COM,uint8_t *buf,uint16_t len)
{
    uint16_t count;
    comcfg_t *cfg;

    cfg = GetUseCom(COM);
    taskENTER_CRITICAL();
    count = SerialTxPut(cfg,buf, len);
        //采用发送完成中断 has some bug
//    Start_TXCtransmit(COM,cfg);

    //采用发送完成空中断
    Start_TXEtransmit(COM,cfg);
    taskEXIT_CRITICAL();
    
    return count;
}



uint16_t Serial_WriteByte( uint8_t COM, uint8_t dat )
{
    uint16_t count;
    comcfg_t *cfg;

    cfg = GetUseCom(COM);
    taskENTER_CRITICAL();
    count = SerialTxPut( cfg, &dat, 1 );
    
    //采用发送完成中断 has some bug
//    Start_TXCtransmit(COM,cfg);
    Start_TXEtransmit(COM,cfg);

    taskEXIT_CRITICAL();

    return count;
}

/**
  * @brief  从接收取出len个数据
  * @param  buf:读入数据缓冲区,len:读入数据长度
  * @note   
  * @retval  返回取到数据的实际个数
  */
uint16_t Serial_Read( uint8_t COM, uint8_t *buf, uint16_t len )
{
    uint16_t length;
    comcfg_t *cfg;

    cfg = GetUseCom(COM);
    taskENTER_CRITICAL();
    length = SerialRxPop(cfg, buf, len);
    taskEXIT_CRITICAL();
    
    return length;
}

/***********only for Isr callback***********/
/**
  * @brief  发送空中断调用函数
  * @param  None
  * @note   
  * @retval  None
  */
void COM0_TXE_Isr_callback(void)
{
    uint8_t temp;
    isrSaveCriticial_status_Variable;

    isrENTER_CRITICAL();
    if(SerialTxPop(&comcfg0,&temp)){
        COM0PutByte(temp);
    }else{
        COM0TxIEDisable();
    }
    isrEXIT_CRITICAL();
}
/**
  * @brief  发送完成中断回调函数
  * @param  None
  * @note   
  * @retval  None
  */
void COM0_TXC_Isr_callback(void)
{


}
/**
  * @brief  接收中断回调函数
  * @param  None
  * @note   
  * @retval  None
  */
void COM0_RX_Isr_callback(void)
{
    uint8_t temp = COM0GetByte();
    isrSaveCriticial_status_Variable;

    isrENTER_CRITICAL();    
    SerialRxPut(&comcfg0,temp);
    isrEXIT_CRITICAL();
}

/**
  * @brief  发送空中断调用函数
  * @param  None
  * @note   
  * @retval  None
  */
void COM1_TXE_Isr_callback(void)
{
    uint8_t temp;
    isrSaveCriticial_status_Variable;

    isrENTER_CRITICAL();
    if(SerialTxPop(&comcfg1,&temp)){
        COM1PutByte(temp);
    }else{
        COM1TxIEDisable();
    }
    isrEXIT_CRITICAL();
}
/**
  * @brief  发送完成中断回调函数
  * @param  None
  * @note   
  * @retval  None
  */
void COM1_TXC_Isr_callback(void)
{


}
/**
  * @brief  接收中断回调函数
  * @param  None
  * @note   
  * @retval  None
  */
void COM1_RX_Isr_callback(void)
{
    uint8_t temp = COM1GetByte();
    isrSaveCriticial_status_Variable;

    isrENTER_CRITICAL();    
    SerialRxPut(&comcfg1,temp);
    isrEXIT_CRITICAL();
}
/**
  * @brief  锟斤拷锟酵匡拷锟叫断碉拷锟矫猴拷锟斤拷
  * @param  None
  * @note   
  * @retval  None
  */
void COM2_TXE_Isr_callback(void)
{
    uint8_t temp;
    isrSaveCriticial_status_Variable;

    isrENTER_CRITICAL();
    if(SerialTxPop(&comcfg2,&temp)){
        COM2PutByte(temp);
    }else{
        COM2TxIEDisable();
    }
    isrEXIT_CRITICAL();
}
/**
  * @brief  锟斤拷锟斤拷锟斤拷锟斤拷卸匣氐锟斤拷锟斤拷锟?
  * @param  None
  * @note   
  * @retval  None
  */
void COM2_TXC_Isr_callback(void)
{


}
/**
  * @brief  锟斤拷锟斤拷锟叫断回碉拷锟斤拷锟斤拷
  * @param  None
  * @note   
  * @retval  None
  */
void COM2_RX_Isr_callback(void)
{
//    uint8_t temp = COM2GetByte();
//    isrSaveCriticial_status_Variable;
//
//    isrENTER_CRITICAL();    
//    SerialRxPut(&comcfg2,temp);
//    isrEXIT_CRITICAL();
}

/**
  * @brief  This function handles usart interrupt request.
  * @param  None
  * @retval None
  */
void USART1_IRQHandler(void)
{
    if(LL_USART_IsActiveFlag_TXE(USART1)){
        
        COM0_TXE_Isr_callback();
    }
    
    if(LL_USART_IsActiveFlag_RXNE(USART1)){
        COM0_RX_Isr_callback();
        LL_USART_ClearFlag_RXNE(USART1);
    }
    
    if(LL_USART_IsActiveFlag_TC(USART1)){
        COM0_TXC_Isr_callback();
        LL_USART_ClearFlag_TC(USART1);
    }
    NVIC_ClearPendingIRQ(USART1_IRQn);
}
void USART2_IRQHandler(void)
{
    if(LL_USART_IsActiveFlag_TXE(USART2)){
        
        COM1_TXE_Isr_callback();
    }
    
    if(LL_USART_IsActiveFlag_RXNE(USART2)){
        COM1_RX_Isr_callback();
        LL_USART_ClearFlag_RXNE(USART2);
    }
    
    if(LL_USART_IsActiveFlag_TC(USART2)){
        COM1_TXC_Isr_callback();
        LL_USART_ClearFlag_TC(USART2);
    }

    NVIC_ClearPendingIRQ(USART2_IRQn);
}

void USART3_IRQHandler(void)
{
    if(LL_USART_IsActiveFlag_TXE((USART3))){
        
        COM2_TXE_Isr_callback();
    }
    
    if(LL_USART_IsActiveFlag_RXNE(USART3)){
        COM2_RX_Isr_callback();
        LL_USART_ClearFlag_RXNE((USART3));
    }
    
    if(LL_USART_IsActiveFlag_TC((USART3))){
        COM2_TXC_Isr_callback();
        LL_USART_ClearFlag_TC((USART3));
    }

    NVIC_ClearPendingIRQ(USART3_IRQn);
}

