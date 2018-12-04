#ifndef __HAL_KEY_H_
#define __HAL_KEY_H_

#include "app_cfg.h"


#define HAL_KEY_RESET_PORT		HAL_KEY_RESET_GPIO_Port
#define HAL_KEY_RESET_PIN		HAL_KEY_RESET_Pin

void halkeyInit(void);
void keyTask(void);

#endif

