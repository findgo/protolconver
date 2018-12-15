

#include "wintom.h"
#include "usart.h"
#include "queue.h"
#include "msglink.h"
#include "timers.h"

// 定义消息最大存储8条
#define WT_MSG_Q_MAX     8
// 定义应答命令等待超时时间
#define WT_RSP_TIMEOUT    50  // 30ms
// 定义无应答命令转换时间
#define WT_TRANSFER_DELAY_TIMEOUT   15  // 15 ms Transfer delay

typedef struct {
    uint8_t cmd;
    uint8_t dat[];
}Wtreq_t;

// 消息静态句柄缓存
static msgboxstatic_t wtmsgboxHandlebuf = MSGBOX_STATIC_INIT(WT_MSG_Q_MAX);

// 时间句柄
static timer_t * wintomTimerHandle = NULL;
// 静态分配时间结构
static timer_t wintomTimer;

static wintom_rspCallbacks_t *wintom_rspCB = NULL;

static void wintom_TimerCB(void *arg);
static int wintomProcessInRcv(void);
static void wintomPrarseRspInApdu(uint8_t command, uint8_t *apdu, uint16_t apdu_len);

/*********************************************************************
 * @brief       电机运行到任意位置停位
 *
 * @param       moto_no -  moto number
 * @param       devID -  device id
 * @param       pos -  0x00 - 0xff 停位点坐标 = 总行程/256 * 停位点
 *
 * @return     
 */
