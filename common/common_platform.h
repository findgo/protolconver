/**
  ******************************************************************************
  * @file   commond_signal.h
  * @author  
  * @version 
  * @date    
  * @brief 		ȫ���ļ��ͺ궨�� ����ƽ̨�޸�
  ******************************************************************************
  * @attention 	20151110     v1.1   	jgb		�ع�	20151120
  ******************************************************************************
  */
#ifndef __COMMON_GLOBAL_H_
#define __COMMON_GLOBAL_H_

#include "stm32F10x.h"


//IO�ڲ����궨��
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum))  

//IO�ڵ�ַӳ��
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

//����IO�ռ�ӳ���ڴ洢�ռ�Ľṹ�������������
#define inp(port)         (*((volatile byte *) (port)))
#define inpw(port)        (*((volatile word *) (port)))
#define inpdw(port)       (*((volatile dword *)(port)))
#define outp(port, val)   (*((volatile byte *) (port)) = ((byte) (val)))
#define outpw(port, val)  (*((volatile word *) (port)) = ((word) (val)))
#define outpdw(port, val) (*((volatile dword *) (port)) = ((dword) (val)))

/***********************************************************
* �깦������:���º�����������
*��ע:		//���������Ϊuint32_t����
*			//�ú������ж�2^31-1����ʱ����
************************************************************/
//��a��b�ĺ���(����),�˺�Ϊ��
#define timer_after(__a,__b) 	((int32_t)(__b) - (int32_t)(__a) < 0)
#define timer_after_eq(__a,__b)  ((int32_t)(__b) - (int32_t)(__a) <= 0)
//��a��b��ǰ��(С��),�˺�Ϊ��
#define timer_befor(__a,__b) 		timer_after(__b,__a)
#define timer_befor_eq(__a,__b) 	timer_after_eq(__b,__a)


/*
 *  This macro is for use by other macros to form a fully valid C statement.
 *  Without this, the if/else conditionals could show unexpected behavior.
 *
 *  For example, use...
 *    #define SET_REGS()  st( ioreg1 = 0; ioreg2 = 0; )
 *  instead of ...
 *    #define SET_REGS()  { ioreg1 = 0; ioreg2 = 0; }
 *  or
 *    #define  SET_REGS()    ioreg1 = 0; ioreg2 = 0;
 *  The last macro would not behave as expected in the if/else construct.
 *  The second to last macro will cause a compiler error in certain uses
 *  of if/else construct
 *
 *  It is not necessary, or recommended, to use this macro where there is
 *  already a valid C statement.  For example, the following is redundant...
 *    #define CALL_FUNC()   st(  func();  )
 *  This should simply be...
 *    #define CALL_FUNC()   func()
 *
 * (The while condition below evaluates false without generating a
 *  constant-controlling-loop type of warning on most compilers.)
 */
#define st(x)      do { x } while (__LINE__ == -1)

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

//! \name standard error code
typedef enum {
    GSF_ERR_NONE                       =0,     //!< none error
    GSF_ERR_GENERAL                    =-1,    //!< unspecified error
    GSF_ERR_NOT_SUPPORT                =-2,    //!< function not supported
    GSF_ERR_NOT_READY                  =-3,    //!< service not ready yet
    GSF_ERR_NOT_AVAILABLE              =-4,    //!< service not available
    GSF_ERR_NOT_ACCESSABLE             =-5,    //!< target not acceesable
    GSF_ERR_NOT_ENOUGH_RESOURCES       =-6,    //!< no enough resources
    GSF_ERR_FAIL                       =-7,    //!< failed
    GSF_ERR_INVALID_PARAMETER          =-8,    //!< invalid parameter
    GSF_ERR_INVALID_RANGE              =-9,    //!< invalid range
    GSF_ERR_INVALID_PTR                =-10,   //!< invalid pointer
    GSF_ERR_INVALID_KEY                =-11,   //!< invalid key
    GSF_ERR_IO                         =-12,   //!< IO error
    GSF_ERR_REQ_ALREADY_REGISTERED     =-13,   //!< request all ready exist
    GSF_ERR_UNKNOWN                    =-128,  //!< unknown error
} gsf_err_t;




#endif
