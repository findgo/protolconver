
#include "loop.h"
#include "app_cfg.h"

//for module
#include "memalloc.h"
#include "timers.h"
#include "event_groups.h"
#include "log.h"

// for ltl
#include "ltl.h"
#include "ltl_app_genattr.h"
//for driver
#include "wintom.h"
#include "mleds.h"
#include "hal_key.h"

#include "curtain.h"
#include "bxnwk.h"

#include "mt_npi.h"


extern void SystemClock_Config(void);

void tasks_init_System(void)
{
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
    
    bxNwkInit();
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
    bxNwkTask();
    timerTask();
    wintomTask();
    npiTask();
    keyTask();
}


int main(void)
{
    // for stm32cubeMX init must be init first
    /* MCU Configuration--------------------------------------------------------*/
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
    SystemCoreClockUpdate();

    // init sysytem
    tasks_init_System();

    // run system
    while(1)
    {
        tasks_Run_System();
    }
}
