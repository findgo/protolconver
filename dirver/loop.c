
#include "loop.h"

//for driver
#include "wintom.h"
#include "mleds.h"
#include "hal_key.h"
#include "systick.h"

#include "memalloc.h"
#include "timers.h"
#include "event_groups.h"
#include "log.h"

#include "ltl.h"
#include "ltl_genattr.h"

#include "curtain.h"
#include "nwk.h"

#include "mt_npi.h"


#if configSUPPORT_TASKS_EVENT > 0
#include "msglink.h"
// 事件触发,为未来低功耗节能
static const pTaskFn_t tasksArr[] =
{
    NULL
};
static const uint8_t tasksCnt = sizeof(tasksArr) / sizeof(tasksArr[0]);
static uint16_t *tasksEvents;
#endif
// 轮询
static const pPollFn_t pollsArr[] =
{
    nwkTask,
    timerTask, 
    wintomTask,
    npiTask,
    keyTask
};
static const uint8_t poolsCnt = sizeof(pollsArr) / sizeof(pollsArr[0]);


static void logInit(void);

void loop_init_System(void)
{
#if configSUPPORT_TASKS_EVENT > 0
    uint8_t taskID = 0;
    
    tasksEvents = (uint16_t *)mo_malloc( sizeof( uint16_t ) * tasksCnt);
    memset( tasksEvents, 0, (sizeof( uint16_t ) * tasksCnt));
#endif

    Systick_Configuration();
    logInit();
    mo_logln(INFO,"loop_init_System init begin!");
    
// driver init 
    // led
    halledInit();
    mledInit();
    // key
    halkeyInit();
    
    nwkInit();
    // curtain    
    wintomInit();
   
    ltl_GeneralBasicAttriInit();
    curtainAttriInit();
    // blink 3 ,show no on net
    mledset(MLED_1, MLED_MODE_FLASH);

    mo_logln(INFO,"loop_init_System init end, and start!");
}
    
void loop_Run_System(void)
{
    uint8_t idx = 0;
    uint16_t events;

    for(idx = 0; idx < poolsCnt; ++idx){
        pollsArr[idx]();
    }
    
#if configSUPPORT_TASKS_EVENT > 0
    // 移植的TI OSAL
    for(idx = 0;idx < tasksCnt; ++idx)
    {
        if (tasksEvents[idx]){ // Task is highest priority that is ready.
            break;
        }
    }
    
    if (idx < tasksCnt) {
    
        taskENTER_CRITICAL();
        events = tasksEvents[idx];
        tasksEvents[idx] = 0;  // Clear the Events for this task.
        taskEXIT_CRITICAL();
    
        events = (tasksArr[idx])( idx, events );
    
        taskENTER_CRITICAL();
        tasksEvents[idx] |= events;  // Add back unprocessed events to the current task.
        taskEXIT_CRITICAL();
    }
#if defined( POWER_SAVING )
    else { // Complete pass through all task events with no activity?
        powerconserve();  // Put the processor/system into sleep
    }
#endif
#endif

}
#if configSUPPORT_TASKS_EVENT > 0
// task id and qmsg map message
typedef struct {
    uint8_t task_id;
    msg_q_t q_taskmsghead;
}taskmsg_mapInfo_t; 
static taskmsg_mapInfo_t *taskmap_search(uint8_t task_id);

static msg_q_t taskmsgq_head;

uint8_t tasks_setEvent(uint8_t task_id, uint16_t event_flag)
{
    isrSaveCriticial_status_Variable;

    if ( task_id < tasksCnt ){
        isrENTER_CRITICAL();
        tasksEvents[task_id] |= event_flag; 
        isrEXIT_CRITICAL();
        return TRUE;
    }
    
    return FALSE;
}

uint8_t tasks_clearEvent(uint8_t task_id, uint16_t event_flag)
{
    isrSaveCriticial_status_Variable;

    if ( task_id < tasksCnt ){
        isrENTER_CRITICAL();
        tasksEvents[task_id] &= ~event_flag; 
        isrEXIT_CRITICAL();
        return TRUE;
    }
    
    return FALSE;
}

int task_msg_assign(uint8_t task_id)
{
    taskmsg_mapInfo_t *mapinfo;

    mapinfo = taskmap_search(task_id);

    if(mapinfo == NULL){
        if ((mapinfo = msgalloc(sizeof(taskmsg_mapInfo_t))) == NULL)
            return MSG_INVALID_POINTER;

        mapinfo->task_id = task_id;
        mapinfo->q_taskmsghead = NULL;

        msgQputFront(&taskmsgq_head, mapinfo);
    }
    
    return MSG_SUCCESS;
}

// not find it : NULL,else find it 
static taskmsg_mapInfo_t *taskmap_search(uint8_t task_id)
{
    taskmsg_mapInfo_t *srch;

    msgQ_for_each_msg(&taskmsgq_head, srch){
        if(srch->task_id == task_id){ // find it 
            break;
        }
    }

    return srch;
}

int task_msg_Genericput(uint8_t dest_taskid, void *msg_ptr, uint8_t isfront)
{
    taskmsg_mapInfo_t *mapinfo;

    if(dest_taskid >= tasksCnt){
        return MSG_INVALID_TASK;
    }

    mapinfo = taskmap_search(dest_taskid);
    if(!mapinfo){
        return MSG_TASK_MSG_UNINT;
    }

    msgQGenericput(&(mapinfo->q_taskmsghead), msg_ptr, isfront);
    tasks_setEvent(dest_taskid, SYS_EVENT_MSG);
    
    return MSG_SUCCESS;
}
int task_msg_put(uint8_t dest_taskid, void *msg_ptr)
{
    return task_msg_Genericput(dest_taskid, msg_ptr, FALSE);
}
int task_msg_putFront(uint8_t dest_taskid, void *msg_ptr)
{
    return task_msg_Genericput(dest_taskid, msg_ptr, TRUE);
}
void *tasks_msg_accept(uint8_t task_id)
{
    taskmsg_mapInfo_t *mapinfo;

    if(task_id >= tasksCnt){
        return NULL;
    }

    mapinfo = taskmap_search(task_id);
    if(!mapinfo){
        return NULL;
    }

    return msgQpop(&(mapinfo->q_taskmsghead));
}

void *tasks_msg_peek(uint8_t task_id)
{
    taskmsg_mapInfo_t *mapinfo;

    if(task_id >= tasksCnt){
        return NULL;
    }

    mapinfo = taskmap_search(task_id);
    if(!mapinfo){
        return NULL;
    }

    return msgQpeek(&(mapinfo->q_taskmsghead));
}

#endif


static void logInit(void)
{
    SerialDrvInit(COM2, 115200, 8, DRV_PAR_NONE);
    mo_log_set_max_logger_level(LOG_LEVEL_DEBUG);
}


/* 重定向fputc 到输出，单片机一般为串口*/ 
int fputc(int ch, FILE *f)
{
    /* e.g. write a character to the USART */
    (void)Serial_WriteByte(COM2,ch);
    
//    (void)Serial_WriteByte(COM2,ch);
//    USART_SendData(USART_USING2,ch);
    /* Loop until the end of transmission */
//    while(USART_GetFlagStatus(USART_USING2, USART_FLAG_TC) == RESET);
    
    return ch;
}

