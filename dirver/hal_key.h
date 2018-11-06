#ifndef __HAL_KEY_H_
#define __HAL_KEY_H_

#include "app_cfg.h"


#define HAL_KEY_RESET_PORT		GPIOB
#define HAL_KEY_RESET_PIN		GPIO_Pin_8

#define HAL_KEY_GPIO_PeriphClock_EN() do{ RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); }while(0)


void halkeyInit(void);

uint8_t halkeyResetIsDown(void);

void keyTask(void);

#endif

