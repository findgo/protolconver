#include "hal_key.h"
#include "mkey.h"
#include "timers.h"
#include "mt_app.h"

#define HAL_KEY_SCAN_TIME   10

#define key_log(format,args...)      log_debugln(format,##args )

static uint8_t halkeyResetIsDown(void);
static void halkeyCB(void *arg);

uint8_t BxGPIO_ReadInputDataBit(GPIO_TypeDef* GPIOx, uint32_t GPIO_Pin)
{
    if ((LL_GPIO_ReadInputPort(GPIOx) & GPIO_Pin) != (uint32_t)0){
        return 0x01;
    }
    
    return 0x00;
}

static timer_t keytimer;
static mkeycfgStatic_t keycfgReset;

void halkeyInit(void)
{
    mkeyAssign(&keycfgReset, halkeyResetIsDown, MKEY_PRESS1_DOWN, MKEY_PRESS1_LONG, MKEY_NULL, 0, 600, 0);
    timerAssign(&keytimer, halkeyCB , (void *)&keytimer);
    timerStart(&keytimer, HAL_KEY_SCAN_TIME);
}


static uint8_t halkeyResetIsDown(void)
{
    return !LL_GPIO_IsInputPinSet(HAL_KEY_RESET_PORT, HAL_KEY_RESET_PIN);
}

static void halkeyCB(void *arg)
{
    mkeydecetor_task();
    timerRestart(*((timer_t * *)arg), HAL_KEY_SCAN_TIME);
}

void keyTask(void)
{
    uint8_t keyVal = mkeygetvalue();

    switch(keyVal){
    case MKEY_PRESS1_DOWN:
        key_log("reset key down!");
        //mtsapi_SystemReset();
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

