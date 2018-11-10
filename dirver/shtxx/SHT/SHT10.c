
#include "SHT10.h"

void SHT10_Init(void)
{
	unsigned char sat,checksum;
	pDevicePro->printf("SHT10_Init...\r\n");
	SHT_POWER33_ON();
	Sht_GPIO_Config();
	s_connectionreset();
	
	if(s_read_statusreg(&sat,&checksum))
	pDevicePro->printf("READ STATUSREG FAILED\r\n");
	else
//	pDevicePro->printf("original statusreg is %x\n",sat);
		sat=0x00;
	if( s_write_statusreg(&sat)) pDevicePro->printf("Commond errr\r\n");
	s_read_statusreg(&sat,&checksum);
	if(sat == 0)	
	{
		pDevicePro->printf("SHT10_Init初始化完成\r\n\r\n");
	}
	else 
	{
		pDevicePro->printf("SHT10_Init初始化失败\r\n\r\n");
	}

}

void Sht_GPIO_Config(void)
{
   
  GPIO_InitTypeDef  GPIO_InitStructure;
	/* 使能与 I2C1 有关的时钟 */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
    
  /* PB6-I2C1_SCL、PB7-I2C1_SDA*/
  GPIO_InitStructure.GPIO_Pin =  SDA | SCL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	       // 开漏输出
  GPIO_Init(SHT10_PORT_I2C, &GPIO_InitStructure);
}

void SHT_POWER33_ON(void)
{
	 GPIO_InitTypeDef  GPIO_InitStructure;
   GPIO_InitStructure.GPIO_Pin = POWER33;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	       // 开漏输出
   GPIO_Init(SHT10_PORT_I2C, &GPIO_InitStructure);	
	 GPIO_SetBits(SHT10_PORT_I2C,POWER33);
		delay_ms(100);
}

void SDA_SET_OUT(void)
{
	/* 使能与 I2C1 有关的时钟 */
    
  /* PB6-I2C1_SCL、PB7-I2C1_SDA*/
  GPIO_InitTypeDef  GPIO_InitStructure;
   GPIO_InitStructure.GPIO_Pin = SDA;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	       // 开漏输出
   GPIO_Init(SHT10_PORT_I2C, &GPIO_InitStructure);	
}
void SDA_SET_IN(void)
{
   GPIO_InitTypeDef  GPIO_InitStructure;
   GPIO_InitStructure.GPIO_Pin = SDA;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	       // 开漏输出
   GPIO_Init(SHT10_PORT_I2C, &GPIO_InitStructure);	
}
void SCL_OUT(u8 out)
{

    if (out)
	GPIO_SetBits(SHT10_PORT_I2C,SCL);
	else
	GPIO_ResetBits(SHT10_PORT_I2C,SCL);
}
void SDA_OUT(u8 out)
{
    SDA_SET_OUT();
    if (out)
	GPIO_SetBits(SHT10_PORT_I2C,SDA);
	else
	GPIO_ResetBits(SHT10_PORT_I2C,SDA);
}
u8 SDA_READ(void)
{
	SDA_SET_IN();
	return GPIO_ReadInputDataBit(SHT10_PORT_I2C,SDA);
	
}

//----------------------------------------------------------------------------------
char s_write_byte(unsigned char value)
//----------------------------------------------------------------------------------
// writes a byte on the Sensibus and checks the acknowledge 
{ 
  unsigned char i,error=0;  
  for (i=0x80;i>0;i/=2)             //shift bit for masking
  { 
  	if (i & value)
    SDA_OUT(1);          //masking value with i , write to SENSI-BUS
    else SDA_OUT(0);                        
    delay_us(2);                      //observe setup time
    SCL_OUT(1);                          //clk for SENSI-BUS
    delay_us(6);        //pulswith approx. 5 us  	
    SCL_OUT(0);
    delay_us(2);                         //observe hold time
  }
  SDA_OUT(1);                          //release DATA-line
  delay_us(2);                         //observe setup time
  SCL_OUT(1);                          //clk #9 for ack 
  error=SDA_READ();                      //check ack (DATA will be pulled down by SHT11)
  SCL_OUT(0);       
  return error;                     //error=1 in case of no acknowledge
}

//----------------------------------------------------------------------------------
char s_read_byte(unsigned char ack)
//----------------------------------------------------------------------------------
// reads a byte form the Sensibus and gives an acknowledge in case of "ack=1" 
{ 
  unsigned char i,val=0;
  SDA_OUT(1);                           //release DATA-line
  for (i=0x80;i>0;i=i>>1)             //shift bit for masking
  { 
    SCL_OUT(1);                        //clk for SENSI-BUS
    if (SDA_READ()) val=(val | i);        //read bit  
    SCL_OUT(0);  					 
  }
  SDA_OUT(!ack);                        //in case of "ack==1" pull down DATA-Line
  delay_us(2);                          //observe setup time
  SCL_OUT(1);                            //clk #9 for ack
  delay_us(6);          //pulswith approx. 5 us 
  SCL_OUT(0);
  delay_us(2);                         //observe hold time						    
  SDA_OUT(1);                          //release DATA-line
  return val;
}


