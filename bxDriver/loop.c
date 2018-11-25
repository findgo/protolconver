
#include "loop.h"
#include "app_cfg.h"

//for module
#include "memalloc.h"
#include "timers.h"
#include "event_groups.h"
#include "log.h"

// for ltl
#include "ltl.h"
#include "ltl_genattr.h"
//for driver
#include "wintom.h"
#include "mleds.h"
#include "hal_key.h"
#include "systick.h"

#include "curtain.h"
#include "nwk.h"

#include "mt_npi.h"


extern void SystemClock_Config(void);

#if configSUPPORT_TASKS_EVENT > 0
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


    /* Initialize all configured peripherals */
    MX_GPIO_Init();

    Systick_Configuration();
    log_Init();
    log_infoln("loop_init_System init begin!");
    
// driver init 
    // led
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


int main(void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    
    
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
    
    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_2);
    
    /* System interrupt init*/
    
    /**NOJTAG: JTAG-DP Disabled and SW-DP Enabled 
    */
    LL_GPIO_AF_Remap_SWJ_NOJTAG();
    /* USER CODE BEGIN Init */
    
    /* USER CODE END Init */
    
    /* Configure the system clock */
    SystemClock_Config();

    // init sysytem
    tasks_init_System();

    // run system
    while(1)
    {
        tasks_Run_System();
    }
    


}
