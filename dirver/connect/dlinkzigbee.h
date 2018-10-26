
/*
NOTE：如果不知道波特率，将Band reset 引脚连着到GND，这样可以用默认115200波特率
 UART接口参数
 起始位 1位
 数据位 8位
 停止位 1位
 校验位 无校验 
 波特率默认115200，可被用户设置以下值：
 2400 4800 9600 14400 19200 28800 38400 57600 115200 230400 125000 250000 500000

 帧格式(低8位先发 再发高8位)
 包头    长度(数据 + 4 含端口和地址) 源端口 目的端口 远程地址  数据 包尾
  1 (0xfe)  1                         1       1       2        n   1 (0xff)        
*/


#ifndef __DLINKZIGBEE_H__
#define __DLINKZIGBEE_H__

#include "app_cfg.h"
#include "mserial.h"

#define DL_LOCAL_ADD        0x0000 
#define DL_COORDINATOR      0x0001
#define DL_BROADCAST_ADD    0xffff
#define DL_PAIND_INVALID    0xffff
//!<@  port define 
//  0x00~0x7F 端口由模块内部程序占用
//  0x80~0xFF 端口开放给Uart连接的MCU或者电脑。
/* internal port */
#define DL_PORT_DISCOVER  0x20  // data*100ms

/* NOTE: 只接收远程地址为0x0000的包，信息读取和修改只能通过本模块的UART进行，
不能远程操作  
配置信息要发送重启命令包才会生效，其它形式或丢失*/
#define DL_PORT_BASIC_INFO               0x21
//for read basic info
#define DL_BASIC_INFO_CMD_RD_ADD         0x01
#define DL_BASIC_INFO_CMD_RD_PANID       0x02
#define DL_BASIC_INFO_CMD_RD_CHANNEL     0x03
#define DL_BASIC_INFO_CMD_RD_BAND        0x04
//for write basic info
#define DL_BASIC_INFO_CMD_WR_ADD         0x11    // not set 0x0000 or 0xffff
#define DL_BASIC_INFO_CMD_WR_PANID       0x12    // not set 0xffff
#define DL_BASIC_INFO_CMD_WR_CHANNEL     0x13    // range 0x0b to 0x1a
#define DL_BASIC_INFO_CMD_WR_BAND        0x14    // range 0x00 to 0x0c
#define DL_BASIC_INFO_UPDATE_INFO_RESET  0x10   // 必须发送重启命令才生效

// read command response packet
#define DL_BASIC_INFO_RD_RSP_ADD         0x21
#define DL_BASIC_INFO_RD_RSP_PANID       0x22
#define DL_BASIC_INFO_RD_RSP_CHANNEL     0x23
#define DL_BASIC_INFO_RD_RSP_BAND        0x24
// write command response packet
#define DL_BASIC_INFO_WR_RSP_SUCCESS         0x00
#define DL_BASIC_INFO_WR_RSP_NO_REMOTE_ACESS 0xf0
#define DL_BASIC_INFO_WR_RSP_CMD_ERR         0xf8
#define DL_BASIC_INFO_WR_RSP_LENGTH_ERR      0xf9
#define DL_BASIC_INFO_WR_RSP_VALUE_UNUSEALBE 0xfa

/*error report port*/
#define DL_PORT_ERR_REPORT 0x22
/*RSSI port*/
#define DL_PORT_RSSI    0x23  // dst address not set 0x0000 or 0xffff
/*controlled TTL output*/
#define DL_PORT_TTLPIN4 0x44
#define DL_PORT_TTLPIN5 0x45

#define DL_TTL_CMD_HI 0x11
#define DL_TTL_CMD_LO 0x10
#define DL_TTL_CMD_RD 0x12

/* for user port define */
#define DL_PORT_INFO        0xb0
#define DL_PORT_PASSTHROUGH 0xc0

//@<! packet define value
#define DL_PACKET_HEAD  0xfe
#define DL_PACKET_TAIL  0xff
//@<! packet offset
#define DL_PACKET_HEAD_OFS  0
#define DL_PACKET_LENGH_OFS 1
//!<@ NOTE: escape character
/*传输过程中如果遇到端口号，地址，或者数据部份出现ff,则使用fe fd代替
如果出现FE，则用FE FC来代替。以免传输过程中出现的包头和包尾，使接收收
误判断。在传输中这种替换称为“转义
NOTE:包长度并不受转义影响，转义增加的不计算在内.*/

