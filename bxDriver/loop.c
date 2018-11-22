
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
const pTaskFn_t tasksArr[] =
{
    NULL
};
const uint8_t tasksCnt = sizeof(tasksArr) / sizeof(tasksArr[0]);
uint16_t *tasksEvents;
#endif

void tasks_init_System(void)
{
#if configSUPPORT_TASKS_EVENT > 0
    uint8_t taskID = 0;
    
    tasksEvents = (uint16_t *)mo_malloc( sizeof( uint16_t ) * tasksCnt);
    memset( tasksEvents, 0, (sizeof( uint16_t ) * tasksCnt));
#endif

    Systick_Configuration();
    log_Init();
    log_infoln("loop_init_System init begin!");
    
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

    log_infoln("loop_init_System init end, and start!");
}
void tasksPoll(void)
{
    nwkTask();
    timerTask();
    wintomTask();
    npiTask();
    keyTask();
}

