
#include "xiic.h"


/*********************************************************************************/
/*
函数名：iic_init
功  能：配置SDA 和SCL端口
参  数：无
返回值：无
*/
void xIIC_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
	/*****配置GPIO口**************************************/
    RCC_APB2PeriphClockCmd(xIIC_RCC_PORT, ENABLE);
   
	GPIO_InitStructure.GPIO_Pin=xIIC_SCL_PIN|xIIC_SDA_PIN;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	
	GPIO_Init(xIIC_SCL_PORT, &GPIO_InitStructure);	
	
	/********将IIC的 SCL SDA 上拉释放总线******************/
	xIIC_SDA_PIN_OUT = 1;
	xIIC_SCL_PIN_OUT = 1;
}

/*
函数名：iic_start
功  能：启动iic，启动方式，在SCL高电平期间将SDA由高置低
参  数：无
返回值：无
*/
static void xIIC_start(void)
{
	xIIC_SDA_OUTPUT();
	xIIC_SDA_PIN_OUT = 1;
	xIIC_SCL_PIN_OUT = 1;
	xIIC_DELAY_WIDE();
	xIIC_DELAY_WIDE();
	xIIC_SDA_PIN_OUT = 0;
	xIIC_DELAY_WIDE();
	xIIC_DELAY_WIDE();
	xIIC_SCL_PIN_OUT = 0;
	xIIC_DELAY_WIDE();
}
/*
函数名：iic_stop
功  能：停止传输数据，实现方式在SCL高电平期间将SDA由低置高
参  数：无
返回值：无
*/
static void xIIC_stop(void)
{
	xIIC_SDA_OUTPUT();	
    xIIC_SCL_PIN_OUT = 0;
	xIIC_SDA_PIN_OUT = 0;
	xIIC_DELAY_WIDE();
	xIIC_DELAY_WIDE();
	xIIC_SCL_PIN_OUT = 1;
	xIIC_DELAY_WIDE();
	xIIC_DELAY_WIDE();
	xIIC_SDA_PIN_OUT = 1;
	xIIC_DELAY_WIDE();
}
/*
函数名：iic_ack
功  能：接收从机应答信号，释放掉总线读取SDA置看是否有负脉冲,
        当一段时间无应答默认接收完毕
参  数：无
返回值：成功: success 失败: failed
*/
static u8 xIIC_wait_ack(void)
{
	u8 i=0;
    
	xIIC_SDA_INPUT();
    xIIC_SDA_PIN_OUT = 1;
    xIIC_DELAY_WIDE();
	xIIC_SCL_PIN_OUT = 1;
    xIIC_DELAY_WIDE();
	while(xIIC_SDA_PIN_IN)
	{
		i++;
		if(i>250) 
		{		
            xIIC_stop();			//如果i>255则产生非应答信号，iic停止
			return Failed;
		}
		
	}
	xIIC_SCL_PIN_OUT=0;//时钟输出0 
	
	return Success;
}
/*
函数名：iic_nask
功  能：产生非应答信号
参  数：无
返  回：无
*/
static void xIIC_nack(void)
{
	xIIC_SCL_PIN_OUT = 0;
	xIIC_SDA_OUTPUT();
	xIIC_SDA_PIN_OUT = 1;
	xIIC_DELAY_WIDE();
	xIIC_SCL_PIN_OUT = 1;
	xIIC_DELAY_WIDE();
	xIIC_SCL_PIN_OUT = 0;
}	
/*
函数名：iic_ask
功  能：产生ask应答
参  数：无
返  回：无
*/
static void xIIC_ack(void)
{
	xIIC_SCL_PIN_OUT = 0;
	xIIC_SDA_OUTPUT();
	xIIC_SDA_PIN_OUT = 0;
	xIIC_DELAY_WIDE();
	xIIC_SCL_PIN_OUT = 1;
	xIIC_DELAY_WIDE();
	xIIC_SCL_PIN_OUT = 0;
}
/*
函数名：iic_bit_write
功  能：传送一个字节
参  数：u8
返回值：无
*/
static void xIIC_byte_write(u8 byte)
{
    u8 i;
    
	xIIC_SDA_OUTPUT();
	xIIC_SCL_PIN_OUT = 0;
	for(i = 1;i <= 8; i++ )
	{
		xIIC_SDA_PIN_OUT = ( byte >> (8 - i)) & 0x01;
		xIIC_DELAY_WIDE();
		xIIC_SCL_PIN_OUT = 1;
		xIIC_DELAY_WIDE();
		xIIC_SCL_PIN_OUT = 0;
		xIIC_DELAY_WIDE();		
	}
}
/*
函数名：iic_bit_read
功  能：主机读取一个字节
参  数：ask
返回值：
*/
static u8 xIIC_byte_read(u8 Isneedask)
{
    u8 i,byte=0;
    
	xIIC_SDA_INPUT();
    for(i=0;i<8;i++)
	{
        xIIC_SCL_PIN_OUT = 0;
        xIIC_DELAY_WIDE();
        xIIC_SCL_PIN_OUT = 1;
        byte <<=1;
        if(xIIC_SDA_PIN_IN)
        byte++;
        xIIC_DELAY_WIDE();
	}
	if(!Isneedask)
		xIIC_nack();
	else
		xIIC_ack();
    
	return byte;
}
// success or failed
uint8_t xIICheckDevice(uint8_t devddress)
{
    uint8_t ucAck;

	if (xIIC_SDA_PIN_IN && xIIC_SCL_PIN_IN)
	{
		xIIC_start();		/* 发送启动信号 */

		/* 发送设备地址+读写控制bit0（0 = w， 1 = r) bit7 先传 */
		xIIC_byte_write(devddress);
		ucAck = xIIC_wait_ack();	/* 检测设备的ACK应答 */

		xIIC_stop();			/* 发送停止信号 */

		return ucAck;
	}
    
	return Failed;	/* I2C总线异常 */
}
uint8_t xIICWriteByte(u8 devaddr,u8 memAddress,u8 data)
{
    return xIICWriteMultiBytes( devaddr, memAddress,1, &data);
}
uint8_t xIICWriteMultiBytes(u8 devaddr,u8 memAddress,u8 len,u8 *wbuf)
{
	uint8_t ret ;
	int i = 0;
    
	xIIC_start();						/* 总线开始信号 */
	xIIC_byte_write(devaddr);	        /* 发送设备地址+写信号 */
	ret = xIIC_wait_ack();
    if(memAddress != xIICDEV_NO_MEM_ADDR){
    	xIIC_byte_write(memAddress);		/* 内部寄存器地址 */
        xIIC_wait_ack(); 
    }
    
	for(i = 0; i < len; i++){
		xIIC_byte_write(wbuf[i]);   /* 内部寄存器数据 */
		ret = xIIC_wait_ack();		
	}
	xIIC_stop();	
    
	return ret;
}

