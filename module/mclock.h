/**
  ******************************************************************************
  * @file   mod_clock.c
  * @author  
  * @version 
  * @date    
  * @brief   ϵͳʱ������
  ******************************************************************************
  * @attention      v1.1    jgb     �ع�        			 20151112  
  * @attention      v1.2    jgb     �������������淶          20170427
  ******************************************************************************
  */
#ifndef __MCLOCK_H_
#define __MCLOCK_H_

#include "app_cfg.h"

#define CLOCKTICK_RATE_HZ    (1000)   // 1000hz
#define CLOCKTICK_RATE_MS   (1000 / CLOCKTICK_RATE_HZ)


typedef struct {
    uint32_t start;
}ctimer_t;

uint32_t mcu_getCurSysctime(void);
uint32_t mcu_elapsedSystime(void);
void mcu_systime_isr_callback(void);

#define ctimerStart(t)  do{ (t).start = mcu_getCurSysctime();}while(0)
#define ctimerExpired(t, timeout)   timer_after_eq(mcu_getCurSysctime(), (t).start + timeout)

#endif

