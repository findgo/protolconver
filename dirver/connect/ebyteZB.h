/*
HEX指令,需要P1.6接地
 UART接口参数
 起始位 1位
 数据位 8位
 停止位 1位
 校验位 无校验 
 波特率默认115200，可被用户设置以下值：
// 2400 4800 9600 14400 19200 38400 43000 57600 76800 115200 
   128000 230400  256000 460800 921600 1000000

 帧格式(低8位先发 再发高8位)
 CMD    长度(LEN)      数据      包尾
  1 (FE/FD/FC)  1     n   1 (0xff)    

 note : 单读PANID失败
*/

#ifndef __EYBTEZB_H__
#define __EYBTEZB_H__


#include "app_cfg.h"
#include "mserial.h"

// user define change
#define EBYTE_NODE_DEVTYPE  EBYTE_DEVTYPE_ROUTE

#define EBYTE_PACKET_DATA_LEN   67
#define EBYTE_PACKET_BUFF_MAX  ( 1 + 1 + EBYTE_PACKET_DATA_LEN + 1 ) // CMD + len + data + tail

#define EBYTE_PACKET_TAIL   0xff

//@<! packet offset
#define EBYTE_PACKET_HEAD_OFS  0
#define EBYTE_PACKET_LENGH_OFS 1

// CMD
#define EBYTE_PACKET_HEAD_CMD_RD        0xfe    // 读取命令
#define EBYTE_PACKET_HEAD_CMD_WR       0xfd     // 配置命令
#define EBYTE_PACKET_HEAD_CMD_DATA      0xfc    // 数据命令
// uart return 
#define EBYTE_PACKET_HEAD_CMD_RD_RSP      0xfb 
#define EBYTE_PACKET_HEAD_CMD_WR_RSP      0xfa

// 以下是特殊返回
#define EBYTE_PACKET_HEAD_CMD_UART_ERR    0xf7   // 
#define EBYTE_PACKET_HEAD_CMD_SPECIFIC    0xff   // 
//
#define EBYTE_PACKET_FORMAT_ERR     0xff    // f7 ff 串口访问时格式错误 
#define EBYTE_PACKET_COOR_SETUP     0xff   // ff ff 协调器设备建立网络提示
#define EBYTE_PACKET_JOININ_NET     0xaa   // ff aa 设备加入网络会提示信息
#define EBYTE_PACKET_NO_NET         0x00   // ff 00 模块设备无网络或失去网络会提示信息

//指令
//读取指令 all in info
#define EBYTE_CMD_DEVTYPE            0x01    // 设备类型
#define EBYTE_CMD_NWKSTATE           0x02    // 读取网络状态
#define EBYTE_CMD_PAINID             0x03    // PANID
#define EBYTE_CMD_NWKKEY             0x04    // 网络秘钥
#define EBYTE_CMD_NWKADDR            0x05    // 只读 短地址
#define EBYTE_CMD_MACADDR            0x06    // 只读 ieee地址
#define EBYTE_CMD_COOR_NWKADDR       0x07    // 只读 父节点短地址
#define EBYTE_CMD_COOR_MACADDR       0x08    // 只读 父节点MAC地址
#define EBYTE_CMD_GROUP              0x09    // 网络组号
#define EBYTE_CMD_CHANNEL            0x0A    // 通信信道
#define EBYTE_CMD_TXPOWER            0x0B    // 发送功率
#define EBYTE_CMD_UARTBAND           0x0C    // 串口波特率
#define EBYTE_CMD_SLEEPTIME         0x0D     // 休眠状态  --- 终端节点有效
#define EBYTE_CMD_ALLINFO            0xFE    // 上面所有信息可用这个命令一次性读

#define EBYTE_CMD_DATA_SAVETIME      0x0E    // 节点数据保存时间(路由和协调器有效)
#define EBYTE_CMD_NWKADDR_ONNET      0x10   // 根据IEEE地址,获得NWK地址
#define EBYTE_CMD_GPIO               0x20    // 远程/本地GPIO输入输出状态
#define EBYTE_CMD_GPIO_LEVEL         0x21    // 远程/本地GPIO电平
#define EBYTE_CMD_PWM                0x22    // 远程/本地PWM状态
#define EBYTE_CMD_ADC                0x23    // 只读 远程/本地ADC状态
#define EBYTE_CMD_RESET              0x12     // 只写
#define EBYTE_CMD_RESET_FACTORY      0x13     // 只写

// for data
#define EBYTE_BROADCAST   0x01    // 广播
#define EBYTE_MULTICAST   0x02    // 组播
#define EBYTE_UNICAST     0x03    // 点播

// for broadcast 
#define EBYTE_BROADCAST_MODE_ALL        0x01 //该消息广播到全网络中所有设备
#define EBYTE_BROADCAST_MODE_RXWHENON   0x02 //该消息广播到只对打开了接收（除休眠模式）的设备
#define EBYTE_BROADCAST_MODE_FF         0x03 //该消息广播到所有全功能设备（路由器和协调器）
// for unicast
#define EBYTE_UNICAST_MODE_OSPF         0x01 //透传方式（无携带信息）
#define EBYTE_UNICAST_MODE_NWK          0x02 //短地址方式（携带信息为短地址）
#define EBYTE_UNICAST_MODE_MAC          0x03 //MAC 地址方式（携带信息为 MAC 地址）


