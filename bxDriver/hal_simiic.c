
#include "soft_iic.h"

void IIC_Init(void)
{			
	GPIO_InitTypeDef GPIO_InitStructure;
	
	SOFT_IIC_GPIO_PeriphClock_EN();			     

	GPIO_InitStructure.GPIO_Pin = SOFT_IIC_SCL_PIN;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(SOFT_IIC_SCL_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = SOFT_IIC_SDA_PIN;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(SOFT_IIC_SDA_PORT, &GPIO_InitStructure);

	SOFT_IIC_SDA_HIGH();	  	  
	SOFT_IIC_SCL_HIGH();
}
static void IIC_Start(void)
{
	SOFT_IIC_SDA_OUTPUT();     //sda set output
	SOFT_IIC_SDA_HIGH();	  	  
	SOFT_IIC_SCL_HIGH();
	SOFT_IIC_DELAY_WIDE();
	SOFT_IIC_DELAY_WIDE();
 	SOFT_IIC_SDA_LOW();		//START:when CLK is high,DATA change form high to low 
	SOFT_IIC_DELAY_WIDE();
	SOFT_IIC_DELAY_WIDE();
	SOFT_IIC_SCL_LOW();		//ready to send data or receive data
}

static void IIC_Stop(void)
{
	SOFT_IIC_SDA_OUTPUT();	//sda set output
	SOFT_IIC_SCL_LOW();
	SOFT_IIC_SDA_LOW();		//STOP:when CLK is high DATA change form low to high
	SOFT_IIC_DELAY_WIDE();
	SOFT_IIC_DELAY_WIDE();
	SOFT_IIC_SCL_HIGH(); 
	SOFT_IIC_DELAY_WIDE();
	SOFT_IIC_DELAY_WIDE();
	SOFT_IIC_SDA_HIGH();	//send stop signal
	SOFT_IIC_DELAY_WIDE();
}

//成功: success ,失败 : failed
static uint8_t IIC_Wait_Ack(void)
{
	uint8_t ucErrTime = 0;
	
	SOFT_IIC_SDA_INPUT();      //SDA set input  
	SOFT_IIC_SDA_HIGH();
	SOFT_IIC_DELAY_WIDE();
	SOFT_IIC_SCL_HIGH();
	SOFT_IIC_DELAY_WIDE();
	while(SOFT_IIC_SDA_READ)
	{
		ucErrTime++;
		if(ucErrTime > 50){
			IIC_Stop();
			return Failed;
		}
		SOFT_IIC_DELAY_WIDE();
	}
	SOFT_IIC_SCL_LOW();
	
	return Success;  
} 

// 产生应答
static void IIC_Ack(void)
{
	SOFT_IIC_SCL_LOW();
	SOFT_IIC_SDA_OUTPUT();
	SOFT_IIC_SDA_LOW();
	SOFT_IIC_DELAY_WIDE();
	SOFT_IIC_SCL_HIGH();
	SOFT_IIC_DELAY_WIDE();
	SOFT_IIC_SCL_LOW();
}
//产生非应答	
static void IIC_NAck(void)
{
	SOFT_IIC_SCL_LOW();
	SOFT_IIC_SDA_OUTPUT();
	SOFT_IIC_SDA_HIGH();
	SOFT_IIC_DELAY_WIDE();
	SOFT_IIC_SCL_HIGH();
	SOFT_IIC_DELAY_WIDE();
	SOFT_IIC_SCL_LOW();
}					 				     

static void IIC_Send_Byte(uint8_t txbyte)
{                        
	uint8_t i;

	SOFT_IIC_SDA_OUTPUT(); 	    
	SOFT_IIC_SCL_LOW();		//begin to send data
	for(i = 0; i < 8; i++){
		if(txbyte & 0x80)
			SOFT_IIC_SDA_HIGH();
		else
			SOFT_IIC_SDA_LOW();
		txbyte <<= 1; 	  
		SOFT_IIC_DELAY_WIDE();   
		SOFT_IIC_SCL_HIGH();
		SOFT_IIC_DELAY_WIDE(); 
		SOFT_IIC_SCL_LOW();	
		SOFT_IIC_DELAY_WIDE();
	}	 
} 	 
   
static uint8_t IIC_Read_Byte(uint8_t isNeedAck)
{
	uint8_t i,receive = 0;

	SOFT_IIC_SDA_INPUT();//SDA set input
	for(i = 0; i < 8; i++ ){
		SOFT_IIC_SCL_LOW(); 
		SOFT_IIC_DELAY_WIDE();
		SOFT_IIC_SCL_HIGH();
		receive <<= 1;
		if(SOFT_IIC_SDA_READ)
			receive |= 0x01;   
		SOFT_IIC_DELAY_WIDE();
	}					 
	if(isNeedAck)
		IIC_Ack(); //send ACK 
	else
		IIC_NAck();//send NACK

	return receive;
}

uint8_t IICreadBuf(uint8_t devAddress, uint8_t memAddress, uint8_t length, uint8_t *buf)
{
    uint8_t i = 0;
	uint8_t temp;
	
	IIC_Start();
	IIC_Send_Byte(devAddress);	   //send devAddress address and write
	IIC_Wait_Ack();
	if(memAddress != IICDEV_NO_MEM_ADDR){
		IIC_Send_Byte(memAddress);   // send memAddress address 
    	IIC_Wait_Ack();	
	}
	
	IIC_Start();
	IIC_Send_Byte(devAddress | 0x01); //send devAddress address and read 	
	IIC_Wait_Ack();
	
    for(i = 0; i < length; i++){ 
		 if(i != (length - 1))
		 	temp = IIC_Read_Byte(TRUE);  //read with ack
		 else
			temp = IIC_Read_Byte(FALSE);	//last byte with nack

		buf[i] = temp;
	}
    IIC_Stop();//stop
    
    return  Success;
}


uint8_t IICreadByte(uint8_t devAddress, uint8_t memAddress, uint8_t *pdata)
{
    return IICreadBuf(devAddress, memAddress, 1, pdata);
}

uint8_t IICreadBit(uint8_t devAddress, uint8_t memAddress, uint8_t bitNum, uint8_t *data)
{
	uint8_t byte;
	uint8_t status = Failed;

	status = IICreadBuf( devAddress, memAddress, 1, &byte);
	*data = (byte > 0) ? 0x01 : 0x00;

	return status;
}
uint8_t IICreadBits(uint8_t devAddress, uint8_t memAddress, uint8_t bitStart, uint8_t length, uint8_t *data)
{
	uint8_t byte;
	uint8_t mask;
	uint8_t status = Failed;

	if((status = IICreadBuf( devAddress, memAddress, 1, &byte)) == Success){
		mask = (1 << length) - 1;
		byte >>= bitStart;
		byte &= mask;
		*data = byte;
	}
	
	return status;
}
uint8_t IICwriteBuf(uint8_t devAddress, uint8_t memAddress, uint8_t length, uint8_t *buf)
{
	uint8_t i = 0;

	IIC_Start();
	IIC_Send_Byte(devAddress);	   //send devAddress address and write
	IIC_Wait_Ack();
	if(memAddress != IICDEV_NO_MEM_ADDR){
		IIC_Send_Byte(memAddress);  // send memAddress address 
		IIC_Wait_Ack();	
	}
	for(i = 0; i < length; i++){
		IIC_Send_Byte(buf[i]); 
		IIC_Wait_Ack(); 
	}
	IIC_Stop();//stop

	return Success;
}

uint8_t IICwriteByte(uint8_t devAddress, uint8_t memAddress, uint8_t data)
{
    return IICwriteBuf(devAddress, memAddress, 1, &data);
}


uint8_t IICwriteBits(uint8_t devAddress, uint8_t memAddress, uint8_t bitStart, uint8_t length, uint8_t data)
{
    uint8_t byte;
	uint8_t mask;
	uint8_t status = Failed;
		
    if ((status = IICreadBuf(devAddress, memAddress, 1, &byte)) == Success) {
        mask = ((1 << length) - 1) << bitStart;
        data <<= bitStart;
        data &= mask;
        byte &= ~mask;
        byte |= data;
        
        return IICwriteBuf(devAddress, memAddress, 1, &byte);
    } 
	
	return status;
}
uint8_t IICwriteBit(uint8_t devAddress, uint8_t memAddress, uint8_t bitNum, uint8_t bitVal)
{
    uint8_t byte;
    
    IICreadBuf(devAddress, memAddress, 1, &byte); // 先读回来
    byte = (bitVal > 0) ? (byte | (1 << bitNum)) : (byte & ~(1 << bitNum));// 再改变
    
    return IICwriteBuf(devAddress, memAddress, 1, &byte); // 写回去
}

//------------------End of File-------------

