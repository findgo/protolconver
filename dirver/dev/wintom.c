

#include "wintom.h"

static uint8_t wt_txbuf[16];


/*********************************************************************
 * @brief       wintom request 
 *
 * @param       cmdCode -  command code
 * @param       para0 -  parameter 0
 * @param       para1 - parameter 0
 * @param       paraleftbuf -  pointer to the parameter left, can be NULL
 * @param       paraleftlen -  parameter left length
 *
 * @return     
 */
uint8_t wintom_request(uint8_t cmdCode, uint8_t para0, uint8_t para1,uint8_t *paraleftbuf, uint8_t paraleftlen)
{
    uint8_t i;
    uint8_t wt_len = 0;
    uint8_t checksum = 0;
    
    wt_txbuf[wt_len++] = WT_PACKET_HEAD_MSB;
    wt_txbuf[wt_len++] = WT_PACKET_HEAD_LSB;
    wt_txbuf[wt_len++] = 3 + ( paraleftbuf == NULL ? 0 : paraleftlen );
    
    wt_txbuf[wt_len++] = cmdCode;
    checksum += cmdCode;
    wt_txbuf[wt_len++] = para0;
    checksum += para0;
    wt_txbuf[wt_len++] = para1;
    checksum += para1;
    
    if(paraleftbuf != NULL){
        for(i = 0;i < paraleftlen; i++){
            wt_txbuf[wt_len] = *paraleftbuf;
            checksum += *paraleftbuf;
            paraleftbuf++;
            wt_len++;
        }
    }

    wt_txbuf[wt_len++] = checksum;

    WT_SEND(wt_txbuf,wt_len);

    return TRUE;
}

/*********************************************************************
 * @brief       电机运行到任意位置停位
 *
 * @param       moto_no -  moto number
 * @param       devID -  device id
 * @param       pos -  0x00 - 0xff 停位点坐标 = 总行程/256 * 停位点
 *
 * @return     
 */
void wintom_runtoPos(uint8_t devID, uint8_t pos)
{
    uint8_t tpos = pos;

    wintom_request(WT_CMDCODE_GO_POS,WT_MOTO_NO_GENERAL, devID, &tpos, 1);
}
/*********************************************************************
 * @brief       设置角度
 *
 * @param       moto_no -  moto number
 * @param       devID -  device id
 * @param       angle -  0 - 100, 系统对角度0-180度分为100份,所以范围为0x00 - 0x64
 *
 * @return     
 */
void wintom_setAngle(uint8_t moto_no, uint8_t devID, uint8_t angle)
{
    uint8_t tangle = angle;

    tangle = angle > 100 ? 100 : angle;

    wintom_request(WT_CMDCODE_GO_POS, moto_no, devID, &tangle, 1);
}
/*  special command */
/*********************************************************************
 * @brief       设置设备ID
 *
 * @param       channel - 通道号 范围0x00 - 0x0f,共16个通道
 * @param       moto_no -  moto number
 * @param       devID -  device id
 *
 * @return     
 */
void wintom_setDevID(uint8_t channel, uint8_t moto_no, uint8_t devID)
{
    uint8_t tdevID = devID;

    // channel out of range
    if(channel > WT_CHANNEL_15)
        return;

    wintom_request(WT_CMDCODE_SET_DEVID, channel, moto_no, &tdevID, 1);
}
/*********************************************************************
 * @brief       设置设备ID
 *
 * @param       channel - 通道号 范围0x00 - 0x0f,共16个通道
 *
 * @return     
 */
void wintom_getSingleDevID(uint8_t channel)
{
    uint8_t wt_len = 0;
    uint8_t checksum = 0;
    
    // channel out of range
    if(channel > WT_CHANNEL_15)
        return;

    wt_txbuf[wt_len++] = WT_PACKET_HEAD_MSB;
    wt_txbuf[wt_len++] = WT_PACKET_HEAD_LSB;
    wt_txbuf[wt_len++] = 2;
    
    wt_txbuf[wt_len++] = WT_CMDCODE_GET_DEVID;
    checksum += WT_CMDCODE_GET_DEVID;
    wt_txbuf[wt_len++] = channel;
    checksum += channel;

    wt_txbuf[wt_len++] = checksum;

    WT_SEND(wt_txbuf,wt_len);
}

void wintom_parsing()
{
    
}
