/***********************************************************************************
Project:          SHT1x demo program (V2.1)
Filename:         SHT1x_Sample_Code.c    

Prozessor:        80C51 family
Compiler:         Keil Version 6.14

Autor:            MST
Copyrigth:        (c) Sensirion AG      
***********************************************************************************/
#include "./SHT/SHT1x.h"

 void SHT1x_DataPinOut(void) 
{ 
    GPIO_InitTypeDef GPIO_InitStructure;


    GPIO_InitStructure.GPIO_Pin =  SHT1x_I2C_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SHT1x_PORT_I2C, &GPIO_InitStructure);
		

} 
 void SHT1x_DataPinIn(void) 
{ 
    GPIO_InitTypeDef GPIO_InitStructure;


    GPIO_InitStructure.GPIO_Pin =  SHT1x_I2C_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(SHT1x_PORT_I2C, &GPIO_InitStructure);
} 
 void SHT1x_SckPinOut(void) 
{ 
    GPIO_InitTypeDef GPIO_InitStructure;


    GPIO_InitStructure.GPIO_Pin =  SHT1x_I2C_SCL_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SHT1x_PORT_I2C, &GPIO_InitStructure);
		

} 
//----------------------------------------------------------------------------------
//初始化SHT1x，根据需要请参考pdf进行修改****

void SHT1x_Init(void)
{

	SHT1x_SckPinOut();
	SHT1x_DataPinOut();	
	GPIO_SetBits(SHT1x_PORT_I2C, SHT1x_I2C_SCL_PIN);
	GPIO_SetBits(SHT1x_PORT_I2C, SHT1x_I2C_SDA_PIN);
}

 int8_t SHT1x_write_byte(uint8_t value)
//----------------------------------------------------------------------------------
// writes a byte on the Sensibus and checks the acknowledge 
{ 

  uint8_t i,error=0;  
	  SHT1x_DataPinOut();
  for (i=0x80;i>0;i/=2)             //shift bit for masking
  { 
  SHT1x_DataPinOut();
  if (i & value) SHT1x_SDA_PIN_OUT=1;          //masking value with i , write to SENSI-BUS
    else SHT1x_SDA_PIN_OUT=0;                        
    SHT1x_SCL_PIN_OUT=1;                          //clk for SENSI-BUS
    _nop_();_nop_();_nop_();        //pulswith approx. 5 us  	
    SHT1x_SCL_PIN_OUT=0;
  }
  SHT1x_SDA_PIN_OUT=1;                           //release SHT1x_SDA_PIN_IN-line

  SHT1x_SCL_PIN_OUT=1;                            //clk #9 for ack 
		SHT1x_DataPinIn();

  error=SHT1x_SDA_PIN_IN;                       //check ack (SHT1x_SDA_PIN_IN will be pulled down by SHT11)
  SHT1x_SCL_PIN_OUT=0;        
  return error;                     //error=1 SHT1x_DataPinIn case of no acknowledge
}

//----------------------------------------------------------------------------------
 int8_t SHT1x_read_byte(uint8_t ack)
//----------------------------------------------------------------------------------
// reads a byte form the Sensibus and gives an acknowledge SHT1x_DataPinIn case of "ack=1" 
{ 

  uint8_t i,val=0;
	  SHT1x_DataPinOut();
  SHT1x_SDA_PIN_OUT=1;                           //release SHT1x_SDA_PIN_IN-line
  for (i=0x80;i>0;i/=2)             //shift bit for masking
  { 
    SHT1x_DataPinOut();
    SHT1x_SCL_PIN_OUT=1; 
    SHT1x_DataPinIn();                         //clk for SENSI-BUS
    if (SHT1x_SDA_PIN_IN) val=(val | i);        //read bit  
    SHT1x_DataPinOut();
    SHT1x_SCL_PIN_OUT=0;  					 
  }
  SHT1x_SDA_PIN_OUT=!ack;                        //SHT1x_DataPinIn case of "ack==1" pull down SHT1x_SDA_PIN_OUT-Line
  SHT1x_SCL_PIN_OUT=1;                            //clk #9 for ack
  _nop_();_nop_();_nop_();          //pulswith approx. 5 us 
  SHT1x_SCL_PIN_OUT=0;						    
  SHT1x_SDA_PIN_OUT=1;                           //release SHT1x_SDA_PIN_OUT-line
  return val;
}

//----------------------------------------------------------------------------------
 void SHT1x_transstart(void)
{  
   SHT1x_SDA_PIN_OUT=1; SHT1x_SCL_PIN_OUT=0;                   //Initial state

   _nop_();
   SHT1x_SCL_PIN_OUT=1;

   _nop_();
	
   SHT1x_SDA_PIN_OUT=0;

   _nop_();
   SHT1x_SCL_PIN_OUT=0;  

   _nop_();_nop_();_nop_();
   SHT1x_SCL_PIN_OUT=1;

   _nop_();
   SHT1x_SDA_PIN_OUT=1;	

   _nop_();
   SHT1x_SCL_PIN_OUT=0;		   
}

//----------------------------------------------------------------------------------
 void SHT1x_connectionreset(void)
{  
  uint8_t i; 
  SHT1x_SDA_PIN_IN=1; SHT1x_SCL_PIN_OUT=0;                    //Initial state
  for(i=0;i<9;i++)                  //9 SHT1x_SCL_PIN_OUT cycles
  { SHT1x_SCL_PIN_OUT=1;
    SHT1x_SCL_PIN_OUT=0;
  }
  SHT1x_transstart();                   //transmission start
}

//----------------------------------------------------------------------------------
 int8_t SHT1x_softreset(void)
