/**
  ******************************************************************************
  * @file   commond_platform.h
  * @author  
  * @version 
  * @date    
  * @brief 		全局文件和宏定义 根据平台修改
  ******************************************************************************
  * @attention 	20151110     v1.1   	jgb		重构	20151120
  * @attention 	20181109     v1.2   	jgb		改文件名, 只提供平台定义
  ******************************************************************************
  */
#ifndef __COMMON_PLATFORM_H_
#define __COMMON_PLATFORM_H_

#include "stm32F10x.h"


//IO口操作宏定义
#define BITBAND(addr, bitnum) (((addr) & 0xF0000000)+0x2000000+(((addr) &0xFFFFF)<<5)+((bitnum)<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum))  

//IO口地址映射
#define GPIOA_ODR_Addr    (GPIOA_BASE + 12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE + 12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE + 12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE + 12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE + 12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE + 12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE + 12) //0x40011E0C    

#define GPIOA_IDR_Addr    (GPIOA_BASE + 8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE + 8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE + 8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE + 8) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE + 8) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE + 8) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE + 8) //0x40011E08 

//对于IO空间映射在存储空间的结构，输入输出处理
#define inp(port)         (*((volatile byte *) (port)))
#define inpw(port)        (*((volatile word *) (port)))
#define inpdw(port)       (*((volatile dword *)(port)))
#define outp(port, val)   (*((volatile byte *) (port)) = ((byte) (val)))
#define outpw(port, val)  (*((volatile word *) (port)) = ((word) (val)))
#define outpdw(port, val) (*((volatile dword *) (port)) = ((dword) (val)))

//-- IO口操作,只对单一的IO口;确保n的值小于16!--------------------------------
#define PAout(n)   		BIT_ADDR(GPIOA_ODR_Addr,n)  						// 输出 
#define PAin(n)    		BIT_ADDR(GPIOA_IDR_Addr,n)  						// 输入 

#define PBout(n)   		BIT_ADDR(GPIOB_ODR_Addr,n)  						// 输出 
#define PBin(n)    		BIT_ADDR(GPIOB_IDR_Addr,n)  						// 输入 

#define PCout(n)   		BIT_ADDR(GPIOC_ODR_Addr,n)  						// 输出 
#define PCin(n)    		BIT_ADDR(GPIOC_IDR_Addr,n)  						// 输入 

#define PDout(n)   		BIT_ADDR(GPIOD_ODR_Addr,n)  						// 输出 
#define PDin(n)    		BIT_ADDR(GPIOD_IDR_Addr,n)  						// 输入 

#define PEout(n)   		BIT_ADDR(GPIOE_ODR_Addr,n)  						// 输出 
#define PEin(n)    		BIT_ADDR(GPIOE_IDR_Addr,n)  						// 输入

#define PFout(n)   		BIT_ADDR(GPIOF_ODR_Addr,n)  						// 输出 
#define PFin(n)    		BIT_ADDR(GPIOF_IDR_Addr,n)  						// 输入

#define PGout(n)   		BIT_ADDR(GPIOG_ODR_Addr,n)  						// 输出 
#define PGin(n)    		BIT_ADDR(GPIOG_IDR_Addr,n)  						// 输入

//-- IO口输入输出模式配置,只对单一的IO口;!--------------------------------
/* 一般用于通信 
 *NOTE: 当为输入时,如何设置上下拉, 要以设置ODR寄存器高低来决定上下拉*/
/* or 的前面用于配置输入的, or的后面用于配置输出的 */
#define PxCfgAin_Or_PP          0x00
#define PxCfgFloating_Or_OD     0x01
#define PxCfgPD_Or_AFPP         0x02
#define PxCfgRes_Or_AFOD        0x03
#define PxModeInput         0x00
#define PxModeOutputS10     0x01
#define PxModeOutputS2      0x02
#define PxModeOutputS50     0x03
/****适用于端口0-7*************************************************/
#define PLxClearPortCfg(GPIOx,n)          ((GPIOx)->CRL &= ~(0x0000000F << ( (n) * 4)))
#define PLxSetPortCfg(GPIOx,n,CNFy,MODEy) ((GPIOx)->CRL |= (((CNFy << 2) | MODEy) << ( (n) * 4)))

