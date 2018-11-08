
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


#if LOOP_TASKS_EVENT_ENABLE > 0
// 事件触发,为未来低功耗节能
static const pTaskFn_t tasksArr[] =
{
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
#if LOOP_TASKS_EVENT_ENABLE > 0
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
    
#if LOOP_TASKS_EVENT_ENABLE > 0
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
#if LOOP_TASKS_EVENT_ENABLE > 0
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

