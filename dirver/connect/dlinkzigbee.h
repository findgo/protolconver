
/*
NOTE：如果不知道波特率，将Band reset 引脚连着到GND，这样可以用默认115200波特率
 UART接口参数
 数据位 8位
 起始位 1位
 停止位 1位
 校验位 无校验 
 波特率默认115200，可被用户设置以下值：
 2400 4800 9600 14400 19200 28800 38400 57600 115200 230400 125000 250000 500000

 帧格式
 包头    长度(数据 + 4 含端口和地址) 源端口 目的端口 远程地址  数据 包尾
  1 (fe)  1                         1       1       2           n   ff        
*/


#ifndef __DLINKZIGBEE_H__
#define __DLINKZIGBEE_H__

#include "app_cfg.h"
#include "mserial.h"

#define DL_LOCAL_ADD 0x0000 
#define DL_BROADCAST_ADD 0xffff
#define DL_PAIND_INVALID 0xffff
#define DL_CHANNEL_MIN  0x0b
#define DL_CHANNEL_MAX  0x1a    
#define DL_BANDRATE_MAX 0x0c
//!<@  port define 
//  0x00~0x7F 端口由模块内部程序占用
//  0x80~0xFF 端口开放给Uart连接的MCU或者电脑。
/* internal port */
#define DL_PORT_DISCOVER  0x20  // data*100ms

/* NOTE: 只接收远程地址为0x0000的包，信息读取和修改只能通过本模块的UART进行，
不能远程操作  
配置信息要发送重启命令包才会生效，其它形式或丢失*/
#define DL_PORT_CONFIGURE 0x21

#define DL_CONFIGURE_CMD_RD_ADD 0x01
#define DL_CONFIGURE_CMD_RD_PANID 0x02
#define DL_CONFIGURE_CMD_RD_CHANNEL 0x03
#define DL_CONFIGURE_CMD_RD_BAND 0x04
#define DL_CONFIGURE_CMD_WR_ADD 0x11 // not set 0x0000 or 0xffff
#define DL_CONFIGURE_CMD_WR_PANID 0x12 // not set 0xffff
#define DL_CONFIGURE_CMD_WR_CHANNEL 0x13 // range 0x0b to 0x1a
#define DL_CONFIGURE_CMD_WR_BAND 0x14   // range 0x00 to 0x0c
#define DL_CONFIGURE_UPDATE_INFO_RESET 0x10

// read command response packet
#define DL_CONFIGURE_RD_RSP_ADD 0x21
#define DL_CONFIGURE_RD_RSP_PANID 0x22
#define DL_CONFIGURE_RD_RSP_CHANNEL 0x23
#define DL_CONFIGURE_RD_RSP_BAND 0x24
// write command response packet
#define DL_CONFIGURE_WR_RSP_SUCCESS 0x00
#define DL_CONFIGURE_WR_RSP_NO_REMOTE_ACESS 0xf0
#define DL_CONFIGURE_WR_RSP_CMD_ERR 0xf8
#define DL_CONFIGURE_WR_RSP_LENGTH_ERR 0xf9
#define DL_CONFIGURE_WR_RSP_VALUE_UNUSEALBE 0xfa

/*error report port*/
#define DL_PORT_ERR_REPORT 0x22
/*RSSI port*/
#define DL_PORT_RSSI 0x23  // dst address not set 0x0000 or 0xffff
/*controlled TTL output*/
#define DL_PORT_TTLPIN4 0x44
#define DL_PORT_TTLPIN5 0x45

#define DL_TTL_CMD_HI 0x11
#define DL_TTL_CMD_LO 0x10
#define DL_TTL_CMD_RD 0x12

/* for user port define */
#define DL_PORT_INFO 0xb0
#define DL_PORT_PASSTHROUGH 0xc0

//@<! packet define value
#define DL_PACKET_HEAD  0xfe
#define DL_PACKET_TAIL  0xff
//@<! packet offset
#define DL_PACKET_HEAD_OFS 0
#define DL_PACKET_LENGH_OFS 1
//!<@ NOTE: escape character
/*传输过程中如果遇到端口号，地址，或者数据部份出现ff,则使用fe fd代替
如果出现FE，则用FE FC来代替。以免传输过程中出现的包头和包尾，使接收收
误判断。在传输中这种替换称为“转义
NOTE:包长度并不受转义影响，转义增加的不计算在内.*/

/*max buffer packet*/
#define DL_PACKET_BUFF_MAX 63

// define 
#define DL_SEND(buf,len)    Serial_WriteBuf(COM0,buf,len)
#define DL_RCV(buf,len)     Serial_Read(COM0,buf,len)
#define DL_RCVBUFLEN()      serialRxValidAvail(COM0)

void dlink_request(uint8_t src_port, uint8_t dst_port, uint16_t dst_address, uint8_t * dat, uint8_t dat_len);

#define dlink_request_info(dst_port,dst_address,dat,dat_len) dlink_request(DL_PORT_INFO,dst_port,dst_address,dat,dat_len)
#define dlink_request_passthrough(dst_address,dat,dat_len) dlink_request(DL_PORT_PASSTHROUGH,DL_PORT_PASSTHROUGH,dst_address,dat,dat_len)

void dlink_discover(uint16_t dst_address,uint8_t time);
void dlink_rd_basic_info(uint8_t cmd);
void dlink_wr_basic_info(uint8_t cmd,uint16_t new_value);
void dlink_rssi(uint16_t dst_address,uint16_t collect_address);
void dlink_ttlpin4(uint16_t dst_address,uint8_t cmd);
void dlink_ttlpin5(uint16_t dst_address,uint8_t cmd);

void dlink_period_task(void);
void dlink_init(void);

#endif
