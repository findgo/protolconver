/**
  ******************************************************************************
  * @file   heap_macros.h
  * @author   jgb
  * @version 
  * @date    
  * @brief    本文档宏用于控制堆的配置，
  ******************************************************************************
  * @attention      v1.1    jgb     20170512重构 
  * @attention      v1.2    jgb     20180913 抽像出临界保护宏
  ******************************************************************************
  */
  /*  本堆管理全部移植于freertos 的堆管理，几乎未做任何修改，只修改了临界区的保护,将宏抽出来了
  *  heap_1  只实现分配，不实现释放
  *  heap_2  实现分配和释放，但不支持碎片管理
  *  heap_3  对库文件malloc free做了临界保护
  *  heap_4  实现分配和释放,而且支持碎片管理
  *  heap_5  在heap_4的基础上,具有跨多个非相邻内存区域的堆能力
  *
  *  一般情况下对于单片机只要用heap_1或heap_2就行了，只分配不释放用heap_1也行
  *  对于任何修改只需修改本文件即可使用
  */
#ifndef __HEAP_MACROS_H_
#define __HEAP_MACROS_H_

#include <stdint.h>
#include "porting_def.h"
//#include "common_signal.h"

/* set support dynamic alloca compatibility. */
#define configSUPPORT_DYNAMIC_ALLOCATION        1

/* Default total heap size 17K*/
#define configTOTAL_HEAP_SIZE               ( ( size_t ) ( 4 * 1024 ) )

/* set byte alignment */
#define portBYTE_ALIGNMENT                  4
#define portPOINTER_SIZE_TYPE               uint32_t

//提供临界保护宏
// set a save variable and enter exit criticial macro
// portCriticial_state_Variable: 定义一个保存变量
#define portCriticial_state_Variable    //halIntState_t bintstate
#define portCriticial_Enter_code()      //ENTER_SAFE_ATOM_CODE(bintstate)
#define protCriticial_Exit_code()       //EXIT_SAFE_ATOM_CODE(bintstate)

/* set use app malloc failed hook */
#define configUSE_MALLOC_FAILED_HOOK    0

/* for debug printf malloc and free trace */
#define traceMALLOC( pvAddress, uiSize )
#define traceFREE( pvAddress, uiSize )


#endif



