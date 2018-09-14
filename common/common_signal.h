/**
  ******************************************************************************
  * @file   common_signal.h
  * @author  
  * @version 
  * @date    
  * @brief  中断宏与互斥锁宏定义
  ******************************************************************************
  * @attention 	20151110     v1.1   	jgb		重构
  ******************************************************************************
  */

#ifndef __COMMON_SIGNAL_H_
#define __COMMON_SIGNAL_H_

#include "common_platform.h"
#include "common_def.h"

typedef uint8_t halIntState_t;

#define OS_ENTER_CRITICIAL()        st( __set_PRIMASK(1); )
#define OS_EXIT_CRITICIAL()         st( __set_PRIMASK(0); )

#define OS_ENABLE_INTERRUPTS()         st( __set_PRIMASK(0); )
#define OS_DISABLE_INTERRUPTS()        st( __set_PRIMASK(1); )

#define GLOABLE_INTERRUPT_ENABLED()     ((0 ==  __get_PRIMASK()) ? TRUE : FALSE)


//! \brief The safe ATOM code section macro
#define SAFE_ATOM_CODE(__CODE)     \
    {\
        halIntState_t bintstate = GLOABLE_INTERRUPT_ENABLED();\
        OS_ENTER_CRITICIAL();\
        __CODE;\
        if (bintstate){ \
            OS_EXIT_CRITICIAL();\
        }\
    }
#define ENTER_SAFE_ATOM_CODE(bintstate)  \
            bintstate = GLOABLE_INTERRUPT_ENABLED();\
            OS_ENTER_CRITICIAL();

//! \brief Exit from the safe atom operations
#define EXIT_SAFE_ATOM_CODE(bintstate)           \
            if (bintstate) {    \
                OS_EXIT_CRITICIAL();\
            }

//! \brief ATOM code section macro
#define ATOM_CODE(__CODE)  \
            {\
                OS_ENTER_CRITICIAL();\
                __CODE;\
                OS_EXIT_CRITICIAL();\
            }

#define ENTER_ATOM_CODE()  OS_ENTER_CRITICIAL();             
//! \brief Exit from the atom operations
#define EXIT_ATOM_CODE()   OS_EXIT_CRITICIAL();

#endif