uint8_t xIICReadByte(u8 devaddr,u8 memAddress)
{
    uint8_t data;
    
    xIICReadMultiBytes(devaddr,memAddress,1,&data);

    return data;    
}

uint8_t xIICReadMultiBytes(u8 devaddr,u8 memAddress,u8 len,u8 *wbuf)
{
    uint8_t i = 0;
	
	xIIC_start();
	xIIC_byte_write(devaddr);	   //send devAddress address and write
	xIIC_wait_ack();
	if(memAddress != xIICDEV_NO_MEM_ADDR){
		xIIC_byte_write(memAddress);   // send memAddress address 
    	xIIC_wait_ack();	
	}
	
	xIIC_start();
	xIIC_byte_write(devaddr | 0x01); //send devAddress address and read 	
	xIIC_wait_ack();
	
    for(i = 0; i < len; i++){ 
		 if(i != (len - 1))
		 	wbuf[i] = xIIC_byte_read(TRUE);  //read with ack
		 else
			wbuf[i] = xIIC_byte_read(FALSE);	//last byte with nack
	}
    xIIC_stop();//stop
    
    return  Success;
}

/******************************非标准IIC Only for SHT2x*****************************************************/
/* 非主机模式 */ /* 主机模式不实现 */ 
uint8_t SHT_DevReadbyPoll(u8 devaddr,u8 _usRegAddr,u8 _ucLen,u8 *_pRegBuf)
{
    uint8_t i;
    uint32_t retry = 90000;
    
    xIIC_start();					/* 总线开始信号 */

    xIIC_byte_write(devaddr);	/* 发送设备地址+写信号 */
	xIIC_wait_ack();

    xIIC_byte_write(_usRegAddr);		/* 地址低8位 */
	xIIC_wait_ack();

    do
    {
        delay_us(1);
        xIIC_start();
        xIIC_byte_write(devaddr + 0x01);	/* 发送设备地址+读信号 */
    }while((xIIC_wait_ack() == Failed) && (retry--));

    if(retry > 0){
        for (i = 0; i <_ucLen; i++)
        {
            if(i != (_ucLen - 1))
               _pRegBuf[i] = xIIC_byte_read(TRUE);  //read with ack
            else
               _pRegBuf[i] = xIIC_byte_read(FALSE);   //last byte with nack
        }
        
        xIIC_stop();                            /* 总线停止信号 */
	    return Success;
    }
    
	return Failed;
}

