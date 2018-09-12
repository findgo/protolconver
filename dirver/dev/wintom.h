

/* 广东创明遮阳科技有限公司
    通用RS-485通讯协议
    UART接口参数
     起始位 1位
     数据位 8位
     停止位 1位
     校验位 无校验 
     波特率默认9600
command packet defined
    包头      命令长度 命令 命令参数 .... checksum
   2 (55 aa)  cmdlength      cmd   cmd paran checksum   
   NOTE: 命令长度 = 命令长度后 checksum前的字节数
   起始标记头固定为0x55 0xaa 先发MSB 再发LSB
    */
/*
寄存器位置(控制通道号)： 每台电机内部有16个， 16个地址可相同
组地址(电路机号,高8位)： 0x01-0xff, 0x00为通用地址
NOTE：如果控制器是单路或电机内置控制器，电机路号就是0x00
设备ID地址(低8位)： 0x01-0xff，0x00为通用地址

NOTE：所以控制一台具体的设备由上面三个地址组成
NOTE: 当控制器或电机进入地址设置模式时， 主机可以设置它的地址码， 也可以查询它的地址码。

*/
#ifndef __WINTOM_H__
#define __WINTOM_H__


#define WT_PACKET_HEAD_MSB  0x55
#define WT_PACKET_HEAD_LSB  0xaa

#define WT_MOTO_NO_GENERAL  0x00 // 如果控制器是单路或电机内置控制器时用0x00
#define WT_DEV_ID_GERNERAL  0x00 // 其实相当于广播
// command code
//General
#define WT_CMDCODE_OPEN                 0x01
#define WT_CMDCODE_STOP                 0x02
#define WT_CMDCODE_CLOSE                0x03
#define WT_CMDCODE_GO_POS               0x04
#define WT_CMDCODE_GET_POS              0x05
#define WT_CMDCODE_REVERSING            0x06
#define WT_CMDCODE_CLEAR_TRIP           0x07
#define WT_CMDCODE_HAND_ENABLE          0x08
#define WT_CMDCODE_HAND_DISABLE         0x09
#define WT_CMDCODE_SET_OPEN_LIMIT_POINT 0x0a
#define WT_CMDCODE_SET_CLOSE_LIMIT_POINT 0x0b
#define WT_CMDCODE_SET_MID_POS1         0x0e
#define WT_CMDCODE_SET_MID_POS2         0x0f
#define WT_CMDCODE_RUNTO_MID_POS1       0x0c
#define WT_CMDCODE_RUNTO_MID_POS2       0x0d
#define WT_CMDCODE_LEAF_UP              0x10
#define WT_CMDCODE_LEAF_DOWN            0x11
#define WT_CMDCODE_SET_ANGLE            0x14
#define WT_CMDCODE_GET_ANGLE            0x05
#define WT_CMDCODE_SET_DEVID            0x81
#define WT_CMDCODE_GET_DEVID            0x82
#define WT_CMDCODE_DEL_DEVID            0x83
#define WT_CMDCODE_GET_STATUS           0x84

#define WT_CHANNEL_0    0x00
#define WT_CHANNEL_1    0x01
#define WT_CHANNEL_2    0x02
#define WT_CHANNEL_3    0x03
#define WT_CHANNEL_4    0x04
#define WT_CHANNEL_5    0x05
#define WT_CHANNEL_6    0x06
#define WT_CHANNEL_7    0x07
#define WT_CHANNEL_8    0x08
#define WT_CHANNEL_9    0x09
#define WT_CHANNEL_10   0x0a
#define WT_CHANNEL_11   0x0b
#define WT_CHANNEL_12   0x0c
#define WT_CHANNEL_13   0x0d
#define WT_CHANNEL_14   0x0e
#define WT_CHANNEL_15   0x0f
#define WT_CHANNEL_ALL  0x10

#define WT_MOTO_STATUS_OPEN_LIMIT_POINT_BIT     ((uint8_t)1 << 0)
#define WT_MOTO_STATUS_CLOSE_LIMIT_POINT_BIT    ((uint8_t)1 << 1)
#define WT_MOTO_STATUS_MID_POS1_BIT             ((uint8_t)1 << 2)
#define WT_MOTO_STATUS_MID_POS2_BIT             ((uint8_t)1 << 3)

#define WT_SEND(buf,lengh)
/*
  parabuf：可为NULL
  当parabuf为null是为0;
*/
void wintom_request(uint8_t cmdCode, uint8_t ofs1,uint8_t ofs2,
                        uint8_t * parabuf, uint8_t paralen);

