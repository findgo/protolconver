#ifndef _SHT10_H_
#define _SHT10_H_

#include "globals.h"
#include <math.h>
#include "ZNWG.h"



#define SCL GPIO_Pin_9
#define SDA GPIO_Pin_8
#define POWER33 GPIO_Pin_7
#define SHT10_PORT_I2C     GPIOB

#define noACK 0
#define ACK   1
#define STATUS_REG_W 0x06   //000   0011    0
#define STATUS_REG_R 0x07   //000   0011    1
#define MEASURE_TEMP 0x03   //000   0001    1
#define MEASURE_HUMI 0x05   //000   0010    1
#define RESET        0x1e   //000   1111    0


typedef enum {
    ACK                        = 0,
    NO_ACK                     = 1
} SHT2xI2cAck;


typedef union 
{  u16 i;
  float f;
} value;


enum {TEMP,HUMI};
void SHT_POWER33_ON(void);
void Sht_GPIO_Config(void);
void SDA_SET_IN(void);
void SCL_OUT(u8 out);
void SDA_OUT(u8 out);
u8 SDA_READ(void);
float calc_dewpoint(float h,float t);
void calc_sth11(float *p_humidity ,float *p_temperature);
char s_measure(unsigned char *p_value, unsigned char *p_checksum, unsigned char mode);
char s_write_statusreg(unsigned char *p_value);
char s_read_statusreg(unsigned char *p_value, unsigned char *p_checksum);
char s_softreset(void);
void s_connectionreset(void);
void s_transstart(void);
char s_read_byte(unsigned char ack);
char s_write_byte(unsigned char value);

void SHT10_Periodic_Handle(void);
void SHT10_Init(void);
#endif