uint8_t SHT_DevReadMeasure(u8 devaddr,u8 len,u8 *wbuf)
{
    uint8_t i = 0;
	uint16_t retry = 100;
    
	xIIC_start();
	xIIC_byte_write(devaddr);	   //send devAddress address and write
	xIIC_wait_ack();


    do
    {
        delay_us(1);
        xIIC_start();
        xIIC_byte_write(devaddr | 0x01);	/* 发送设备地址+读信号 */
    }while((xIIC_wait_ack() == Failed) && (retry--));
		
    if(retry > 0){
        for (i = 0; i <len; i++)
        {
            if(i != (len - 1))
               wbuf[i] = xIIC_byte_read(TRUE);  //read with ack
            else
               wbuf[i] = xIIC_byte_read(FALSE);   //last byte with nack
        }
        
        xIIC_stop();                            /* 总线停止信号 */
	    return Success;
    }
    
	return Failed;
}

//way3
void SHT_DevReadMultiReg(u8 devaddr,uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen)
{
    uint8_t i;

    xIIC_start();					/* 总线开始信号 */

    xIIC_byte_write(devaddr);	/* 发送设备地址+写信号 */
	xIIC_wait_ack();

    xIIC_byte_write(_usRegAddr >> 8);	/* 地址高8位 */
	xIIC_wait_ack();

    xIIC_byte_write(_usRegAddr);		/* 地址低8位 */
	xIIC_wait_ack();

	xIIC_start();
    xIIC_byte_write(devaddr + 0x01);	/* 发送设备地址+读信号 */
	xIIC_wait_ack();

	for (i = 0; i < _ucLen; i++)
	{
        if(i != (_ucLen - 1))
           _pRegBuf[i] = xIIC_byte_read(TRUE);  //read with ack
        else
           _pRegBuf[i] = xIIC_byte_read(FALSE);   //last byte with nack
	}

    xIIC_stop();							/* 总线停止信号 */
}
void SHT_DevWriteMultiReg(u8 devaddr,uint16_t _usRegAddr, uint8_t *_pRegBuf, uint8_t _ucLen)
{
	uint8_t i;

    xIIC_start();					/* 总线开始信号 */

    xIIC_byte_write(devaddr);	/* 发送设备地址+写信号 */
	xIIC_wait_ack();

    xIIC_byte_write(_usRegAddr >> 8);	/* 地址高8位 */
	xIIC_wait_ack();

    xIIC_byte_write(_usRegAddr);		/* 地址低8位 */
	xIIC_wait_ack();

	for (i = 0; i < _ucLen; i++)
	{
	    xIIC_byte_write(_pRegBuf[i]);		/* 寄存器数据 */
		xIIC_wait_ack();
	}

    xIIC_stop();                   			/* 总线停止信号 */
}
