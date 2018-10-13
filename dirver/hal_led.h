




#ifndef __HAL_LED_H_
#define __HAL_LED_H_

#include "app_cfg.h"


#define HAL_REDLED_PORT		GPIOB
#define HAL_REDLED_PIN		GPIO_Pin_8

#define HAL_BLUELED_PORT    GPIOB
#define HAL_BLUELED_PIN    GPIO_Pin_9

#define HAL_LED_GPIO_PeriphClock_EN() do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); }while(0)


void halledInit(void);

#endif