//----------------------------------------------------------------------------------
// resets the sensor by a softreset 
{ 
  uint8_t error=0;  
  SHT1x_connectionreset();              //reset communication
  error+=SHT1x_write_byte(SHT1X_RESET);       //send RESET-command to sensor
  return error;                     //error=1 SHT1x_DataPinIn case of no response form the sensor
}

//----------------------------------------------------------------------------------
 int8_t SHT1x_read_statusreg(uint8_t *p_value, uint8_t *p_checksum)
//----------------------------------------------------------------------------------
// reads the status register with checksum (8-bit)
{ 
  uint8_t error=0;
  SHT1x_transstart();                   //transmission start
  error=SHT1x_write_byte(SHT1X_STATUS_REG_R); //send command to sensor
  *p_value=SHT1x_read_byte(SHT1X_ACK);        //read status register (8-bit)
  *p_checksum=SHT1x_read_byte(SHT1X_noACK);   //read checksum (8-bit)  
  return error;                     //error=1 SHT1x_DataPinIn case of no response form the sensor
}

//----------------------------------------------------------------------------------
 int8_t SHT1x_write_statusreg(uint8_t *p_value)
//----------------------------------------------------------------------------------
// writes the status register with checksum (8-bit)
{ 
  uint8_t error=0;
  SHT1x_transstart();                   //transmission start
  error+=SHT1x_write_byte(SHT1X_STATUS_REG_W);//send command to sensor
  error+=SHT1x_write_byte(*p_value);    //send value of status register
  return error;                     //error>=1 SHT1x_DataPinIn case of no response form the sensor
}
 							   
//----------------------------------------------------------------------------------
 int8_t SHT1x_measure(uint8_t *p_value, uint8_t *p_checksum, uint8_t mode)
//----------------------------------------------------------------------------------
// makes a measurement (humidity/temperature) with checksum
{ 
  uint8_t error=0;
  uint16_t i;

  SHT1x_transstart();                   //transmission start
  switch(mode){                     //send command to sensor
    case SHT1x_TEMP	: error+=SHT1x_write_byte(SHT1X_MEASURE_TEMP); break;
    case SHT1x_HUMI	: error+=SHT1x_write_byte(SHT1X_MEASURE_HUMI); break;
    default     : break;	 
  }
  for (i=0;i<65535;i++) if(SHT1x_SDA_PIN_IN==0) break; //wait until sensor has finished the measurement
  if(SHT1x_SDA_PIN_IN) error+=1;                // or timeout (~2 sec.) is reached
  * (p_value)=	SHT1x_read_byte(SHT1X_ACK);    //read the first byte (MSB)
  *(p_value+1) =SHT1x_read_byte(SHT1X_ACK);    //read the second byte (LSB)
  *p_checksum =	SHT1x_read_byte(SHT1X_noACK);  //read checksum
  return error;
}
//----------------------------------------------------------------------------------------
 void SHT1x_calc(float *p_humidity ,float *p_temperature)
{  
  float rh=*p_humidity;             // rh:      Humidity [Ticks] 12 Bit 
  float t=*p_temperature;           // t:       Temperature [Ticks] 14 Bit
  float rh_lin;                     // rh_lin:  Humidity linear
  float rh_true;                    // rh_true: Temperature compensated humidity
  float t_C;                        // t_C   :  Temperature [癈]

  t_C=t*0.01 - 39.6;                  //calc. temperature from ticks to [癈]
  rh_lin=C3*rh*rh + C2*rh + C1;     //calc. humidity from ticks to [%RH]
  rh_true=(t_C-25)*(T1+T2*rh)+rh_lin;   //calc. temperature compensated humidity [%RH]
  if(rh_true>100)rh_true=100;       //cut if the value is outside of
  if(rh_true<0.1)rh_true=0.1;       //the physical possible range

  *p_temperature=t_C;               //return temperature [癈]
  *p_humidity=rh_true;              //return humidity[%RH]
}

//--------------------------------------------------------------------
 float SHT1x_calc_dewpoint(float h,float t)
{ 
	float logEx,dew_point;
  logEx=0.66077+7.5*t/(237.3+t)+(log10(h)-2);
  dew_point = (logEx - 0.66077)*237.3/(0.66077+7.5-logEx);
  return dew_point;
}

//----------------------------------------------------------------------------------
bool SHT1x_Periodic_Handle(void)
{ 
	value humi_val,temp_val;
  float dew_point;
  uint8_t error=0,checksum;

	error+=SHT1x_measure((uint8_t*) &humi_val.i,&checksum,SHT1x_HUMI);  //measure humidity
	error+=SHT1x_measure((uint8_t*) &temp_val.i,&checksum,SHT1x_TEMP);  //measure temperature
	
	if(error!=0) 
	{
		SHT1x_connectionreset();                 //SHT1x_DataPinIn case of an error: connection reset
		pDevicePro->printf("SHT1x_connectionreset\r\n");
		return false;
	}
	else
	{
		humi_val.f=(float)humi_val.i;                   //converts integer to float
		temp_val.f=(float)temp_val.i;                   //converts integer to float
		SHT1x_calc(&humi_val.f,&temp_val.f);            //calculate humidity, temperature
		dew_point=SHT1x_calc_dewpoint(humi_val.f,temp_val.f); //calculate dew point
		
//		sys.temperature=temp_val.f;
//		sys.humidity=humi_val.f;
//		sys.dew_point=dew_point;
		pDevicePro->printf("SHT1x  %f  %f  %f\r\n",temp_val.f,humi_val.f,dew_point);
		return true;
	}
	return true;
	//----------wait approx. 0.8s to avoid heating up SHTxx------------------------------      
	//-----------------------------------------------------------------------------------                       
}