/* check escape (port address and data )
 * return offset length
*/
#define DL_ESCAPE_CHAR1         0xff  // ff -> fe fd
#define DL_ESCAPE_CHAR2         0xfe  // fe -> fe fc
#define DL_ESCAPE_ASSIT_CHAR1   0xfd
#define DL_ESCAPE_ASSIT_CHAR2   0xfc

/*max buffer packet*/
#define DL_PACKET_BUFF_MAX (63 + 1 + 1) //　head +payload + tail


//for channel
#define DL_CHANNEL_11       0x0b
#define DL_CHANNEL_12       0x0c
#define DL_CHANNEL_13       0x0d
#define DL_CHANNEL_14       0x0e
#define DL_CHANNEL_15       0x0f
#define DL_CHANNEL_16       0x10
#define DL_CHANNEL_17       0x11
#define DL_CHANNEL_18       0x12
#define DL_CHANNEL_19       0x13
#define DL_CHANNEL_20       0x14
#define DL_CHANNEL_21       0x15
#define DL_CHANNEL_22       0x16
#define DL_CHANNEL_23       0x17
#define DL_CHANNEL_24       0x18
#define DL_CHANNEL_25       0x19
#define DL_CHANNEL_26       0x1a
#define DL_CHANNEL_MIN      DL_CHANNEL_11
#define DL_CHANNEL_MAX      DL_CHANNEL_26    

// for bandrare
#define DL_BAND_2400        0x00
#define DL_BAND_4800        0x01
#define DL_BAND_9600        0x02
#define DL_BAND_14400       0x03
#define DL_BAND_19200       0x04
#define DL_BAND_22800       0x05
#define DL_BAND_38400       0x06
#define DL_BAND_57600       0x07
#define DL_BAND_115200      0x08
#define DL_BAND_230400      0x09
#define DL_BAND_125000      0x0a
#define DL_BAND_250000      0x0b
#define DL_BAND_500000      0x0c
#define DL_BANDRATE_MAX     DL_BAND_500000

typedef struct dl_basicInfo_s{
    uint16_t shortaddress;
    uint16_t pandid;
    uint32_t bandrate;  
    uint16_t channel;
    uint8_t ttlpin4;
    uint8_t ttlpin5;
}dl_basicInfo_t;

//@brief  apdu解析回调函数
//src_addr: source address, apdu: pointer adpu, apdu_len: apdu length 
typedef void (*dl_apduParsepfn_t)(uint16_t src_addr, uint8_t *apdu, uint8_t apdu_len);

// define 
#define DL_SEND(buf,len)    Serial_WriteBuf(COM0,buf,len)
#define DL_RCV(buf,len)     Serial_Read(COM0,buf,len)
#define DL_RCVBUFLEN()      SerialRxValidAvail(COM0)

uint8_t dlink_request(uint8_t src_port, uint8_t dst_port, uint16_t dst_address, uint8_t * dat, uint8_t dat_len);

#define dlink_info_request(dst_port,dst_address,dat,dat_len) dlink_request(DL_PORT_INFO,dst_port,dst_address,dat,dat_len)
#define dlink_passthrough_request(dst_address,dat,dat_len) dlink_request(DL_PORT_PASSTHROUGH,DL_PORT_PASSTHROUGH,dst_address,dat,dat_len)

uint8_t dlink_discover_request(uint16_t dst_address,uint8_t time);
uint8_t dlink_rd_local_basic_info(uint8_t cmd);
uint8_t dlink_wr_local_basic_info(uint8_t cmd,uint16_t new_value);
uint8_t dlink_rssi_request(uint16_t dst_address,uint16_t collect_address);
uint8_t dlink_ttlpin4_requset(uint16_t dst_address,uint8_t cmd);
uint8_t dlink_ttlpin5_request(uint16_t dst_address,uint8_t cmd);
dl_basicInfo_t *dlink_getbasicInfo(void);

uint8_t dl_registerParseCallBack(dl_apduParsepfn_t info_cb, dl_apduParsepfn_t passthrough_cb);
void dlinkTask(void);
void dlink_init(void);

#endif
