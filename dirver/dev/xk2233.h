#ifndef __XK2233_H__
#define __XK2233_H__

#include <stdlib.h>
#include <stdbool.h>
#include "bsp_uart5.h"
#include "json_fun.h"

//跟新版的ZET6V2 eep冲突  因此关闭这个功能
#if defined (ZET6V2)||defined(ZET6V60)
#define XK2233_MOD 0

#else
#define XK2233_MOD 0 
#endif

#define XK2233_CMD_LEARNING     0x88
#define XK2233_CMD_LEARNED      0x86
#define XK2233_CMD_SET_MODEL    0x02
#define XK2233_CMD_SW           0x04
#define XK2233_CMD_MODE         0x05
#define XK2233_CMD_TEMPERATURE  0x06
#define XK2233_CMD_WINDSPEED    0x07
#define XK2233_CMD_WINDDIR      0x08
#define XK2233_CMD_CHIPSLEEP    0x01
#define XK2233_CMD_CHIPINITSTART 0xaa
#define XK2233_CMD_CHIPINITEND  0xCC

#define XK2233_MODE_AUTO        0x00    // 自动
#define XK2233_MODE_COOL        0x01    // 制冷
#define XK2233_MODE_DEHUMID     0x02    // 除湿
#define XK2233_MODE_WIND        0x03    // 送风
#define XK2233_MODE_HEAD        0x04    // 制暖

#define XK2233_TEMP_MIN         0x10   // 16度
#define XK2233_TEMP_MAX         0x1e   // 31度

#define XK2233_WINDSPEED_MIN         0x00   
#define XK2233_WINDSPEED_MAX         0x03   // 3档

#define XK2233_WINDDIR_AUTO         0x00   // 自动摆风
#define XK2233_WINDDIR_HAND         0x01   // 手动摆风

#define XK2233_EXCUTE_SUCCESS       0x89
#define XK2233_EXCUTE_FAILED        0xE0

#define XK2233_FIXED_LENGTH          5

#define XK2233_SEND(buf,len)        UART5_TxWrite(buf,len)
#define XK2233_RCV(buf,len)         UART5_RxRead(buf,len)
#define XK2233_RCVBUFLEN()      
#define XK2233UART_TxRead(x,l)      UART5_TxRead(buf,len)
#define XK2233UART_RxWrite(buf,len) UART5_RxWrite(buf,len)

typedef struct 
{
	uint8_t learnmode;
	uint8_t ready;
	uint8_t cmdstatus;//online,offline,err,succ
	uint8_t ircmd;
	
	//空调模式
	uint16_t model; //空调型号
	uint8_t sw;	    //FF:开 00:关
	uint8_t mode;   //00: 自动 01: 制冷 02: 除湿 03: 送风 04: 制暖
	uint8_t temp;   //10H-1EH(16-31℃)
	uint8_t speed;  //00 = 自动 01=1 档位 02=2 档位 03=3 档位
	uint8_t wdir;   //00 = 自动摆风 01 手动摆风
	
	//学校模式
	uint8_t code;
	uint8_t xx1;
	uint8_t xx2;
}XK2233_DATA;

extern XK2233_DATA xk2233data;
extern uint8_t XK2233Status_state;

void Xk2223_request(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4);
//学习代号(0x00-0x64)
void XK2233_sendlearningFun(uint8_t learning_code);
//发射已学习代号(0x00-0x64)
void XK2233_sendLearnedFun(uint8_t learned_code, uint8_t sw);
//部分空调操作
//设定空调型号
 void XK2233_sendsetACModel(uint16_t model);
//设定空调开关
 void XK2233_sendACsw(uint8_t sw);
//设定空调运行模式
 void XK2233_sendACmode(uint8_t mode);
//设定空调温度命令
 void XK2233_sendACtemp(uint8_t temp);
//设定空调风速命令
 void XK2233_sendACWindspeed(uint8_t speed);
//设定空调风向命令
 void XK2233_sendACWindDir(uint8_t winddir);
//休眠
 void XK2233_sendChipsleep(void);
//初始化启动命令
 void XK2233_sendChipInitStart(void);
//结束初始化启动命令
 void XK2233_sendChipInitEnd(void);

void XK2233_Periodic_Handle(void);
bool XK2233_readDeviceStatus(uint8_t* buffer,uint8_t len);

///////////////////json_fun///////////////////
void XK2233_ParseJson(char *src);
void createXK2233StatusJson(XK2233_DATA data,char *messagetype,char *status,uint16_t pid,uint16_t qos);
void subscribe_topic_IR(void);
#endif
