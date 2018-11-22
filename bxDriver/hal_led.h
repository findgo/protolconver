




#ifndef __HAL_LED_H_
#define __HAL_LED_H_

#include "app_cfg.h"


#define HAL_LED_RED_PORT		GPIOB
#define HAL_LED_RED_PIN		GPIO_Pin_8

#define HAL_LED_BLUE_PORT    GPIOB
#define HAL_LED_BLUE_PIN    GPIO_Pin_9

#define HAL_LED_GPIO_PeriphClock_EN() do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); }while(0)


void halledInit(void);

#endif
