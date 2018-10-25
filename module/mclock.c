
#include "mclock.h"


static volatile uint32_t Ms_ClockTimerCounter = 0; //time count

/**
  * @brief  ���Ms_ClockTimerCounter��ֵ��
  * @param  None
  * @note   Ms_ClockTimerCounterΪ�ж�ʱ������ 
  * @retval Ms_ClockTimerCounter ��ǰֵ
  */
uint32_t mcu_getCurSysctime(void)
{
    return Ms_ClockTimerCounter;
}
/**
  * @brief  �������ʱ��
  * @param  None
  * @note       ÿ�ߵ�����һ�Σ�����ʱ�䶼���ᳬ2^16��ʱ��
  * @retval  ����ʱ�� ���2^32
  */
uint32_t mcu_elapsedSystime(void)
{
    static volatile uint32_t wHisTimerCounter = 0;
    uint32_t elapsedMSec = 0;
    uint32_t tmp;

    tmp = Ms_ClockTimerCounter;//���ϵͳʱ��

    if(tmp != wHisTimerCounter){
        elapsedMSec = (uint32_t)((tmp - wHisTimerCounter)&0xffffffffu);
        wHisTimerCounter = tmp;
    }

    return elapsedMSec;
}

/**
  * @brief  
  * @param  None
  * @note   
  * @retval  None
  */
void mcu_systime_isr_callback(void)
{
    Ms_ClockTimerCounter++;
}