/* General command */
#define wintom_open(moto_num,devID) wintom_request(WT_CMDCODE_OPEN,moto_num,devID,NULL,0)
#define wintom_stop(moto_num,devID) wintom_request(WT_CMDCODE_STOP,moto_num,devID,NULL,0)
#define wintom_close(moto_num,devID) wintom_request(WT_CMDCODE_CLOSE,moto_num,devID,NULL,0)
#define wintom_reversing(moto_num,devID) wintom_request(WT_CMDCODE_REVERSING,moto_num,devID,NULL,0)

/*NOTE:   1.读取窗帘位置操作前必须先设置好开限制点和关限制点,设置方法下有详解。
         2.此功能只对管状电机和开合帘电机有效。
@begin */
void wintom_runtoPos(uint8_t moto_no, uint8_t devID, uint8_t pos);
#define wintom_getPos(moto_num,devID) wintom_request(WT_CMDCODE_GET_POS,moto_num,devID,NULL,0)
#define wintom_cleartrip(devID) wintom_request(WT_CMDCODE_CLEAR_TRIP,WT_MOTO_NO_GENERAL,devID,NULL,0)
/* 
@end */

/*NOTE:   只对开合帘电机有效。
@begin */
#define wintom_handEnable(devID) wintom_request(WT_CMDCODE_HAND_ENABLE,WT_MOTO_NO_GENERAL,devID,NULL,0)
#define wintom_handDisable(devID) wintom_request(WT_CMDCODE_HAND_DISABLE,WT_MOTO_NO_GENERAL,devID,NULL,0)
/*
@end */

/*NOTE:   只对管状电机和开合帘电机有效。
@begin */
#define wintom_setOpenLimitPoint(devID) wintom_request(WT_CMDCODE_SET_OPEN_LIMIT_POINT,WT_MOTO_NO_GENERAL,devID,NULL,0)
#define wintom_setCloseLimitPoint(devID) wintom_request(WT_CMDCODE_SET_CLOSE_LIMIT_POINT,WT_MOTO_NO_GENERAL,devID,NULL,0)
#define wintom_setMidPos1(devID) wintom_request(WT_CMDCODE_SET_MID_POS1,WT_MOTO_NO_GENERAL,devID,NULL,0)
#define wintom_setMidPos2(devID) wintom_request(WT_CMDCODE_SET_MID_POS2,WT_MOTO_NO_GENERAL,devID,NULL,0)
#define wintom_runtoMidPos1(devID) wintom_request(WT_CMDCODE_RUNTO_MID_POS1,WT_MOTO_NO_GENERAL,devID,NULL,0)
#define wintom_runtoMidPos2(devID) wintom_request(WT_CMDCODE_RUNTO_MID_POS2,WT_MOTO_NO_GENERAL,devID,NULL,0)
/*
@end */

/*NOTE:   1.此功能对管状电机、开合帘电机及外置的控制器都有效。
@begin */
#define wintom_leafUp(moto_num,devID) wintom_request(WT_CMDCODE_LEAF_UP,moto_num,devID,NULL,0)
#define wintom_leafDown(moto_num,devID) wintom_request(WT_CMDCODE_LEAF_DOWN,moto_num,devID,NULL,0)
/*
@end */

/* NOTE: 此功能对百叶电机的控制有效。
@begin */
void wintom_setAngle(uint8_t moto_no, uint8_t devID,uint8_t angle);
#define wintom_getAngle(moto_num,devID) wintom_request(WT_CMDCODE_GET_ANGLE,moto_num,devID,NULL,0)
/*
@end */

/* NOTE:创明众联 B 电机及控制器处于地址设置模式时才有用！
@begin */
void wintom_setDevID(uint8_t channel, uint8_t moto_no, uint8_t devID);
#define wintom_getMultiDevID(channel,moto_no) wintom_request(WT_CMDCODE_GET_DEVID,channel,moto_no,NULL,0)
void wintom_getSingleDevID(uint8_t channel);
#define wintom_delDevID(channel,moto_no) wintom_request(WT_CMDCODE_DEL_DEVID,channel,moto_no,NULL,0)
/*
@end */
#define wintom_getMotoStatus(channel,moto_no) wintom_request(WT_CMDCODE_GET_STATUS,channel,moto_no,NULL,0)

#endif