uint8_t wintom_runtoPos(uint8_t devID, uint8_t pos)
{
    return wintom_request(WT_CMDCODE_GO_POS,WT_MOTO_NO_GENERAL, devID, &pos, 1);
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
uint8_t wintom_setAngle(uint8_t moto_no, uint8_t devID, uint8_t angle)
{
    angle = angle > 100 ? 100 : angle;

    return wintom_request(WT_CMDCODE_GO_POS, moto_no, devID, &angle, 1);
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
uint8_t wintom_setDevID(uint8_t channel, uint8_t moto_no, uint8_t devID)
{
    // channel out of range
    if(channel > WT_CHANNEL_15)
        return FALSE;

    return wintom_request(WT_CMDCODE_SET_DEVID, channel, moto_no, &devID, 1);
}
/*********************************************************************
 * @brief       设置设备ID
 *
 * @param       channel - 通道号 范围0x00 - 0x0f,共16个通道
 *
 * @return     
 */
uint8_t wintom_getSingleDevID(uint8_t channel)
{
    uint8_t size;
    uint8_t checksum = 0;
    Wtreq_t *reqmsg;
    uint8_t *pbuf;

    // channel out of range
    if(channel > WT_CHANNEL_15)
        return FALSE;
    
    if(msgBoxIdle(&wtmsgboxHandlebuf) < 1) // full on message on the list
        return FALSE;
    
    size = 2 + 1 + 1 + 1  + 1; // head(2) + len(1) + cmdcode(1) + para0(1) + checksum
    reqmsg = (Wtreq_t *)msgalloc(sizeof(Wtreq_t) + size);
    if(reqmsg == NULL)
        return FALSE;

    reqmsg->cmd = WT_CMDCODE_GET_DEVID; // ID use store the frame code
    
    pbuf = reqmsg->dat;
    *pbuf++ = WT_PACKET_HEAD_MSB;
    *pbuf++ = WT_PACKET_HEAD_LSB;
    *pbuf++ = 2;

    *pbuf++ = WT_CMDCODE_GET_DEVID;
    checksum += WT_CMDCODE_GET_DEVID;
    *pbuf++ = channel;
    checksum += channel;

    *pbuf++ = checksum;

    if(msgBoxpost(&wtmsgboxHandlebuf, reqmsg) < 0){
        msgdealloc(reqmsg);
        return FALSE;
    }

    return TRUE;
}


static uint8_t checksum(uint8_t *dat,uint16_t length)
{
    uint8_t i,sum;

    for(i = 0, sum = 0; i < length; i++){
        sum += dat[i];
    }

    return sum;
}

uint8_t wintom_request(uint8_t cmdCode, uint8_t para0, uint8_t para1,uint8_t *paraleftbuf, uint8_t paraleftlen)
{
    uint8_t size;
    uint8_t checksum = 0;
    Wtreq_t *reqmsg;
    uint8_t *pbuf;
    
    if(msgBoxIdle(&wtmsgboxHandlebuf) < 1) // full on message on the list
        return FALSE;
    
    size = 2 + 1 + 1 + 1 + 1 + ( paraleftbuf == NULL ? 0 : paraleftlen ) + 1; // head(2) + len(1) + cmdcode(1) + para0(1) + para1(1) + otherpara + checksum
    reqmsg = (Wtreq_t *)msgalloc(sizeof(Wtreq_t) + size);
    if(reqmsg == NULL)
        return FALSE;

    reqmsg->cmd = WT_CMDCODE_GET_DEVID; // ID use store the frame code

    pbuf = reqmsg->dat;
    *pbuf++ = WT_PACKET_HEAD_MSB;
    *pbuf++ = WT_PACKET_HEAD_LSB;
    *pbuf++ = 3 + ( paraleftbuf == NULL ? 0 : paraleftlen );
    
    *pbuf++ = cmdCode;
    checksum += cmdCode;
    *pbuf++ = para0;
    checksum += para0;
    *pbuf++ = para1;
    checksum += para1;
    
    if(paraleftbuf != NULL){
        for(size = 0;size < paraleftlen; size++){
            *pbuf = *paraleftbuf;
            checksum += *paraleftbuf;
            paraleftbuf++;
            pbuf++;
        }
    }

    *pbuf++ = checksum;

    if(msgBoxpost(&wtmsgboxHandlebuf, reqmsg) < 0){
        msgdealloc(reqmsg);
        return FALSE;
    }
    
    return TRUE;
}


#define WT_RCVFSM_HEAD0        0
#define WT_RCVFSM_HEAD1        1
#define WT_RCVFSM_LENGH        2
#define WT_RCVFSM_TOKEN        3
#define WT_RCVFSM_CHECKSUM     4

static uint8_t wt_packetRcvlen;
static uint8_t *wt_packetRcvbufptr = NULL;
static uint8_t wt_packetRcvbytes;

static uint8_t wt_rcvfsm_state = WT_RCVFSM_HEAD0;
static int wintomProcessInRcv(void)
{
    uint8_t ch;
    uint16_t bytesInRxBuffer;
    
    while(WT_RCVBUFLEN())
    {
        WT_RCV(&ch, 1); //read one byte

        switch (wt_rcvfsm_state){
        case WT_RCVFSM_HEAD0:
            if (ch == WT_PACKET_HEAD_MSB)
                wt_rcvfsm_state = WT_RCVFSM_HEAD1;
            break;
          
        case WT_RCVFSM_HEAD1:
            if(ch == WT_PACKET_HEAD_LSB)
                wt_rcvfsm_state = WT_RCVFSM_LENGH;
            break;
            
        case WT_RCVFSM_LENGH:
            wt_packetRcvbufptr = (uint8_t *)mo_malloc(ch);
            
            wt_packetRcvlen = ch;
            wt_packetRcvbytes = 0;
            wt_rcvfsm_state = WT_RCVFSM_TOKEN;
            break;

        case WT_RCVFSM_TOKEN:

            wt_packetRcvbufptr[wt_packetRcvbytes++] = ch;

            bytesInRxBuffer = WT_RCVBUFLEN();

            /* If the remain of the data is there, read them all, otherwise, just read enough */
            if (bytesInRxBuffer <= ( wt_packetRcvlen - wt_packetRcvbytes )){
                if(bytesInRxBuffer != 0){
                    WT_RCV(&wt_packetRcvbufptr[wt_packetRcvbytes], bytesInRxBuffer);
                    wt_packetRcvbytes += bytesInRxBuffer;
                }
            }
            else{
                WT_RCV(&wt_packetRcvbufptr[wt_packetRcvbytes], wt_packetRcvlen - wt_packetRcvbytes);
                wt_packetRcvbytes += wt_packetRcvlen - wt_packetRcvbytes;
            }

            if(wt_packetRcvbytes == wt_packetRcvlen)
                wt_rcvfsm_state = WT_RCVFSM_CHECKSUM;

            break;
            
        case WT_RCVFSM_CHECKSUM:
            
            if(ch == checksum(wt_packetRcvbufptr, wt_packetRcvlen)){
                wintomPrarseRspInApdu(wt_packetRcvbufptr[0], &wt_packetRcvbufptr[1], wt_packetRcvlen - 1);
                mo_free(wt_packetRcvbufptr);
                wt_packetRcvbufptr = NULL;
                
                return TRUE;
            }
            
            mo_free(wt_packetRcvbufptr);
            wt_packetRcvbufptr = NULL;
            
            wt_rcvfsm_state = WT_RCVFSM_HEAD0;
            
            break;
            
        default:          
            wt_rcvfsm_state = WT_RCVFSM_HEAD0;
            break;
        }
    }

    return FALSE;
}

static uint8_t wintom_state = 0;
void wintomTask(void)
{
    Wtreq_t *reqmsg = NULL;
    uint8_t cmd;
    
    if(wintom_state == 0){ // idle ,check any request on the list
        // check any request on the list ? 

        if((reqmsg = msgBoxaccept(&wtmsgboxHandlebuf)) == NULL)
            return;

        WT_SEND(reqmsg->dat, msglen(reqmsg) - 1);
        cmd = reqmsg->cmd;

        msgdealloc(reqmsg);
        
        if(cmd == WT_CMDCODE_GET_POS || cmd == WT_CMDCODE_GET_ANGLE
            || cmd == WT_CMDCODE_GET_DEVID || cmd == WT_CMDCODE_GET_MOTOSTATUS){
            timerStart(wintomTimerHandle, WT_RSP_TIMEOUT); // start a rsp time out
            wintom_state = 1;
        }
        else {
            timerStart(wintomTimerHandle, WT_TRANSFER_DELAY_TIMEOUT); // Transfer delay
            wintom_state = 2;
        }
    }
    else if(wintom_state == 1){ // wait for rsp
        if(wintomProcessInRcv()){      
            timerStop(wintomTimerHandle);
            wintom_state = 0;  
        }
    }
}

static void wintom_TimerCB(void *arg)
{
    if(wintom_state == 1){ // rsp timeout
        if(wt_packetRcvbufptr){
            mo_free(wt_packetRcvbufptr);
            wt_packetRcvbufptr = NULL;
        }
        wt_rcvfsm_state = WT_RCVFSM_HEAD0;
    }
    
    wintom_state = 0;
}

void wintomInit(void)
{
    timerAssign(&wintomTimer,  wintom_TimerCB, NULL);
    SerialDrvInit(COM1, 9600, 0, DRV_PAR_NONE);
}

static void wintomPrarseRspInApdu(uint8_t command, uint8_t *apdu, uint16_t apdu_len)
{
    if(wintom_rspCB == NULL)
        return;
    
    switch (command){
    case WT_CMDCODE_GET_POS:
        if(wintom_rspCB->pfnRspgetPos)
            wintom_rspCB->pfnRspgetPos(apdu[0],apdu[1],apdu[2]);
        break;
        
    case WT_CMDCODE_GET_MOTOSTATUS:
        if(wintom_rspCB->pfnRspgetMotoStatus)
            wintom_rspCB->pfnRspgetMotoStatus(apdu[0],apdu[1],apdu[2]);
        break;
    default:
        break;
    }
}

/*********************************************************************
* @brief  注册wintomrsp信息
*
* @param   rspCB
*
* @return 
*/
void wintom_registerRspCallBack(wintom_rspCallbacks_t *rspCB)
{
    wintom_rspCB = rspCB;
}