#define PLxModeInputAin(GPIOx,n)        {PLxClearPortCfg(GPIOx,n); PLxSetPortCfg(GPIOx,n,PxCfgAin_Or_PP,PxModeInput); }
#define PLxModeInputFloating(GPIOx,n)   {PLxClearPortCfg(GPIOx,n); PLxSetPortCfg(GPIOx,n,PxCfgFloating_Or_OD,PxModeInput); }
#define PLxModeInputIPU(GPIOx,n)        {(GPIOx)->ODR |= 1 << (n);PLxClearPortCfg(GPIOx,n); PLxSetPortCfg(GPIOx,n,PxCfgPD_Or_AFPP,PxModeInput); }
#define PLxModeInputIPD(GPIOx,n)        {(GPIOx)->ODR &= ~(1 << (n));PLxClearPortCfg(GPIOx,n); PLxSetPortCfg(GPIOx,n,PxCfgPD_Or_AFPP,PxModeInput); }
//速率2M
#define PLxModeOutputS2PP(GPIOx,n)      {PLxClearPortCfg(GPIOx,n); PLxSetPortCfg(GPIOx,n,PxCfgAin_Or_PP,PxModeOutputS2); }
#define PLxModeOutputS2OD(GPIOx,n)      {PLxClearPortCfg(GPIOx,n); PLxSetPortCfg(GPIOx,n,PxCfgFloating_Or_OD,PxModeOutputS2); }
#define PLxModeOutputS2AFPP(GPIOx,n)    {PLxClearPortCfg(GPIOx,n); PLxSetPortCfg(GPIOx,n,PxCfgPD_Or_AFPP,PxModeOutputS2); }
#define PLxModeOutputS2AFOD(GPIOx,n)    {PLxClearPortCfg(GPIOx,n); PLxSetPortCfg(GPIOx,n,PxCfgRes_Or_AFOD,PxModeOutputS2); }
//速率10M
#define PLxModeOutputS10PP(GPIOx,n)     {PLxClearPortCfg(GPIOx,n); PLxSetPortCfg(GPIOx,n,PxCfgAin_Or_PP,PxModeOutputS10); }
#define PLxModeOutputS10OD(GPIOx,n)     {PLxClearPortCfg(GPIOx,n); PLxSetPortCfg(GPIOx,n,PxCfgFloating_Or_OD,PxModeOutputS10); }
#define PLxModeOutputS10AFPP(GPIOx,n)   {PLxClearPortCfg(GPIOx,n); PLxSetPortCfg(GPIOx,n,PxCfgPD_Or_AFPP,PxModeOutputS10); }
#define PLxModeOutputS10AFOD(GPIOx,n)   {PLxClearPortCfg(GPIOx,n); PLxSetPortCfg(GPIOx,n,PxCfgRes_Or_AFOD,PxModeOutputS10); }
//速率50M
#define PLxModeOutputS50PP(GPIOx,n)     {PLxClearPortCfg(GPIOx,n); PLxSetPortCfg(GPIOx,n,PxCfgAin_Or_PP,PxModeOutputS50); }
#define PLxModeOutputS50OD(GPIOx,n)     {PLxClearPortCfg(GPIOx,n); PLxSetPortCfg(GPIOx,n,PxCfgFloating_Or_OD,PxModeOutputS50); }
#define PLxModeOutputS50AFPP(GPIOx,n)   {PLxClearPortCfg(GPIOx,n); PLxSetPortCfg(GPIOx,n,PxCfgPD_Or_AFPP,PxModeOutputS50); }
#define PLxModeOutputS50AFOD(GPIOx,n)   {PLxClearPortCfg(GPIOx,n); PLxSetPortCfg(GPIOx,n,PxCfgRes_Or_AFOD,PxModeOutputS50); }
/****适用于端口8-15*************************************************/
#define PHxClearPortCfg(GPIOx,n)          ((GPIOx)->CRH &= ~(0x0000000F << ( (n) * 4)))
#define PHxSetPortCfg(GPIOx,n,CNFy,MODEy) ((GPIOx)->CRH |= (((CNFy << 2) | MODEy) << ( (n) * 4)))