// 设备类型
#define EBYTE_DEVTYPE_COOR      0x00
#define EBYTE_DEVTYPE_ROUTE     0x01
#define EBYTE_DEVTYPE_ENDDEV    0x02
// nwk state
#define EBYTE_NWK_STATE_OFF         0x00
#define EBYTE_NWK_STATE_AVAILABLE   0x01
// for group default 1
#define EBYTE_GROUP_MIN      1
#define EBYTE_GROUP_MAX      99
//for channel
#define EBYTE_CHANNEL_11       0x0b // default
#define EBYTE_CHANNEL_12       0x0c
#define EBYTE_CHANNEL_13       0x0d
#define EBYTE_CHANNEL_14       0x0e
#define EBYTE_CHANNEL_15       0x0f
#define EBYTE_CHANNEL_16       0x10
#define EBYTE_CHANNEL_17       0x11
#define EBYTE_CHANNEL_18       0x12
#define EBYTE_CHANNEL_19       0x13
#define EBYTE_CHANNEL_20       0x14
#define EBYTE_CHANNEL_21       0x15
#define EBYTE_CHANNEL_22       0x16
#define EBYTE_CHANNEL_23       0x17
#define EBYTE_CHANNEL_24       0x18
#define EBYTE_CHANNEL_25       0x19
#define EBYTE_CHANNEL_26       0x1a
#define EBYTE_CHANNEL_MIN      EBYTE_CHANNEL_11
#define EBYTE_CHANNEL_MAX      EBYTE_CHANNEL_26    
//txpower
#define EBYTE_TXPOWER_0       0x00  // -3db
#define EBYTE_TXPOWER_1       0x01  // -1.5db
#define EBYTE_TXPOWER_2       0x02  // 0
#define EBYTE_TXPOWER_3       0x03  // 2.5
#define EBYTE_TXPOWER_4       0x04  // 4.5
//#define EBYTE_TXPOWER_5       0x05  // 无

// for uart bandrate
#define EBYTE_BAND_2400        0x00
#define EBYTE_BAND_4800        0x01
#define EBYTE_BAND_9600        0x02
#define EBYTE_BAND_14400       0x03
#define EBYTE_BAND_19200       0x04
#define EBYTE_BAND_38400       0x05
#define EBYTE_BAND_43000       0x06
#define EBYTE_BAND_57600       0x07
#define EBYTE_BAND_76800       0x08
#define EBYTE_BAND_115200      0x09
#define EBYTE_BAND_128000      0x0a
#define EBYTE_BAND_340400      0x0b
#define EBYTE_BAND_256000      0x0c
#define EBYTE_BAND_460800      0x0d
#define EBYTE_BAND_921600      0x0e
#define EBYTE_BAND_1000000     0x0f
#define EBYTE_BANDRATE_MAX     EBYTE_BAND_1000000

// for gpio pin
#define EBYTE_GPIO_P00          0x00
#define EBYTE_GPIO_P01          0x01
#define EBYTE_GPIO_P02          0x02
#define EBYTE_GPIO_P03          0x03
#define EBYTE_GPIO_P04          0x04
#define EBYTE_GPIO_P05          0x05
#define EBYTE_GPIO_P06          0x06
#define EBYTE_GPIO_P20          0x07
#define EBYTE_GPIO_P21          0x08
#define EBYTE_GPIO_P22          0x09
// gpio in/out state
#define EBYTE_GPIO_STATE_OUT    0x00
#define EBYTE_GPIO_STATE_IN     0x01
// gpio level status
#define EBYTE_GPIO_LEVEL_LOW     0x00
#define EBYTE_GPIO_LEVEL_HIGH    0x01
#define EBYTE_GPIO_LEVEL_TOGGLE  0x02

// for pwm pin 占空比 和周期均是62.5ns的倍数
#define EBYTE_PWM_DUTY1_P02          EBYTE_GPIO_P02
#define EBYTE_PWM_DUTY2_P03          EBYTE_GPIO_P03
#define EBYTE_PWM_DUTY3_P04          EBYTE_GPIO_P04
#define EBYTE_PWM_DUTY4_P05          EBYTE_GPIO_P05
#define EBYTE_PWM_DUTY5_P06          EBYTE_GPIO_P06

//for adc pin 采样值是16位
#define EBYTE_ADC_ENABLE        0x00
#define EBYTE_ADC_DISABLE       0x01
// peripheral address explain, 0 - fff8 为对应设备
#define EBYTE_PERIPHERAL_NWK_LOCAL          0xffff
#define EBYTE_PERIPHERAL_NWK_BROADCAST_ALL  0xfffe
#define EBYTE_PERIPHERAL_NWK_BROADCAST_RXWHENONL 0xfffd
#define EBYTE_PERIPHERAL_NWK_BROADCAST_COOR 0xfffc

