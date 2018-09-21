

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

static QueueHandle_t wintomqueueHandle = NULL;
static QueueStatic_t wintomqueue;
static uint8_t wintomStorage[WINTOM_QUEUE_ITEM_CAP * sizeof(wintomItem_t)];

static TimerHandle_t wintomTimerHandle = NULL;
static TimerStatic_t wintomTimer;

static void wintom_TimerCB(void *arg);
static int wintomProcessInRcv(void);
static void wintom_ProcessInApdu(uint8_t command, uint8_t *apdu, uint16_t apdu_len);

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
    
    pItem = (wintomItem_t *)queueOnAlloc(wintomqueueHandle);
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
    
    pItem = (wintomItem_t *)queueOnAlloc(wintomqueueHandle);
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


#define WT_FSM_HEAD0        0
#define WT_FSM_HEAD1        1
#define WT_FSM_LENGH        2
#define WT_FSM_TOKEN        3
#define WT_FSM_CHECKSUM     4

static uint8_t wt_packetlen;
static uint8_t wt_packetbuf[WINTOM_QUEUE_BUFF_SIZE];
static uint8_t wt_packetbytes;

static uint8_t wt_fsm_state = WT_FSM_HEAD0;
static int wintomProcessInRcv(void)
{
    uint8_t ch;

    while(WT_RCVBUFLEN())
    {
        WT_RCV(&ch, 1); //read one byte

        switch (wt_fsm_state){
        case WT_FSM_HEAD0:
            if (ch == WT_PACKET_HEAD_MSB)
                wt_fsm_state = WT_FSM_HEAD1;
            break;
          
        case WT_FSM_HEAD1:
            if(ch == WT_PACKET_HEAD_LSB)
                wt_fsm_state = WT_FSM_HEAD1;
            break;
            
        case WT_FSM_LENGH:
            // out of max packet lengh
            if(ch > WINTOM_QUEUE_BUFF_SIZE){
                wt_fsm_state = WT_FSM_HEAD0;
                return FALSE;
            }
            
            wt_packetlen = ch;
            wt_packetbytes = 0;
            wt_fsm_state = WT_FSM_TOKEN;
            break;

        case WT_FSM_TOKEN:

            wt_packetbuf[wt_packetbytes] = ch;
            ++wt_packetbytes;
            if(wt_packetbytes >=  wt_packetlen)
                wt_fsm_state = WT_FSM_CHECKSUM;
            break;
            
        case WT_FSM_CHECKSUM:
            
            if(ch == checksum(wt_packetbuf, wt_packetlen)){
                wintom_ProcessInApdu(wt_packetbuf[0], &wt_packetbuf[1], wt_packetlen - 1);
                return TRUE;
            }
            wt_fsm_state = WT_FSM_HEAD0;
            
            break;
            
        default:          
            wt_fsm_state = WT_FSM_HEAD0;
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
        pItem = (wintomItem_t *) queueOnPeek(wintomqueueHandle);
        if(pItem == NULL) // no request in the list
            return;

        WT_SEND(pItem->dat, pItem->datlen);
        queuePop(wintomqueueHandle, NULL); // pop past the data

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
        wt_fsm_state = WT_FSM_HEAD0;
    }
    
    wintom_state = 0;
}

void wintom_Init(void)
{
    wintomqueueHandle = queueAssign(&wintomqueue, WINTOM_QUEUE_ITEM_CAP  , sizeof(wintomItem_t)  , wintomStorage); 
    wintomTimerHandle = timerAssign(&wintomTimer,  wintom_TimerCB, NULL);

    SerialDrvInit(COM1, 9600, 0, DRV_PAR_NONE);
}

static void wintom_ProcessInApdu(uint8_t command, uint8_t *apdu, uint16_t apdu_len)
{
          
}