#define PHxModeInputAin(GPIOx,n)        {PHxClearPortCfg(GPIOx,n); PHxSetPortCfg(GPIOx,n,PxCfgAin_Or_PP,PxModeInput); }
#define PHxModeInputFloating(GPIOx,n)   {PHxClearPortCfg(GPIOx,n); PHxSetPortCfg(GPIOx,n,PxCfgFloating_Or_OD,PxModeInput); }
#define PHxModeInputIPU(GPIOx,n)        {(GPIOx)->ODR |= 1 << (n);PHxClearPortCfg(GPIOx,n); PHxSetPortCfg(GPIOx,n,PxCfgPD_Or_AFPP,PxModeInput); }
#define PHxModeInputIPD(GPIOx,n)        {(GPIOx)->ODR &= ~(1 << (n));PHxClearPortCfg(GPIOx,n); PHxSetPortCfg(GPIOx,n,PxCfgPD_Or_AFPP,PxModeInput); }
//速率2M
#define PHxModeOutputS2PP(GPIOx,n)      {PHxClearPortCfg(GPIOx,n); PHxSetPortCfg(GPIOx,n,PxCfgAin_Or_PP,PxModeOutputS2); }
#define PHxModeOutputS2OD(GPIOx,n)      {PHxClearPortCfg(GPIOx,n); PHxSetPortCfg(GPIOx,n,PxCfgFloating_Or_OD,PxModeOutputS2); }
#define PHxModeOutputS2AFPP(GPIOx,n)    {PHxClearPortCfg(GPIOx,n); PHxSetPortCfg(GPIOx,n,PxCfgPD_Or_AFPP,PxModeOutputS2); }
#define PHxModeOutputS2AFOD(GPIOx,n)    {PHxClearPortCfg(GPIOx,n); PHxSetPortCfg(GPIOx,n,PxCfgRes_Or_AFOD,PxModeOutputS2); }
//速率10M
#define PHxModeOutputS10PP(GPIOx,n)     {PHxClearPortCfg(GPIOx,n); PHxSetPortCfg(GPIOx,n,PxCfgAin_Or_PP,PxModeOutputS10); }
#define PHxModeOutputS10OD(GPIOx,n)     {PHxClearPortCfg(GPIOx,n); PHxSetPortCfg(GPIOx,n,PxCfgFloating_Or_OD,PxModeOutputS10); }
#define PHxModeOutputS10AFPP(GPIOx,n)   {PHxClearPortCfg(GPIOx,n); PHxSetPortCfg(GPIOx,n,PxCfgPD_Or_AFPP,PxModeOutputS10); }
#define PHxModeOutputS10AFOD(GPIOx,n)   {PHxClearPortCfg(GPIOx,n); PHxSetPortCfg(GPIOx,n,PxCfgRes_Or_AFOD,PxModeOutputS10); }
//速率50M
#define PHxModeOutputS50PP(GPIOx,n)     {PHxClearPortCfg(GPIOx,n); PHxSetPortCfg(GPIOx,n,PxCfgAin_Or_PP,PxModeOutputS50); }
#define PHxModeOutputS50OD(GPIOx,n)     {PHxClearPortCfg(GPIOx,n); PHxSetPortCfg(GPIOx,n,PxCfgFloating_Or_OD,PxModeOutputS50); }
#define PHxModeOutputS50AFPP(GPIOx,n)   {PHxClearPortCfg(GPIOx,n); PHxSetPortCfg(GPIOx,n,PxCfgPD_Or_AFPP,PxModeOutputS50); }
#define PHxModeOutputS50AFOD(GPIOx,n)   {PHxClearPortCfg(GPIOx,n); PHxSetPortCfg(GPIOx,n,PxCfgRes_Or_AFOD,PxModeOutputS50); }

/***********************************************************
* 宏功能描述:以下宏解决回绕问题
*备注:		//宏参数必须为uint32_t类型
*			//该宏最大可判断2^31-1的延时程序
************************************************************/
//当a在b的后面(大于),此宏为真
#define timer_after(__a,__b) 	((int32_t)(__b) - (int32_t)(__a) < 0)
#define timer_after_eq(__a,__b)  ((int32_t)(__b) - (int32_t)(__a) <= 0)
//当a在b的前面(小于),此宏为真
#define timer_befor(__a,__b) 		timer_after(__b,__a)
#define timer_befor_eq(__a,__b) 	timer_after_eq(__b,__a)


//! \name finit state machine state
typedef enum {
    fsm_rt_err          = -1,    //!< fsm error, error code can be get from other interface
    fsm_rt_cpl          = 0,     //!< fsm complete
    fsm_rt_on_going     = 1,     //!< fsm on-going
    fsm_rt_asyn         = 2,     //!< fsm asynchronose complete, you can check it later.
    fsm_rt_wait_for_obj = 3,     //!< fsm wait for object
} fsm_rt_t;

#define IS_FSM_ERR(__FSM_RT)	((__FSM_RT) < fsm_rt_cpl)
#define IS_FSM_CPL(__FSM_RT)	((__FSM_RT) == fsm_rt_cpl)


#endif
