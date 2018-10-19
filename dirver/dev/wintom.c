

#include "wintom.h"
#include "usart.h"
#include "queue.h"
#include "timers.h"

#define WINTOM_QUEUE_ITEM_CAP     5
#define WINTOM_QUEUE_BUFF_SIZE    16

typedef struct {
    uint8_t cmd;
    uint8_t datlen;
    uint8_t dat[WINTOM_QUEUE_BUFF_SIZE];
}wintomItem_t;

static QueueHandle_t wintomSendqueueHandle = NULL;
static QueueStatic_t wintomSendqueue;
static uint8_t wintomSendStorage[WINTOM_QUEUE_ITEM_CAP * sizeof(wintomItem_t)];

static wt_apduParsepfn_t wt_processInApdu_cb = NULL;

static TimerHandle_t wintomTimerHandle = NULL;
static TimerStatic_t wintomTimer;

static void wintom_TimerCB(void *arg);
static int wintomProcessInRcv(void);

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
    wintomItem_t *pItem = NULL;   

    // channel out of range
    if(channel > WT_CHANNEL_15)
        return;
    
    pItem = (wintomItem_t *)queueOnAlloc(wintomSendqueueHandle);
    if(pItem == NULL)
        return;

    pItem->dat[wt_len++] = WT_PACKET_HEAD_MSB;
    pItem->dat[wt_len++] = WT_PACKET_HEAD_LSB;
    pItem->dat[wt_len++] = 2;

    // save command    
    pItem->cmd = WT_CMDCODE_GET_DEVID;

    pItem->dat[wt_len++] = WT_CMDCODE_GET_DEVID;
    checksum += WT_CMDCODE_GET_DEVID;
    pItem->dat[wt_len++] = channel;
    checksum += channel;

    pItem->dat[wt_len++] = checksum;
    pItem->datlen = wt_len;
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
    uint8_t i;
    uint8_t wt_len = 0;
    uint8_t checksum = 0;
    wintomItem_t *pItem = NULL;
    
    pItem = (wintomItem_t *)queueOnAlloc(wintomSendqueueHandle);
    if(pItem == NULL)
        return FALSE;

    pItem->dat[wt_len++] = WT_PACKET_HEAD_MSB;
    pItem->dat[wt_len++] = WT_PACKET_HEAD_LSB;
    pItem->dat[wt_len++] = 3 + ( paraleftbuf == NULL ? 0 : paraleftlen );

    // save command 
    pItem->cmd = cmdCode;
    
    pItem->dat[wt_len++] = cmdCode;
    checksum += cmdCode;
    pItem->dat[wt_len++] = para0;
    checksum += para0;
    pItem->dat[wt_len++] = para1;
    checksum += para1;
    
    if(paraleftbuf != NULL){
        for(i = 0;i < paraleftlen; i++){
            pItem->dat[wt_len] = *paraleftbuf;
            checksum += *paraleftbuf;
            paraleftbuf++;
            wt_len++;
        }
    }

    pItem->dat[wt_len++] = checksum;

    pItem->datlen = wt_len; // store the payload length

    return TRUE;
}


#define WT_RCVFSM_HEAD0        0
#define WT_RCVFSM_HEAD1        1
#define WT_RCVFSM_LENGH        2
#define WT_RCVFSM_TOKEN        3
#define WT_RCVFSM_CHECKSUM     4

static uint8_t wt_packetRcvlen;
static uint8_t wt_packetRcvbuf[WINTOM_QUEUE_BUFF_SIZE];
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
            // out of max packet lengh
            if(ch > WINTOM_QUEUE_BUFF_SIZE){
                wt_rcvfsm_state = WT_RCVFSM_HEAD0;
                return FALSE;
            }
            
            wt_packetRcvlen = ch;
            wt_packetRcvbytes = 0;
            wt_rcvfsm_state = WT_RCVFSM_TOKEN;
            break;

        case WT_RCVFSM_TOKEN:

            wt_packetRcvbuf[wt_packetRcvbytes++] = ch;

            bytesInRxBuffer = WT_RCVBUFLEN();

            /* If the remain of the data is there, read them all, otherwise, just read enough */
            if (bytesInRxBuffer <= ( wt_packetRcvlen - wt_packetRcvbytes )){
                if(bytesInRxBuffer != 0){
                    WT_RCV(&wt_packetRcvbuf[wt_packetRcvbytes], bytesInRxBuffer);
                    wt_packetRcvbytes += bytesInRxBuffer;
                }
            }
            else{
                WT_RCV(&wt_packetRcvbuf[wt_packetRcvbytes], bytesInRxBuffer);
                wt_packetRcvlen += wt_packetRcvlen - wt_packetRcvbytes;
            }

            if(wt_packetRcvbytes == wt_packetRcvlen)
                wt_rcvfsm_state = WT_RCVFSM_CHECKSUM;

            break;
            
        case WT_RCVFSM_CHECKSUM:
            
            if(ch == checksum(wt_packetRcvbuf, wt_packetRcvlen) && wt_processInApdu_cb){
                wt_processInApdu_cb(wt_packetRcvbuf[0], &wt_packetRcvbuf[1], wt_packetRcvlen - 1);
                return TRUE;
            }
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
    wintomItem_t *pItem = NULL;
    
    if(wintom_state == 0){ // idle ,check any request on the list
        pItem = (wintomItem_t *) queueOnPeek(wintomSendqueueHandle);
        if(pItem == NULL) // no request in the list
            return;

        memset(wt_packetRcvbuf, 0, sizeof(wt_packetRcvbuf));
        WT_SEND(pItem->dat, pItem->datlen);
        queuePop(wintomSendqueueHandle, NULL); // pop past the data

        if(pItem->cmd == WT_CMDCODE_GET_POS || pItem->cmd == WT_CMDCODE_GET_ANGLE
            || pItem->cmd == WT_CMDCODE_GET_DEVID || pItem->cmd == WT_CMDCODE_GET_MOTOSTATUS){
            timerStart(wintomTimerHandle, 10); // rsp time out
            wintom_state = 1;
        }
        else {
            timerStart(wintomTimerHandle, 10); // Transfer delay
            wintom_state = 2;
        }

    }
    else if(wintom_state == 1){
        if(wintomProcessInRcv()){      
            timerStop(wintomTimerHandle);
            wintom_state = 0;  
        }
    }
}

static void wintom_TimerCB(void *arg)
{
    if(wintom_state == 1){ // rsp timeout
        wt_rcvfsm_state = WT_RCVFSM_HEAD0;
    }
    
    wintom_state = 0;
}

void wintom_Init(void)
{
    wintomSendqueueHandle = queueAssign(&wintomSendqueue, WINTOM_QUEUE_ITEM_CAP  , sizeof(wintomItem_t)  , wintomSendStorage); 
    wintomTimerHandle = timerAssign(&wintomTimer,  wintom_TimerCB, NULL);

    SerialDrvInit(COM1, 9600, 0, DRV_PAR_NONE);
}

/*********************************************************************
* @brief  注册APDU解析回调函数
*
* @param   info_cb - 消息回调
* @param   passthrough_cb - 透传回调
*
* @return 
*/
uint8_t wintom_registerParseCallBack(wt_apduParsepfn_t processInApdu_cb)
{
    wt_processInApdu_cb = processInApdu_cb;

    return TRUE;
}