#define EBYTE_MAC_LEN        8
#define EBYTE_NWKKEY_LEN     16
#define EBYTE_INFO_LEN       45 
typedef struct {
    uint8_t devType;
    uint8_t nwk_state;
    uint16_t panid;
    
    uint8_t nwkkey[EBYTE_NWKKEY_LEN];
    
    uint16_t nwkaddr;
    uint16_t coor_nwkaddr;
    uint8_t macaddr[EBYTE_MAC_LEN];
    uint8_t coor_macaddr[EBYTE_MAC_LEN];
    
    uint8_t group;
    uint8_t channel;
    uint8_t txpower;
    uint8_t band;
    
    uint8_t sleep_time;  // 单位s, 默认0,休眠关闭          休眠状态,仅对节点有效
    uint8_t data_savetime; // 单位s, 范围1-120 默认30,该节点数据保存时间, 仅对路由器和协调器有效
    uint8_t valid;
    uint8_t dumy1;
}ebyte_info_t;

typedef struct {
    uint8_t devType;
    uint8_t channel;
    uint16_t panid;

    uint8_t group;
    uint8_t txpower;
    uint8_t node_time;  // 终端时, 单位s, 默认0,休眠关闭          休眠状态,仅对节点有效
                        // 路帐或协调器时, 单位s, 范围1-120 默认30,该节点数据保存时间, 仅对路由器和协调器有效
    uint8_t spare; // 预留 
    
    uint8_t nwkkey[EBYTE_NWKKEY_LEN];
}ebyteNvPara_t;

// define uart interface
#define EBYTE_SEND(buf,len)     Serial_WriteBuf(COM0,buf,len)
#define EBYTE_RCV(buf,len)      Serial_Read(COM0,buf,len)
#define EBYTE_RCVBUFLEN()       SerialRxValidAvail(COM0)

#define ebyte_rdAllInfo()               ebyte_cfgReq(EBYTE_PACKET_HEAD_CMD_RD, EBYTE_CMD_ALLINFO, NULL, 0) 
#define ebyte_rdDataSaveTime()          ebyte_cfgReq(EBYTE_PACKET_HEAD_CMD_RD, EBYTE_CMD_DATA_SAVETIME, NULL, 0);
#if 0
#define ebyte_rdNwkAddrOnNet(pmac)     ebyte_cfgReq(EBYTE_PACKET_HEAD_CMD_RD, EBYTE_CMD_NWKADDR_ONNET, pmac, EBYTE_MAC_LEN)
uint8_t ebyte_rdperipheral( uint8_t cmd, uint16_t nwkaddr, uint8_t gpio_pwm_adc);

#define ebyte_wrDevType(devtype)    ebyte_wrGenericsingle(EBYTE_CMD_DEVTYPE, devtype)
uint8_t ebyte_wrPanid(uint16_t panid);
#define ebyte_wrNwkKey(pnwkkey)     ebyte_cfgReq(EBYTE_PACKET_HEAD_CMD_WR, EBYTE_CMD_NWKKEY, pnwkkey, EBYTE_NWKKEY_LEN)
#define ebyte_wrGroup(group)        ebyte_wrGenericsingle(EBYTE_CMD_GROUP, group)
#define ebyte_wrChannel(channel)    ebyte_wrGenericsingle(EBYTE_CMD_CHANNEL, channel)
#define ebyte_wrTxpower(txpower)    ebyte_wrGenericsingle(EBYTE_CMD_TXPOWER, txpower)
#define ebyte_wrUartBand(band)      ebyte_wrGenericsingle(EBYTE_CMD_UARTBAND, band)
#define ebyte_wrSleepTime(time)     ebyte_wrGenericsingle(EBYTE_CMD_SLEEPTIME, time)
uint8_t ebyte_wrGpio(uint8_t cmd, uint16_t nwkaddr, uint8_t gpio, uint8_t val);
// PWM不实现,
#endif

#define ebyte_wrDataSaveTime(time)  ebyte_wrGenericsingle(EBYTE_CMD_DATA_SAVETIME, time)
#define ebyte_reset() ebyte_cfgReq(EBYTE_PACKET_HEAD_CMD_WR, EBYTE_CMD_RESET, NULL, 0)
#define ebyte_resetFactory() ebyte_cfgReq(EBYTE_PACKET_HEAD_CMD_WR, EBYTE_CMD_RESET_FACTORY, NULL, 0)
uint8_t ebyte_wrAllInfo(ebyteNvPara_t *cfg);


//透传传输
uint8_t ebyte_OSPFreq(uint16_t dstaddr, uint8_t *dat, uint8_t dat_len);

void ebyteZBInit(void);
void ebyteZBTask(void);

uint8_t ebyte_NetAvailable(void);
uint16_t ebyte_nwkAddr(void);

// 内部API
uint8_t ebyte_cfgReq(uint8_t Headcmd, uint8_t cmd, uint8_t *premain, uint8_t reaminlen);
uint8_t ebyte_wrGenericsingle(uint8_t cmd, uint8_t value);


#endif