//----------------------------------------------------------------------------------
void s_transstart(void)
//----------------------------------------------------------------------------------
// generates a transmission start 
//       _____         ________
// DATA:      |_______|
//           ___     ___
// SCK : ___|   |___|   |______
{  
   SDA_OUT(1); SCL_OUT(0);                   //Initial state
   delay_us(2);
   SCL_OUT(1);
   delay_us(2);
   SDA_OUT(0);
   delay_us(2);
   SCL_OUT(0);  
   delay_us(6);
   SCL_OUT(1);
   delay_us(2);
   SDA_OUT(1);		   
   delay_us(2);
   SCL_OUT(0);		   
}

//----------------------------------------------------------------------------------
void s_connectionreset(void)
//----------------------------------------------------------------------------------
// communication reset: DATA-line=1 and at least 9 SCK cycles followed by transstart
//       _____________________________________________________         ________
// DATA:                                                      |_______|
//          _    _    _    _    _    _    _    _    _        ___     ___
// SCK : __| |__| |__| |__| |__| |__| |__| |__| |__| |______|   |___|   |______
{  
  unsigned char i; 
  SDA_OUT(1); 
  SCL_OUT(0);                    //Initial state
  for(i=0;i<9;i++)                  //9 SCK cycles
  { SCL_OUT(1);
    SCL_OUT(0);
  }
  s_transstart();                   //transmission start
}

//----------------------------------------------------------------------------------
char s_softreset(void)
//----------------------------------------------------------------------------------
// resets the sensor by a softreset 
{ 
  unsigned char error=0;  
  s_connectionreset();              //reset communication
  error+=s_write_byte(RESET);       //send RESET-command to sensor
  return error;                     //error=1 in case of no response form the sensor
}

//----------------------------------------------------------------------------------
char s_read_statusreg(unsigned char *p_value, unsigned char *p_checksum)
//----------------------------------------------------------------------------------
// reads the status register with checksum (8-bit)
{ 
  unsigned char error=0;
  s_transstart();                   //transmission start
  error=s_write_byte(STATUS_REG_R); //send command to sensor
  *p_value=s_read_byte(ACK);        //read status register (8-bit)
  *p_checksum=s_read_byte(noACK);   //read checksum (8-bit)  
  return error;                     //error=1 in case of no response form the sensor
}

//----------------------------------------------------------------------------------
char s_write_statusreg(unsigned char *p_value)
//----------------------------------------------------------------------------------
// writes the status register with checksum (8-bit)
{ 
  unsigned char error=0;
  s_transstart();                   //transmission start
  error+=s_write_byte(STATUS_REG_W);//send command to sensor
  error+=s_write_byte(*p_value);    //send value of status register
  return error;                     //error>=1 in case of no response form the sensor
}

//----------------------------------------------------------------------------------
char s_measure(unsigned char *p_value, unsigned char *p_checksum, unsigned char mode)
//----------------------------------------------------------------------------------
// makes a measurement (humidity/temperature) with checksum
{ 
  unsigned char error=0;
  unsigned int i;

  s_transstart();                   //transmission start
  switch(mode){                     //send command to sensor
    case TEMP	: error+=s_write_byte(MEASURE_TEMP); break;
    case HUMI	: error+=s_write_byte(MEASURE_HUMI); break;
    default     : break;	 
  }
//  if (mode==TEMP)
//  {
//  for (i=0;i<60000;i++)
//  
//  if(SDA_READ()==0) break; //wait until sensor has finished the measurement
//  if(SDA_READ()) error+=1;               // or timeout (~2 sec.) is reached
//  *(p_value+1)  =s_read_byte(ACK);    //read the first byte (MSB)
//  //printf("p_value is %x\r",*p_value);
//  *(p_value)=s_read_byte(ACK);
//  //printf("p_value+1 is %x\r",*(p_value+1));    //read the second byte (LSB)
//  *p_checksum =s_read_byte(noACK);  //read checksum
//  return error;	
//  }
//
//  else
//  {

  for (i=0;i<60000;i++)
   
  if(SDA_READ()==0) break; //wait until sensor has finished the measurement
  if(SDA_READ()) error+=1;               // or timeout (~2 sec.) is reached
  *(p_value+1)  =s_read_byte(ACK);    //read the first byte (MSB)
 // printf("p_value is %x",*p_value);
  *(p_value)=s_read_byte(ACK);
 // printf("p_value+1 is %x",*(p_value+1));    //read the second byte (LSB)
  *p_checksum =s_read_byte(noACK);  //read checksum
  return error;
//  }
}
void calc_sth10(float *p_humidity ,float *p_temperature)
//----------------------------------------------------------------------------------------
// calculates temperature [癈] and humidity [%RH] 
// input :  humi [Ticks] (12 bit) 
//          temp [Ticks] (14 bit)
// output:  humi [%RH]
//          temp [癈]
{  const float C1=-2.0468;           // for 12 Bit RH
  const float C2=+0.0367;           // for 12 Bit RH
  const float C3=-0.0000015955;     // for 12 Bit RH
  const float T1=+0.01;             // for 12 Bit RH
  const float T2=+0.00008;          // for 12 Bit RH	

  float rh=*p_humidity;             // rh:      Humidity [Ticks] 12 Bit 
  float t=*p_temperature;           // t:       Temperature [Ticks] 14 Bit
  float rh_lin;                     // rh_lin:  Humidity linear
  float rh_true;                    // rh_true: Temperature compensated humidity
  float t_C;                        // t_C   :  Temperature [癈]

  t_C=t*0.01 - 39.7;                //calc. temperature [癈] from 14 bit temp. ticks @ 5V
  rh_lin=C3*rh*rh + C2*rh + C1;     //calc. humidity from ticks to [%RH]
  rh_true=(t_C-25)*(T1+T2*rh)+rh_lin;   //calc. temperature compensated humidity [%RH]
  if(rh_true>100)rh_true=100;       //cut if the value is outside of
  if(rh_true<0.1)rh_true=0.1;       //the physical possible range

  *p_temperature=t_C;               //return temperature [癈]
  *p_humidity=rh_true;              //return humidity[%RH]
}

