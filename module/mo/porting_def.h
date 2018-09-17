

// 本文件主要是从其它平台移植的库的一些定义
/*
 *
 *
 */

#ifndef __PORTING_DEF_H__
#define __PORTING_DEF_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>


/* set support dynamic alloca compatibility. */
#define configSUPPORT_DYNAMIC_ALLOCATION        1







//提供中断级临界保护宏
// set a save variable and enter exit criticial macro
// portCriticial_state_Variable: 定义一个保存变量
//#define portCriticial_state_Variable    //halIntState_t bintstate
//#define portCriticial_Enter_code()      //ENTER_SAFE_ATOM_CODE(bintstate)
//#define protCriticial_Exit_code()       //EXIT_SAFE_ATOM_CODE(bintstate)

// 用于任务级临界保护宏,线程保护?
#define taskENTER_CRITICAL()
#define taskENTER_CRITICAL_FROM_ISR()  
#define taskEXIT_CRITICAL()             
#define taskEXIT_CRITICAL_FROM_ISR( x )         

#define mtCOVERAGE_TEST_DELAY()
#define mtCOVERAGE_TEST_MARKER()
#define configASSERT( x )

#endif
