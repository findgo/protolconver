#include "hal_key.h"
#include "mkey.h"
#include "timers.h"
#include "mt_sapi.h"

#define HAL_KEY_SCAN_TIME   10

#define key_log(format,args...)      log_debugln(format,##args )

static uint8_t halkeyResetIsDown(void);
static void halkeyCB(void *arg);


static TimerHandle_t keytimeHandle;
static TimerStatic_t keytimer;
static mkeycfgStatic_t keycfgReset;


void halkeyInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	HAL_KEY_GPIO_PeriphClock_EN();

//for red led
	GPIO_InitStruct.GPIO_Pin = HAL_KEY_RESET_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(HAL_KEY_RESET_PORT, &GPIO_InitStruct);

    mkeyAssign(&keycfgReset,  halkeyResetIsDown, MKEY_PRESS1_DOWN, MKEY_PRESS1_LONG, MKEY_NULL, 0, 600, 0);
    keytimeHandle = timerAssign(&keytimer, halkeyCB , (void *)&keytimeHandle);
    timerStart(keytimeHandle, HAL_KEY_SCAN_TIME);
}


static uint8_t halkeyResetIsDown(void)
{
    return !GPIO_ReadInputDataBit(HAL_KEY_RESET_PORT, HAL_KEY_RESET_PIN);
}

static void halkeyCB(void *arg)
{
    mkeydecetor_task();
    timerRestart(*((TimerHandle_t *)arg), HAL_KEY_SCAN_TIME);
}

void keyTask(void)
{
    uint8_t keyVal = mkeygetvalue();

    switch(keyVal){
    case MKEY_PRESS1_DOWN:
        key_log("reset key down!");
        
        break;
    case MKEY_PRESS1_UP:
        key_log("reset key up!");
        break;
    case MKEY_PRESS1_LONG:
        key_log("reset key long down!");
        mtsapi_ResetFactory();
        break;
    default:
        break;
    }
    
}