//----------------------------------------------------------------------------------------
void calc_sth11(float *p_humidity ,float *p_temperature)
//----------------------------------------------------------------------------------------
// calculates temperature [癈] and humidity [%RH] 
// input :  humi [Ticks] (12 bit) 
//          temp [Ticks] (14 bit)
// output:  humi [%RH]
//          temp [癈]
{ const float C1=-4.0;           // for 12 Bit RH
  const float C2=+0.0405;           // for 12 Bit RH
  const float C3=-0.0000028;     // for 12 Bit RH
  const float T1=+0.01;             // for 12 Bit RH
  const float T2=+0.00008;          // for 12 Bit RH	
	const float d1 = 39.6;						//5V   ==  40.1    3V == 39.6

  float rh=*p_humidity;             // rh:      Humidity [Ticks] 12 Bit 
  float t=*p_temperature;           // t:       Temperature [Ticks] 14 Bit
  float rh_lin;                     // rh_lin:  Humidity linear
  float rh_true;                    // rh_true: Temperature compensated humidity
  float t_C;                        // t_C   :  Temperature [癈]

  t_C=t*0.01 - d1;                //calc. temperature [癈] from 14 bit temp. ticks @ 5V
  rh_lin=C3*rh*rh + C2*rh + C1;     //calc. humidity from ticks to [%RH]
  rh_true=(t_C-25)*(T1+T2*rh)+rh_lin;   //calc. temperature compensated humidity [%RH]
//  if(rh_true>100)rh_true=100;       //cut if the value is outside of
//  if(rh_true<0.1)rh_true=0.1;       //the physical possible range

  *p_temperature=t_C;               //return temperature [癈]
  *p_humidity=rh_true;              //return humidity[%RH]
}


//--------------------------------------------------------------------
float calc_dewpoint(float h,float t)
//--------------------------------------------------------------------
// calculates dew point
// input:   humidity [%RH], temperature [癈]
// output:  dew point [癈]
{ float k,dew_point ;
  
  k = (log10(h)-2)/0.4343 + (17.62*t)/(243.12+t);
  dew_point = 243.12*k/(17.62-k);
  return dew_point;
}
/*
 float calc_dewpoint(float h,float t)
{ 
	float logEx,dew_point;
  logEx=0.66077+7.5*t/(237.3+t)+(log10(h)-2);
  dew_point = (logEx - 0.66077)*237.3/(0.66077+7.5-logEx);
  return dew_point;
}
*/
void SHT10_Periodic_Handle(void)
{ 
	static uint8_t SHT10_Init_ready_state = 0;
	if(SHT10_Init_ready_state != 1)
	{ 	
//		SHT2x_Init();
		SHT10_Init();
		SHT10_Init_ready_state =1;
		return;
	}


	unsigned char error,checksum;
	value humi_val,temp_val;
	error=0;
	error+=s_measure((unsigned char*) &humi_val.i,&checksum,HUMI);  //measure humidity
	// printf("HUMI DATA IS %x \n",humi_val.i);
	error+=s_measure((unsigned char*) &temp_val.i,&checksum,TEMP);  //measure temperature
	// printf("TEMP DATA IS %x \n",temp_val.i);
	if(error!=0)	 
	{
		s_connectionreset();                 //in case of an error: connection reset
		pDevicePro->printf("读写SHT10,失败，重新初始化STH10\r\n");	
	}
	else
	{ 
		humi_val.f=(float)humi_val.i;                   //converts integer to float
		temp_val.f=(float)temp_val.i;                   //converts integer to float
	 
		calc_sth10(&humi_val.f,&temp_val.f);            //calculate humidity, temperature
		 
		sensor_info.hum = humi_val.f;
		sensor_info.temp = temp_val.f;
		sensor_info.dew_point = calc_dewpoint(sensor_info.hum,sensor_info.temp);
		#ifdef XZLDEBUG
		if(1)DPRINTF("\ttemp:%5.1f℃ humi:%5.1f%% dew_point:%5.1f℃\r\n",sensor_info.temp,sensor_info.hum,sensor_info.dew_point);
		#endif
	 
	}
}
