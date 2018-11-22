#include "app_cfg.h"
#include "loop.h"

static void prvnvicInit(void);

int main(void)
{   
    prvnvicInit();

    // init sysytem
    tasks_init_System();

    // run system
    while(1)
    {
        tasks_Run_System();
    }
    
    //Should never reach this point!
}

//nvic configuration
static void prvnvicInit(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}




